//#ifndef YOLOV8ONNX_H
//#define YOLOV8ONNX_H
//#include <QObject>
//#include <onnxruntime_cxx_api.h>
//#include <codecvt>
//class YoloV8Onnx : public QObject
//{
//    Q_OBJECT
//public:
//    explicit YoloV8Onnx(QObject *parent = nullptr);


//    static constexpr const int width_ = 28;
//      static constexpr const int height_ = 28;
//private:
//  Ort::Env env;
//  Ort::Session session_{env, L"mnist.onnx", Ort::SessionOptions{nullptr}};

//  Ort::Value input_tensor_{nullptr};
//  std::array<int64_t, 4> input_shape_{1, 1, width_, height_};

//  Ort::Value output_tensor_{nullptr};
//  std::array<int64_t, 2> output_shape_{1, 10};
//};

//#endif // YOLOV8ONNX_H
