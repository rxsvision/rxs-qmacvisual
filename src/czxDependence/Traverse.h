#pragma once
#include"opencv2/opencv.hpp"


//无缝遍历，步长和盒子同大
class BoxImageProcessor
{
public:
    // 构造函数
    BoxImageProcessor(int boxWidth, int boxHeight, const cv::Scalar& borderColor = cv::Scalar(0))
        : boxWidth_(boxWidth), boxHeight_(boxHeight), borderColor_(borderColor) {}

    // 设置剩余区域的颜色
    void setBorderColor(const cv::Scalar& color);

    // 处理图像的接口函数，用户需要传入图像和处理函数
    cv::Mat processImage(const cv::Mat& image, const std::function<void(cv::Mat&, cv::Mat&)>& processFunction);
    cv::Mat processImage(const cv::Mat& image, const std::function<cv::Mat(cv::Mat&)>& processFunction);
    cv::Mat getResultImage() const;
    void reshape(int boxWidth, int boxHeight);
private:
    int boxWidth_;   // 盒子宽度
    int boxHeight_;  // 盒子高度
    cv::Scalar borderColor_; // 剩余区域的颜色
    cv::Mat resultImage_;    // 存储处理后的图像

};



class ConvImageProcessor
{
public:
    // 构造函数，允许默认正方形盒子大小，也可以设置不同的长宽
    ConvImageProcessor(const cv::Mat& inputImage, int boxHeight = 3, int boxWidth = 3, int stepVertical = 1, int stepHorizontal = 1, cv::Scalar color = cv::Scalar(0));

    // 遍历图像并处理每个盒子区域
    void process();

    // 设置纯色图的颜色
    void setColor(cv::Scalar newColor);

    // 获取结果图像
    cv::Mat getResultImage() const;

    // 注册处理函数
    void registerProcessor(std::function<uchar(const cv::Mat&)> processor);

private:
    cv::Mat image;               // 输入图像
    cv::Mat resultImage;         // 处理后的结果图像
    int boxSizeHeight;           // 盒子的高度
    int boxSizeWidth;            // 盒子的宽度
    int stepY;                   // 垂直方向的步长
    int stepX;                   // 水平方向的步长
    std::function<uchar(const cv::Mat&)> processBoxRegion; // 注册的处理函数
};