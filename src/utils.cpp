#include "utils.h"
#include "BYTETracker.h"
#include "lapjv.h"

///**
// * @brief utils 构造函数
// * @param
// */
//utils::utils(QObject *parent)
//    : QObject{parent}
//{

//}




/**
 * @brief 将[xmin, xmax]归一化到[a, b]区间
 * @param value
 * @return
 */
int Normalization(int value) {
    int ymin = 500;
    int ymax = 2500;
    int xmin = 0;
    int xmax = 270;
    int y = (ymax - ymin) * (value - xmin) / (xmax - xmin) + ymin;
    return y;
}

/**
 * @brief QImage转Mat
 * @param image
 * @return
 */
cv::Mat QImageTocvMat(const QImage &image)
{
    cv::Mat mat;
    switch(image.format())
    {
    case QImage::Format_Grayscale8: //灰度图，每个像素点1个字节（8位）
    case QImage::Format_Indexed8: //Mat构造：行数，列数，存储结构，数据，step每行多少字节
                mat = cv::Mat(image.height(), image.width(), CV_8UC1, (void*)image.constBits(), image.bytesPerLine());
    break;
    case QImage::Format_ARGB32:
    case QImage::Format_RGB32:
    case QImage::Format_ARGB32_Premultiplied:
                mat = cv::Mat(image.height(), image.width(), CV_8UC4, (void*)image.constBits(), image.bytesPerLine());
    break;
    case QImage::Format_RGB888: //RR,GG,BB字节顺序存储
                mat = cv::Mat(image.height(), image.width(), CV_8UC3, (void*)image.constBits(), image.bytesPerLine());
                cv::cvtColor(mat, mat, cv::COLOR_RGB2BGR); //opencv需要转为BGR的字节顺序
        break;
    }
    return mat;
}

/**
 * @brief Mat转QImage
 * @param mat
 * @return
 */
QImage cvMatToQImage(const cv::Mat& mat)
{
    switch (mat.type()) {
    case CV_8UC1:{
        QImage image(mat.cols, mat.rows, QImage::Format_Indexed8);

        image.setColorCount(256);
        for(int i = 0; i < 256; i++){
            image.setColor(i, qRgb(i, i, i));
        }

        uchar *pSrc = mat.data;
        for(int row = 0; row < mat.rows; row ++){
            uchar *pDest = image.scanLine(row);
            memcpy(pDest, pSrc, mat.cols);
            pSrc += mat.step;
        }

        return image;
    }
        break;
    case CV_8UC3:{
        const uchar *pSrc = (const uchar*)mat.data;
        QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);

        return image.rgbSwapped();
    }
        break;
    case CV_8UC4:{
        const uchar *pSrc = (const uchar*)mat.data;
        QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_ARGB32);

        return image.copy();
    }
        break;
    default:
        break;
    }

    return QImage();
}



//int findCameraVideoPath(char *pid, char *vid, char *dpath)
//{
//  struct udev *udev = NULL;
//  struct udev_enumerate *udev_enumerate = NULL;
//  struct udev_list_entry *list_entry = NULL;
//  int count = 0;
//  int flag = 0;
//  char devName[128]={0};

//  udev = udev_new();
//  if(udev == NULL)
//  {
//    //tips
//    return 0;
//  }
//  udev_enumerate = udev_enumerate_new(udev);
//  if(udev_enumerate == NULL)
//  {
//    //tips
//    return 0;
//  }

//  udev_enumerate_add_match_subsystem(udev_enumerate, "video4linux");
//  udev_enumerate_scan_devices(udev_enumerate);
//  udev_list_entry_foreach(list_entry, udev_enumerate_get_list_entry(udev_enumerate))
// {
//    struct udev_device *device;
//    device = udev_device_new_from_syspath(udev_enumerate_get_udev(udev_enumerate), udev_list_entry_get_name(list_entry));
//    if(device!=NULL)
//    {
//        //tips
//            );

//    if(udev_device_get_property_value(device, "ID_VENDOR_ID")!=NULL &&
//           udev_device_get_property_value(device, "ID_MODEL_ID")!=NULL &&
//           !strcmp(vid, udev_device_get_property_value(device, "ID_VENDOR_ID")) &&
//           !strcmp(pid, udev_device_get_property_value(device, "ID_MODEL_ID")))
//        {
//        sprintf(devName,"/dev/video%s", udev_device_get_sysnum(device));
//                flag = 1;
//        count++;
//        }
//    udev_device_unref(device);

//    }
//    else
//    {
//        //tips
//    }
//  }

//   if(flag!=0)
//   {
//      if(strlen(devName)>0)
//        memcpy(dpath, devName, strlen(devName));
//   }

//    udev_enumerate_unref(udev_enumerate);
//    udev_unref(udev);

//    return flag;
//}



vector<STrack*> BYTETracker::joint_stracks(vector<STrack*> &tlista, vector<STrack> &tlistb)
{
    map<int, int> exists;
    vector<STrack*> res;
    for (int i = 0; i < tlista.size(); i++)
    {
        exists.insert(pair<int, int>(tlista[i]->track_id, 1));
        res.push_back(tlista[i]);
    }
    for (int i = 0; i < tlistb.size(); i++)
    {
        int tid = tlistb[i].track_id;
        if (!exists[tid] || exists.count(tid) == 0)
        {
            exists[tid] = 1;
            res.push_back(&tlistb[i]);
        }
    }
    return res;
}

vector<STrack> BYTETracker::joint_stracks(vector<STrack> &tlista, vector<STrack> &tlistb)
{
    map<int, int> exists;
    vector<STrack> res;
    for (int i = 0; i < tlista.size(); i++)
    {
        exists.insert(pair<int, int>(tlista[i].track_id, 1));
        res.push_back(tlista[i]);
    }
    for (int i = 0; i < tlistb.size(); i++)
    {
        int tid = tlistb[i].track_id;
        if (!exists[tid] || exists.count(tid) == 0)
        {
            exists[tid] = 1;
            res.push_back(tlistb[i]);
        }
    }
    return res;
}

vector<STrack> BYTETracker::sub_stracks(vector<STrack> &tlista, vector<STrack> &tlistb)
{
    map<int, STrack> stracks;
    for (int i = 0; i < tlista.size(); i++)
    {
        stracks.insert(pair<int, STrack>(tlista[i].track_id, tlista[i]));
    }
    for (int i = 0; i < tlistb.size(); i++)
    {
        int tid = tlistb[i].track_id;
        if (stracks.count(tid) != 0)
        {
            stracks.erase(tid);
        }
    }

    vector<STrack> res;
    std::map<int, STrack>::iterator  it;
    for (it = stracks.begin(); it != stracks.end(); ++it)
    {
        res.push_back(it->second);
    }

    return res;
}

void BYTETracker::remove_duplicate_stracks(vector<STrack> &resa, vector<STrack> &resb, vector<STrack> &stracksa, vector<STrack> &stracksb)
{
    vector<vector<float> > pdist = iou_distance(stracksa, stracksb);
    vector<pair<int, int> > pairs;
    for (int i = 0; i < pdist.size(); i++)
    {
        for (int j = 0; j < pdist[i].size(); j++)
        {
            if (pdist[i][j] < 0.15)
            {
                pairs.push_back(pair<int, int>(i, j));
            }
        }
    }

    vector<int> dupa, dupb;
    for (int i = 0; i < pairs.size(); i++)
    {
        int timep = stracksa[pairs[i].first].frame_id - stracksa[pairs[i].first].start_frame;
        int timeq = stracksb[pairs[i].second].frame_id - stracksb[pairs[i].second].start_frame;
        if (timep > timeq)
            dupb.push_back(pairs[i].second);
        else
            dupa.push_back(pairs[i].first);
    }

    for (int i = 0; i < stracksa.size(); i++)
    {
        vector<int>::iterator iter = find(dupa.begin(), dupa.end(), i);
        if (iter == dupa.end())
        {
            resa.push_back(stracksa[i]);
        }
    }

    for (int i = 0; i < stracksb.size(); i++)
    {
        vector<int>::iterator iter = find(dupb.begin(), dupb.end(), i);
        if (iter == dupb.end())
        {
            resb.push_back(stracksb[i]);
        }
    }
}

void BYTETracker::linear_assignment(vector<vector<float> > &cost_matrix, int cost_matrix_size, int cost_matrix_size_size, float thresh,
    vector<vector<int> > &matches, vector<int> &unmatched_a, vector<int> &unmatched_b)
{
    if (cost_matrix.size() == 0)
    {
        for (int i = 0; i < cost_matrix_size; i++)
        {
            unmatched_a.push_back(i);
        }
        for (int i = 0; i < cost_matrix_size_size; i++)
        {
            unmatched_b.push_back(i);
        }
        return;
    }

    vector<int> rowsol; vector<int> colsol;
    float c = lapjv(cost_matrix, rowsol, colsol, true, thresh);
    for (int i = 0; i < rowsol.size(); i++)
    {
        if (rowsol[i] >= 0)
        {
            vector<int> match;
            match.push_back(i);
            match.push_back(rowsol[i]);
            matches.push_back(match);
        }
        else
        {
            unmatched_a.push_back(i);
        }
    }

    for (int i = 0; i < colsol.size(); i++)
    {
        if (colsol[i] < 0)
        {
            unmatched_b.push_back(i);
        }
    }
}

vector<vector<float> > BYTETracker::ious(vector<vector<float> > &atlbrs, vector<vector<float> > &btlbrs)
{
    vector<vector<float> > ious;
    if (atlbrs.size()*btlbrs.size() == 0)
        return ious;

    ious.resize(atlbrs.size());
    for (int i = 0; i < ious.size(); i++)
    {
        ious[i].resize(btlbrs.size());
    }

    //bbox_ious
    for (int k = 0; k < btlbrs.size(); k++)
    {
        vector<float> ious_tmp;
        float box_area = (btlbrs[k][2] - btlbrs[k][0] + 1)*(btlbrs[k][3] - btlbrs[k][1] + 1);
        for (int n = 0; n < atlbrs.size(); n++)
        {
            float iw = min(atlbrs[n][2], btlbrs[k][2]) - max(atlbrs[n][0], btlbrs[k][0]) + 1;
            if (iw > 0)
            {
                float ih = min(atlbrs[n][3], btlbrs[k][3]) - max(atlbrs[n][1], btlbrs[k][1]) + 1;
                if(ih > 0)
                {
                    float ua = (atlbrs[n][2] - atlbrs[n][0] + 1)*(atlbrs[n][3] - atlbrs[n][1] + 1) + box_area - iw * ih;
                    ious[n][k] = iw * ih / ua;
                }
                else
                {
                    ious[n][k] = 0.0;
                }
            }
            else
            {
                ious[n][k] = 0.0;
            }
        }
    }

    return ious;
}

vector<vector<float> > BYTETracker::iou_distance(vector<STrack*> &atracks, vector<STrack> &btracks, int &dist_size, int &dist_size_size)
{
    vector<vector<float> > cost_matrix;
    if (atracks.size() * btracks.size() == 0)
    {
        dist_size = atracks.size();
        dist_size_size = btracks.size();
        return cost_matrix;
    }
    vector<vector<float> > atlbrs, btlbrs;
    for (int i = 0; i < atracks.size(); i++)
    {
        atlbrs.push_back(atracks[i]->tlbr);
    }
    for (int i = 0; i < btracks.size(); i++)
    {
        btlbrs.push_back(btracks[i].tlbr);
    }

    dist_size = atracks.size();
    dist_size_size = btracks.size();

    vector<vector<float> > _ious = ious(atlbrs, btlbrs);

    for (int i = 0; i < _ious.size();i++)
    {
        vector<float> _iou;
        for (int j = 0; j < _ious[i].size(); j++)
        {
            _iou.push_back(1 - _ious[i][j]);
        }
        cost_matrix.push_back(_iou);
    }

    return cost_matrix;
}

vector<vector<float> > BYTETracker::iou_distance(vector<STrack> &atracks, vector<STrack> &btracks)
{
    vector<vector<float> > atlbrs, btlbrs;
    for (int i = 0; i < atracks.size(); i++)
    {
        atlbrs.push_back(atracks[i].tlbr);
    }
    for (int i = 0; i < btracks.size(); i++)
    {
        btlbrs.push_back(btracks[i].tlbr);
    }

    vector<vector<float> > _ious = ious(atlbrs, btlbrs);
    vector<vector<float> > cost_matrix;
    for (int i = 0; i < _ious.size(); i++)
    {
        vector<float> _iou;
        for (int j = 0; j < _ious[i].size(); j++)
        {
            _iou.push_back(1 - _ious[i][j]);
        }
        cost_matrix.push_back(_iou);
    }

    return cost_matrix;
}

double BYTETracker::lapjv(const vector<vector<float> > &cost, vector<int> &rowsol, vector<int> &colsol,
    bool extend_cost, float cost_limit, bool return_cost)
{
    vector<vector<float> > cost_c;
    cost_c.assign(cost.begin(), cost.end());

    vector<vector<float> > cost_c_extended;

    int n_rows = cost.size();
    int n_cols = cost[0].size();
    rowsol.resize(n_rows);
    colsol.resize(n_cols);

    int n = 0;
    if (n_rows == n_cols)
    {
        n = n_rows;
    }
    else
    {
        if (!extend_cost)
        {
            cout << "set extend_cost=True" << endl;
            system("pause");
            exit(0);
        }
    }

    if (extend_cost || cost_limit < LONG_MAX)
    {
        n = n_rows + n_cols;
        cost_c_extended.resize(n);
        for (int i = 0; i < cost_c_extended.size(); i++)
            cost_c_extended[i].resize(n);

        if (cost_limit < LONG_MAX)
        {
            for (int i = 0; i < cost_c_extended.size(); i++)
            {
                for (int j = 0; j < cost_c_extended[i].size(); j++)
                {
                    cost_c_extended[i][j] = cost_limit / 2.0;
                }
            }
        }
        else
        {
            float cost_max = -1;
            for (int i = 0; i < cost_c.size(); i++)
            {
                for (int j = 0; j < cost_c[i].size(); j++)
                {
                    if (cost_c[i][j] > cost_max)
                        cost_max = cost_c[i][j];
                }
            }
            for (int i = 0; i < cost_c_extended.size(); i++)
            {
                for (int j = 0; j < cost_c_extended[i].size(); j++)
                {
                    cost_c_extended[i][j] = cost_max + 1;
                }
            }
        }

        for (int i = n_rows; i < cost_c_extended.size(); i++)
        {
            for (int j = n_cols; j < cost_c_extended[i].size(); j++)
            {
                cost_c_extended[i][j] = 0;
            }
        }
        for (int i = 0; i < n_rows; i++)
        {
            for (int j = 0; j < n_cols; j++)
            {
                cost_c_extended[i][j] = cost_c[i][j];
            }
        }

        cost_c.clear();
        cost_c.assign(cost_c_extended.begin(), cost_c_extended.end());
    }

    double **cost_ptr;
    cost_ptr = new double *[sizeof(double *) * n];
    for (int i = 0; i < n; i++)
        cost_ptr[i] = new double[sizeof(double) * n];

    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            cost_ptr[i][j] = cost_c[i][j];
        }
    }

    int* x_c = new int[sizeof(int) * n];
    int *y_c = new int[sizeof(int) * n];

    int ret = lapjv_internal(n, cost_ptr, x_c, y_c);
    if (ret != 0)
    {
        cout << "Calculate Wrong!" << endl;
        system("pause");
        exit(0);
    }

    double opt = 0.0;

    if (n != n_rows)
    {
        for (int i = 0; i < n; i++)
        {
            if (x_c[i] >= n_cols)
                x_c[i] = -1;
            if (y_c[i] >= n_rows)
                y_c[i] = -1;
        }
        for (int i = 0; i < n_rows; i++)
        {
            rowsol[i] = x_c[i];
        }
        for (int i = 0; i < n_cols; i++)
        {
            colsol[i] = y_c[i];
        }

        if (return_cost)
        {
            for (int i = 0; i < rowsol.size(); i++)
            {
                if (rowsol[i] != -1)
                {
                    //cout << i << "\t" << rowsol[i] << "\t" << cost_ptr[i][rowsol[i]] << endl;
                    opt += cost_ptr[i][rowsol[i]];
                }
            }
        }
    }
    else if (return_cost)
    {
        for (int i = 0; i < rowsol.size(); i++)
        {
            opt += cost_ptr[i][rowsol[i]];
        }
    }

    for (int i = 0; i < n; i++)
    {
        delete[]cost_ptr[i];
    }
    delete[]cost_ptr;
    delete[]x_c;
    delete[]y_c;

    return opt;
}

Scalar BYTETracker::get_color(int idx)
{
    idx += 3;
    return Scalar(37 * idx % 255, 17 * idx % 255, 29 * idx % 255);
}
