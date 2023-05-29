#include "utils.h"
/**
 * @brief utils 构造函数
 * @param
 */
utils::utils(QObject *parent)
    : QObject{parent}
{

}

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
