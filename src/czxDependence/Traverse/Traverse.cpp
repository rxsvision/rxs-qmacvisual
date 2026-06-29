#include"Traverse.h"

// 设置剩余区域的颜色
void BoxImageProcessor::setBorderColor(const cv::Scalar& color) {
    borderColor_ = color;
}
// 处理图像的接口函数，用户需要传入图像和处理函数
cv::Mat BoxImageProcessor::processImage(const cv::Mat& image, const std::function<void(cv::Mat&, cv::Mat&)>& processFunction)
{
    if (image.empty()) {
        throw std::invalid_argument("Input image is empty.");
    }

    int imgWidth = image.cols;
    int imgHeight = image.rows;

    // 创建一张空白图像，初始化为指定的边界颜色
    resultImage_ = cv::Mat(imgHeight, imgWidth, image.type(), borderColor_);

    // 计算处理的起始位置和结束位置
    int maxX = (imgWidth - boxWidth_);
    int maxY = (imgHeight - boxHeight_);

    for (int y = 0; y <= maxY; y += boxHeight_) {
        for (int x = 0; x <= maxX; x += boxWidth_) {
            // 计算子图像的矩形区域
            cv::Rect rect(x, y, boxWidth_, boxHeight_);

            // 提取子图像
            cv::Mat subImage = image(rect);

            // 在空白图像中提取子图像区域
            cv::Mat resultSubImage = resultImage_(rect);

            // 调用用户定义的处理函数
            processFunction(subImage, resultSubImage);
        }
    }
    return resultImage_;
}

cv::Mat BoxImageProcessor::processImage(const cv::Mat& image, const std::function<cv::Mat(cv::Mat&)>& processFunction)
{
    if (image.empty()) {
        throw std::invalid_argument("Input image is empty.");
    }

    int imgWidth = image.cols;
    int imgHeight = image.rows;

    // 创建一张空白图像，初始化为指定的边界颜色
    resultImage_ = cv::Mat(imgHeight, imgWidth, image.type(), borderColor_);

    // 计算处理的起始位置和结束位置
    int maxX = (imgWidth - boxWidth_);
    int maxY = (imgHeight - boxHeight_);

    for (int y = 0; y <= maxY; y += boxHeight_) {
        for (int x = 0; x <= maxX; x += boxWidth_) {
            // 计算子图像的矩形区域
            cv::Rect rect(x, y, boxWidth_, boxHeight_);

            // 提取子图像
            cv::Mat subImage = image(rect);

            // 在空白图像中提取子图像区域
            cv::Mat resultSubImage = resultImage_(rect);

            // 调用用户定义的处理函数
            processFunction(subImage).copyTo(resultSubImage);
        }
    }
    return resultImage_;
}

cv::Mat BoxImageProcessor::getResultImage() const {
    return resultImage_;
}

void BoxImageProcessor::reshape(int boxWidth, int boxHeight)
{
    boxWidth_ = boxWidth;
    boxHeight_ = boxHeight;
}




ConvImageProcessor::ConvImageProcessor(const cv::Mat& inputImage, int boxHeight, int boxWidth, int stepVertical, int stepHorizontal, cv::Scalar color)
    : image(inputImage), stepY(stepVertical), stepX(stepHorizontal){
    // 检查输入图像是否为单通道
    if (image.channels() != 1) {
        throw std::invalid_argument("Input image must be single-channel.");
    }
    if (boxHeight % 2 == 0) boxHeight++;
    if (boxWidth % 2 == 0) boxWidth++;
    // 默认正方形盒子
    boxSizeHeight = boxHeight;
    boxSizeWidth = boxWidth;

    resultImage = cv::Mat(image.size(), CV_8UC1, color);

}

void ConvImageProcessor::process() {
    if (!processBoxRegion) {
        throw std::runtime_error("Processing function not registered.");
    }

    // 遍历图像
    for (int y = 0; y + boxSizeHeight <= image.rows; y += stepY) {
        for (int x = 0; x + boxSizeWidth <= image.cols; x += stepX) {
            // 提取盒子区域
            cv::Rect box(x, y, boxSizeWidth, boxSizeHeight);
            cv::Mat boxRegion = image(box);

            // 调用注册的处理函数
            uint8_t result = processBoxRegion(boxRegion);

            // 计算盒子中心坐标
            int centerY = y + boxSizeHeight / 2;
            int centerX = x + boxSizeWidth / 2;

            // 确保中心坐标在图像范围内
            if (centerY < image.rows && centerX < image.cols) {
                resultImage.at<uint8_t>(centerY, centerX) = result;
            }
        }
    }
}

void ConvImageProcessor::setColor(cv::Scalar newColor) {
    resultImage = cv::Mat(image.size(), CV_8UC1, newColor);
}

cv::Mat ConvImageProcessor::getResultImage() const {
    return resultImage;
}

void ConvImageProcessor::registerProcessor(std::function<uchar(const cv::Mat&)> processor) {
    processBoxRegion = processor;
}
