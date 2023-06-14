#include "yolov8onnx.h"

// YOLOX use the same focus in yolov5
class YoloV5Focus : public ncnn::Layer
{
public:
    YoloV5Focus()
    {
        one_blob_only = true;
    }

    virtual int forward(const ncnn::Mat& bottom_blob, ncnn::Mat& top_blob, const ncnn::Option& opt) const
    {
        int w = bottom_blob.w;
        int h = bottom_blob.h;
        int channels = bottom_blob.c;

        int outw = w / 2;
        int outh = h / 2;
        int outc = channels * 4;

        top_blob.create(outw, outh, outc, 4u, 1, opt.blob_allocator);
        if (top_blob.empty())
            return -100;

#pragma omp parallel for num_threads(opt.num_threads)
        for (int p = 0; p < outc; p++)
        {
            const float* ptr = bottom_blob.channel(p % channels).row((p / channels) % 2) + ((p / channels) / 2);
            float* outptr = top_blob.channel(p);

            for (int i = 0; i < outh; i++)
            {
                for (int j = 0; j < outw; j++)
                {
                    *outptr = *ptr;

                    outptr += 1;
                    ptr += 2;
                }

                ptr += w;
            }
        }

        return 0;
    }
};

DEFINE_LAYER_CREATOR(YoloV5Focus)



/**
 * @brief YoloV8Onnx 构造函数
 * @param
 */
YoloV8Onnx::YoloV8Onnx(QObject *parent)
    : QObject{parent}
{
    yolox = new ncnn::Net();
    yolox->register_custom_layer("YoloV5Focus", YoloV5Focus_layer_creator);
    yolox->load_param("./models/yoloxN.param");
    yolox->load_model("./models/yoloxN.bin");

}

const char* YoloV8Onnx::class_names[] = {
    "person", "bicycle", "car", "motorcycle", "airplane", "bus", "train", "truck", "boat", "traffic light",
    "fire hydrant", "stop sign", "parking meter", "bench", "bird", "cat", "dog", "horse", "sheep", "cow",
    "elephant", "bear", "zebra", "giraffe", "backpack", "umbrella", "handbag", "tie", "suitcase", "frisbee",
    "skis", "snowboard", "sports ball", "kite", "baseball bat", "baseball glove", "skateboard", "surfboard",
    "tennis racket", "bottle", "wine glass", "cup", "fork", "knife", "spoon", "bowl", "banana", "apple",
    "sandwich", "orange", "broccoli", "carrot", "hot dog", "pizza", "donut", "cake", "chair", "couch",
    "potted plant", "bed", "dining table", "toilet", "tv", "laptop", "mouse", "remote", "keyboard", "cell phone",
    "microwave", "oven", "toaster", "sink", "refrigerator", "book", "clock", "vase", "scissors", "teddy bear",
    "hair drier", "toothbrush"
};

void YoloV8Onnx::initTracker(){
    int fps = 30;
    tracker = new BYTETracker(fps, 30);
}

void YoloV8Onnx::deleteTracker(){
    delete tracker;
    output_stracks.clear();
}

struct GridAndStride
{
    int grid0;
    int grid1;
    int stride;
};

static inline float intersection_area(const Object& a, const Object& b)
{
    cv::Rect_<float> inter = a.rect & b.rect;
    return inter.area();
}

static void qsort_descent_inplace(std::vector<Object>& faceobjects, int left, int right)
{
    int i = left;
    int j = right;
    float p = faceobjects[(left + right) / 2].prob;

    while (i <= j)
    {
        while (faceobjects[i].prob > p)
            i++;

        while (faceobjects[j].prob < p)
            j--;

        if (i <= j)
        {
            // swap
            std::swap(faceobjects[i], faceobjects[j]);

            i++;
            j--;
        }
    }

#pragma omp parallel sections
    {
#pragma omp section
        {
            if (left < j) qsort_descent_inplace(faceobjects, left, j);
        }
#pragma omp section
        {
            if (i < right) qsort_descent_inplace(faceobjects, i, right);
        }
    }
}

static void qsort_descent_inplace(std::vector<Object>& objects)
{
    if (objects.empty())
        return;

    qsort_descent_inplace(objects, 0, objects.size() - 1);
}

static void nms_sorted_bboxes(const std::vector<Object>& faceobjects, std::vector<int>& picked, float nms_threshold)
{
    picked.clear();

    const int n = faceobjects.size();

    std::vector<float> areas(n);
    for (int i = 0; i < n; i++)
    {
        areas[i] = faceobjects[i].rect.area();
    }

    for (int i = 0; i < n; i++)
    {
        const Object& a = faceobjects[i];

        int keep = 1;
        for (int j = 0; j < (int)picked.size(); j++)
        {
            const Object& b = faceobjects[picked[j]];

            // intersection over union
            float inter_area = intersection_area(a, b);
            float union_area = areas[i] + areas[picked[j]] - inter_area;
            // float IoU = inter_area / union_area
            if (inter_area / union_area > nms_threshold)
                keep = 0;
        }

        if (keep)
            picked.push_back(i);
    }
}

static void generate_grids_and_stride(const int target_size, std::vector<int>& strides, std::vector<GridAndStride>& grid_strides)
{
    for (int i = 0; i < (int)strides.size(); i++)
    {
        int stride = strides[i];
        int num_grid = target_size / stride;
        for (int g1 = 0; g1 < num_grid; g1++)
        {
            for (int g0 = 0; g0 < num_grid; g0++)
            {
                GridAndStride gs;
                gs.grid0 = g0;
                gs.grid1 = g1;
                gs.stride = stride;
                grid_strides.push_back(gs);
            }
        }
    }
}

static void generate_yolox_proposals(std::vector<GridAndStride> grid_strides, const ncnn::Mat& feat_blob, float prob_threshold, std::vector<Object>& objects)
{
    //    const int num_grid = feat_blob.h;
    const int num_class = feat_blob.w - 5;
    const int num_anchors = grid_strides.size();

    const float* feat_ptr = feat_blob.channel(0);
    for (int anchor_idx = 0; anchor_idx < num_anchors; anchor_idx++)
    {
        const int grid0 = grid_strides[anchor_idx].grid0;
        const int grid1 = grid_strides[anchor_idx].grid1;
        const int stride = grid_strides[anchor_idx].stride;

        // yolox/models/yolo_head.py decode logic
        //  outputs[..., :2] = (outputs[..., :2] + grids) * strides
        //  outputs[..., 2:4] = torch.exp(outputs[..., 2:4]) * strides
        float x_center = (feat_ptr[0] + grid0) * stride;
        float y_center = (feat_ptr[1] + grid1) * stride;
        float w = exp(feat_ptr[2]) * stride;
        float h = exp(feat_ptr[3]) * stride;
        float x0 = x_center - w * 0.5f;
        float y0 = y_center - h * 0.5f;

        float box_objectness = feat_ptr[4];
        for (int class_idx = 0; class_idx < num_class; class_idx++){
            float box_cls_score = feat_ptr[5 + class_idx];
            float box_prob = box_objectness * box_cls_score;
            if (box_prob > prob_threshold){
                Object obj;
                obj.rect.x = x0;
                obj.rect.y = y0;
                obj.rect.width = w;
                obj.rect.height = h;
                obj.label = class_idx;
                obj.prob = box_prob;

                objects.push_back(obj);
            }
        } // class loop
        feat_ptr += feat_blob.w;
    } // point anchor loop
}

int YoloV8Onnx::detect_yolox(const cv::Mat& bgr, std::vector<Object>& objects)
{

    int target_size = 416;
    const float prob_threshold = 0.25f;
    const float nms_threshold = 0.45f;
    //const float norm_vals[3] = {1 / 255.f, 1 / 255.f, 1 / 255.f};
    int img_w = bgr.cols;
    int img_h = bgr.rows;

    // letterbox pad to multiple of 32
    int w = img_w;
    int h = img_h;
    float scale = 1.f;
    if (w > h)
    {
        scale = (float)target_size / w;
        w = target_size;
        h = h * scale;
    }
    else
    {
        scale = (float)target_size / h;
        h = target_size;
        w = w * scale;
    }

    ncnn::Mat in = ncnn::Mat::from_pixels_resize(bgr.data, ncnn::Mat::PIXEL_BGR, img_w, img_h, w, h);

    // pad to target_size rectangle
    int wpad = target_size - w;
    int hpad = target_size - h;
    ncnn::Mat in_pad;
    // different from yolov5, yolox only pad on bottom and right side,
    // which means users don't need to extra padding info to decode boxes coordinate.
    ncnn::copy_make_border(in, in_pad, 0, hpad, 0, wpad, ncnn::BORDER_CONSTANT, 114.f);
    //    qDebug() <<"wh:" <<wpad << "x"<< hpad ;
    ncnn::Extractor ex = yolox->create_extractor();

    ex.input("images", in_pad);

    std::vector<Object> proposals;
    {
        ncnn::Mat out;
        ex.extract("output", out);

        static const int stride_arr[] = {8, 16, 32}; // might have stride=64 in YOLOX
        std::vector<int> strides(stride_arr, stride_arr + sizeof(stride_arr) / sizeof(stride_arr[0]));
        std::vector<GridAndStride> grid_strides;
        generate_grids_and_stride(target_size, strides, grid_strides);
        generate_yolox_proposals(grid_strides, out, prob_threshold, proposals);
    }
    // sort all proposals by score from highest to lowest
    qsort_descent_inplace(proposals);

    // apply nms with nms_threshold
    std::vector<int> picked;
    nms_sorted_bboxes(proposals, picked,nms_threshold);

    int count = picked.size();

    objects.resize(count);
    for (int i = 0; i < count; i++)
    {
        objects[i] = proposals[picked[i]];

        // adjust offset to original unpadded
        float x0 = (objects[i].rect.x) / scale;
        float y0 = (objects[i].rect.y) / scale;
        float x1 = (objects[i].rect.x + objects[i].rect.width) / scale;
        float y1 = (objects[i].rect.y + objects[i].rect.height) / scale;

        // clip
        x0 = std::max(std::min(x0, (float)(img_w - 1)), 0.f);
        y0 = std::max(std::min(y0, (float)(img_h - 1)), 0.f);
        x1 = std::max(std::min(x1, (float)(img_w - 1)), 0.f);
        y1 = std::max(std::min(y1, (float)(img_h - 1)), 0.f);

        objects[i].rect.x = x0;
        objects[i].rect.y = y0;
        objects[i].rect.width = x1 - x0;
        objects[i].rect.height = y1 - y0;
    }

    return 0;
}
// Set up camera parameters
#define CAMERA_CENTER_X 320
#define CAMERA_CENTER_Y 240

// Set up servo parameters
#define SERVO_RANGE_MIN -135
#define SERVO_RANGE_MAX 135

// Set up control parameters
#define KP 1.0

int normalizeValue(int x, int min_value, int max_value, int normalized_min, int normalized_max) {
    double normalized_x = (x - min_value) / static_cast<double>(max_value - min_value);
    double scaled_x = normalized_x * (normalized_max - normalized_min) + normalized_min;
    return static_cast<int>(scaled_x);
}

// Control servo to keep object in the center of the image
void controlServo(cv::Rect detection, int& servo_x, int& servo_y) {
    // Calculate detection center coordinates
    double center_x = detection.x + detection.width / 2.0;
    double center_y = detection.y + detection.height / 2.0;
    qDebug() << "objecet center_xy:" << center_x << "," << center_y;
    // Calculate position error
    double error_x = center_x - CAMERA_CENTER_X;
    double error_y = CAMERA_CENTER_Y - center_y  ;
    qDebug() << "objecet error_xy:" << error_x << "," << error_y;
    // Calculate control signal adjustment
    //    int delta_x = (int)(KP * error_x);
    //    int delta_y = (int)(KP * error_y);
    servo_x = normalizeValue(error_x, -320, 320, SERVO_RANGE_MIN,SERVO_RANGE_MAX);
    servo_y = normalizeValue(error_y, -240, 240, SERVO_RANGE_MIN,SERVO_RANGE_MAX);
    // Limit servo control signals within the servo range
    servo_x = std::max(std::min(servo_x, SERVO_RANGE_MAX), SERVO_RANGE_MIN);
    servo_y = std::max(std::min(servo_y, SERVO_RANGE_MAX), SERVO_RANGE_MIN);
    qDebug() << "objecet servo_xy:" << servo_x << "," << servo_y;
}

cv::Mat drawCrosshair(cv::Mat image, cv::Scalar color, int thickness) {
    // Calculate center point of the image
    cv::Point center(image.cols / 2, image.rows / 2);
    // Draw vertical line
    cv::line(image, cv::Point(center.x, 0), cv::Point(center.x, image.rows - 1), color, thickness);
    // Draw horizontal line
    cv::line(image, cv::Point(0, center.y), cv::Point(image.cols - 1, center.y), color, thickness);
    return image;
}

void YoloV8Onnx::tracking(cv::Mat m){
    cv::Scalar color(0, 0, 255); // red color
    int thickness = 2;
    cv::resize(m, m, cv::Size(640,480));

    cv::Mat frame = m;
    std::vector<Object> objects;
    detect_yolox(frame, objects);
    output_stracks = tracker->update(objects);
    cv::Rect rect_1;
    int servo_x = 0;
    int servo_y = 0;
    //单个物体
    if(output_stracks.size() > 0){
        vector<float> tlwh = output_stracks[0].tlwh;
        bool vertical = tlwh[2] / tlwh[3] > 1.6;
        if (tlwh[2] * tlwh[3] > 20 && !vertical){
            Scalar s = tracker->get_color(output_stracks[0].track_id);
            putText(frame, format("%d", output_stracks[0].track_id), Point(tlwh[0], tlwh[1] - 5),
                    0, 0.6, Scalar(0, 0, 255), 2, LINE_AA);
            rectangle(frame, Rect(tlwh[0], tlwh[1], tlwh[2], tlwh[3]), s, thickness);
            int center_x = tlwh[0] + tlwh[2] / 2;
            int center_y = tlwh[1] + tlwh[3] / 2;
            cv::line(frame, cv::Point(center_x, tlwh[1]), cv::Point(center_x, tlwh[1] + tlwh[3]), s, thickness);
            cv::line(frame, cv::Point(tlwh[0], center_y), cv::Point(tlwh[0] + tlwh[2], center_y), s, thickness);
            rect_1 = Rect(tlwh[0], tlwh[1], tlwh[2], tlwh[3]);
            controlServo(rect_1, servo_x, servo_y);
            frame = drawCrosshair(frame, color, thickness);
        }
        cv::Point servo_xy = cv::Point(servo_x, servo_y);
        QImage new_image = cvMatToQImage(frame);
        emit sendDectectImage(new_image, servo_xy);
    }
    //多个物体
    //    for(size_t i = 0; i < output_stracks.size(); i++){
    //        vector<float> tlwh = output_stracks[i].tlwh;
    //        bool vertical = tlwh[2] / tlwh[3] > 1.6;
    //        if (tlwh[2] * tlwh[3] > 20 && !vertical){
    //            Scalar s = tracker->get_color(output_stracks[i].track_id);
    //            putText(frame, format("%d", output_stracks[i].track_id), Point(tlwh[0], tlwh[1] - 5),
    //                    0, 0.6, Scalar(0, 0, 255), 2, LINE_AA);
    //            rectangle(frame, Rect(tlwh[0], tlwh[1], tlwh[2], tlwh[3]), s, 2);
    //            rect_1 = Rect(tlwh[0], tlwh[1], tlwh[2], tlwh[3]);
    //        }
    //    }
}

