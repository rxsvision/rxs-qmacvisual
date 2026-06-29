#include"czxTool2D.h"
#include <numeric>
#include<unordered_map>
#include"Traverse.h"
#include"2D/blobAnalyze.h"


using namespace cv;
using namespace std;

Mat brightnessCorrection(const cv::Mat& grayImage, int kernelSize)
{
    if (kernelSize % 2 == 0) kernelSize++;
    // Apply Gaussian blur to the grayscale image to obtain the background
    cv::Mat background;
    cv::GaussianBlur(grayImage, background, cv::Size(kernelSize, kernelSize), 0);
    //cv::blur(grayImage, background, cv::Size(kernelSize, kernelSize));

    cv::Mat correctedImage = grayImage - background +cv::mean(grayImage)[0];
    return correctedImage;
}

cv::Mat stdConv(const cv::Mat& grayImage, int kernelSize, int alpha)
{
    cv::Mat stdImage = cv::Mat::zeros(grayImage.size(), CV_8UC1);

    #pragma omp parallel for
    for (int y = 0; y < grayImage.rows; ++y) 
    {
        for (int x = 0; x < grayImage.cols; ++x) 
        {
            cv::Rect region = cv::Rect(std::max(0, x - kernelSize / 2), std::max(0, y - kernelSize / 2),
                std::min(kernelSize, grayImage.cols - x + kernelSize / 2),
                std::min(kernelSize, grayImage.rows - y + kernelSize / 2));

            cv::Mat localRegion = grayImage(region);

            cv::Scalar mean, stddev;
            cv::meanStdDev(localRegion, mean, stddev);
            double ret = stddev[0] * alpha;
            stdImage.at<uchar>(y, x) = ret;
        }
    }
    return stdImage;
}

cv::Mat stdConvDif(const cv::Mat& grayImage, int kernelSize, int kernelSize_min)
{
    cv::Mat stdImage = cv::Mat::zeros(grayImage.size(), CV_8UC1);

    #pragma omp parallel for
    for (int y = 0; y < grayImage.rows; ++y)
    {
        for (int x = 0; x < grayImage.cols; ++x) {

            double std, std_min;
            {
                cv::Rect region = cv::Rect(std::max(0, x - kernelSize / 2), std::max(0, y - kernelSize / 2),
                    std::min(kernelSize, grayImage.cols - x + kernelSize / 2),
                    std::min(kernelSize, grayImage.rows - y + kernelSize / 2));

                cv::Mat localRegion = grayImage(region);

                cv::Scalar mean, stddev;
                cv::meanStdDev(localRegion, mean, stddev);
                std = stddev[0];
            }
            {
                cv::Rect region = cv::Rect(std::max(0, x - kernelSize_min / 2), std::max(0, y - kernelSize_min / 2),
                    std::min(kernelSize_min, grayImage.cols - x + kernelSize_min / 2),
                    std::min(kernelSize_min, grayImage.rows - y + kernelSize_min / 2));

                cv::Mat localRegion = grayImage(region);

                cv::Scalar mean, stddev;
                cv::meanStdDev(localRegion, mean, stddev);
                std_min = stddev[0];
            }
            stdImage.at<uchar>(y, x) = static_cast<int>(std - std_min);
        }
    }
    return stdImage;
}

cv::Mat meanConvDif(const cv::Mat& grayImage, int kernelSize, int kernelSize_small)
{
    cv::Mat stdImage = cv::Mat::zeros(grayImage.size(), CV_8UC1);

    #pragma omp parallel for
    for (int y = 0; y < grayImage.rows; ++y)
    {
        for (int x = 0; x < grayImage.cols; ++x) {

            double mean_big, mean_small;
            {
                cv::Rect region = cv::Rect(std::max(0, x - kernelSize / 2), std::max(0, y - kernelSize / 2),
                    std::min(kernelSize, grayImage.cols - x + kernelSize / 2),
                    std::min(kernelSize, grayImage.rows - y + kernelSize / 2));

                cv::Mat localRegion = grayImage(region);

                cv::Scalar mean, stddev;
                cv::meanStdDev(localRegion, mean, stddev);
                mean_big = mean[0];
            }
            {
                cv::Rect region = cv::Rect(std::max(0, x - kernelSize_small / 2), std::max(0, y - kernelSize_small / 2),
                    std::min(kernelSize_small, grayImage.cols - x + kernelSize_small / 2),
                    std::min(kernelSize_small, grayImage.rows - y + kernelSize_small / 2));

                cv::Mat localRegion = grayImage(region);

                cv::Scalar mean, stddev;
                cv::meanStdDev(localRegion, mean, stddev);
                mean_small = mean[0];
            }
            stdImage.at<uchar>(y, x) = max(static_cast<int>(5*(mean_small - mean_big)), 0);
        }
    }
    return stdImage;
}

cv::Mat binaryStd(const cv::Mat& grayImage, int kernelSize, double varianceThreshold) {
    
    cv::Mat binaryImage = cv::Mat::zeros(grayImage.size(), CV_8UC1);


    #pragma omp parallel for
    for (int y = 0; y < grayImage.rows; ++y) {
        for (int x = 0; x < grayImage.cols; ++x) {
            cv::Rect region = cv::Rect(std::max(0, x - kernelSize / 2), std::max(0, y - kernelSize / 2),
                std::min(kernelSize, grayImage.cols - x + kernelSize / 2),
                std::min(kernelSize, grayImage.rows - y + kernelSize / 2));

            cv::Mat localRegion = grayImage(region);

            cv::Scalar mean, stddev;
            cv::meanStdDev(localRegion, mean, stddev);

            if (stddev[0] > varianceThreshold) {
                binaryImage.at<uchar>(y, x) = 255; // 白色
            }
            else {
                binaryImage.at<uchar>(y, x) = 0; // 黑色
            }
        }
    }
    return binaryImage;
}

cv::Mat minusMean(const cv::Mat& inputImage) {
    // 检查输入图像是否为空
    if (inputImage.empty()) {
        std::cerr << "Input image is empty" << std::endl;
        return cv::Mat();
    }

    // 计算图像的均值
    cv::Scalar meanScalar = cv::mean(inputImage);
    int mean = static_cast<int>(meanScalar[0]);

    // 创建一个和输入图像相同大小的图像来存储结果
    cv::Mat result = inputImage.clone();

    // 从每个像素值中减去均值
    result -= mean;

    // 将小于零的像素值置为零
    result.setTo(0, result < 0);

    return result;
}

// 计算一个像素点和其上下左右间隔n像素的差值，并计算这些差值的均值
double calculatePixelDifferenceMean(const cv::Mat& image, cv::Point center, int n) {
    // 确保中心点在图像内部
    if (center.x < 0 || center.x >= image.cols || center.y < 0 || center.y >= image.rows) {
        std::cerr << "中心点超出图像边界！" << std::endl;
        return 0.0;
    }

    std::vector<int> differences;

    uchar centerValue = image.at<uchar>(center);

    // 检查上方像素
    if (center.y - n >= 0) {
        uchar upValue = image.at<uchar>(center.y - n, center.x);
        differences.push_back(max(centerValue - upValue, 0));
    }

    // 检查下方像素
    if (center.y + n < image.rows) {
        uchar downValue = image.at<uchar>(center.y + n, center.x);
        differences.push_back(max(centerValue - downValue, 0));
    }

    // 检查左方像素
    if (center.x - n >= 0) {
        uchar leftValue = image.at<uchar>(center.y, center.x - n);
        differences.push_back(max(centerValue - leftValue, 0));
    }

    // 检查右方像素
    if (center.x + n < image.cols) {
        uchar rightValue = image.at<uchar>(center.y, center.x + n);
        differences.push_back(max(centerValue - rightValue, 0));
    }

    // 计算差值的均值
    if (!differences.empty()) {
        auto max = std::max_element(differences.begin(), differences.end());        
        return *max;
    }
    else {
        return 0.0;
    }
}

cv::Mat difImage(const cv::Mat& image, int n)
{
    // 创建结果图像
    cv::Mat result(image.size(), CV_64F, cv::Scalar(0));

    // 遍历图像的每个像素
    for (int y = 0; y < image.rows; ++y) {
        for (int x = 0; x < image.cols; ++x) {
            cv::Point center(x, y);
            double meanDifference = calculatePixelDifferenceMean(image, center, n);
            result.at<double>(y, x) = meanDifference;
        }
    }

    return result;
}

std::vector<cv::Mat> extractBinaryImages(const cv::Mat& grayImage, int step) {
    // 检查输入是否为灰度图像
    if (grayImage.empty() || grayImage.channels() != 1) {
        throw std::invalid_argument("输入图像必须为灰度图像");
    }

    // 灰度图像列表，用于存储结果
    std::vector<cv::Mat> binaryImages;

    // 遍历每个范围
    for (int i = 0; i <= 255; i += step) {
        cv::Mat binaryImage = cv::Mat::zeros(grayImage.size(), CV_8U);
        cv::inRange(grayImage, cv::Scalar(i), cv::Scalar(i + step-1), binaryImage);
        binaryImages.push_back(binaryImage);
    }

    return binaryImages;
}

cv::Mat maxFilter(const cv::Mat& src, int ksize) {
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(ksize, ksize));

    // 执行最大值滤波（膨胀操作）
    cv::Mat maxFiltered;
    cv::dilate(src, maxFiltered, kernel);

    return maxFiltered;
}

//去除异于周围的点
cv::Mat noiseConv(const cv::Mat& grayImage, int kernelSize)
{
    const int threshold = 10;
    cv::Mat retImage = grayImage.clone();

    #pragma omp parallel for
    for (int y = 0; y < grayImage.rows; ++y)
    {
        for (int x = 0; x < grayImage.cols; ++x)
        {
            cv::Rect region = cv::Rect(std::max(0, x - kernelSize / 2), std::max(0, y - kernelSize / 2),
                std::min(kernelSize, grayImage.cols - x + kernelSize / 2),
                std::min(kernelSize, grayImage.rows - y + kernelSize / 2));

            cv::Mat localRegion = grayImage(region);

            // 将结果的所有元素求和
            double sum = cv::sum(localRegion)[0] - grayImage.at<uchar>(y, x);

            double mean = sum / (localRegion.cols * localRegion.rows - 1);
            if (abs(mean- grayImage.at<uchar>(y, x)) > threshold)
            {
                retImage.at<uchar>(y, x) = mean;
            }
        }
    }
    return retImage;
}

//统计异于周围点数目
cv::Mat countConv(const cv::Mat& grayImage, int kernelSize)
{
    const int threshold = 20;
    cv::Mat conutImage = cv::Mat::zeros(grayImage.size(), CV_8UC1);

    #pragma omp parallel for
    for (int y = 0; y < grayImage.rows; ++y)
    {
        for (int x = 0; x < grayImage.cols; ++x)
        {
            cv::Rect region = cv::Rect(std::max(0, x - kernelSize / 2), std::max(0, y - kernelSize / 2),
                std::min(kernelSize, grayImage.cols - x + kernelSize / 2),
                std::min(kernelSize, grayImage.rows - y + kernelSize / 2));

            cv::Mat localRegion = grayImage(region);

            // 创建一个与源图像大小相同的矩阵，并将其初始化为常数值
            cv::Mat constantMat = cv::Mat::ones(localRegion.size(), localRegion.type()) * grayImage.at<uchar>(y, x);

            // 计算图像与常数的绝对差
            cv::Mat absDiff;
            cv::absdiff(localRegion, constantMat, absDiff);

            // 统计绝对差值大于阈值的像素数目
            cv::Mat mask;
            cv::threshold(absDiff, mask, threshold, 255, cv::THRESH_BINARY);
            int count = cv::countNonZero(mask);

            conutImage.at<uchar>(y, x) = count;
        }
    }
    return conutImage;
}

cv::Mat applyLowPassFilter(cv::Mat& src, int radius) {
    // 将图像转换为浮点型
    cv::Mat srcFloat;
    src.convertTo(srcFloat, CV_32F);

    // 创建一个与源图像相同大小的复数图像用于存储傅里叶变换结果
    cv::Mat planes[] = { srcFloat, cv::Mat::zeros(srcFloat.size(), CV_32F) };
    cv::Mat complexImage;
    cv::merge(planes, 2, complexImage);

    // 执行傅里叶变换
    cv::dft(complexImage, complexImage);

    // 移动频谱图像中的低频分量到图像中心
    cv::Mat tmp;
    int cx = complexImage.cols / 2;
    int cy = complexImage.rows / 2;
    cv::Mat q0(complexImage, cv::Rect(0, 0, cx, cy));   // Top-Left
    cv::Mat q1(complexImage, cv::Rect(cx, 0, cx, cy));  // Top-Right
    cv::Mat q2(complexImage, cv::Rect(0, cy, cx, cy));  // Bottom-Left
    cv::Mat q3(complexImage, cv::Rect(cx, cy, cx, cy)); // Bottom-Right
    q0.copyTo(tmp);
    q3.copyTo(q0);
    tmp.copyTo(q3);
    q1.copyTo(tmp);
    q2.copyTo(q1);
    tmp.copyTo(q2);

    // 创建一个掩码，中心为1（保留低频），其余为0（滤除高频）
    cv::Mat mask = cv::Mat::zeros(complexImage.rows, complexImage.cols, CV_32F);
    cv::circle(mask, cv::Point(cx, cy), radius, cv::Scalar(1), -1);

    // 将掩码转换为复数形式，并与频率域图像相乘
    cv::Mat maskPlanes[] = { mask, mask };
    cv::Mat complexMask;
    cv::merge(maskPlanes, 2, complexMask);
    cv::mulSpectrums(complexImage, complexMask, complexImage, 0);

    // 移动频谱图像中的低频分量回到图像角落
    q0.copyTo(tmp);
    q3.copyTo(q0);
    tmp.copyTo(q3);
    q1.copyTo(tmp);
    q2.copyTo(q1);
    tmp.copyTo(q2);

    // 执行反傅里叶变换
    cv::idft(complexImage, complexImage, cv::DFT_SCALE | cv::DFT_REAL_OUTPUT);

    cv::Mat dst;
    // 提取实部
    cv::split(complexImage, planes);
    cv::normalize(planes[0], dst, 0, 255, cv::NORM_MINMAX);
    dst.convertTo(dst, CV_8U);
    return dst;
}

cv::Mat applyHighPassFilter(cv::Mat& src, int radius) {
    // 将图像转换为浮点型
    cv::Mat srcFloat;
    src.convertTo(srcFloat, CV_32F);

    // 创建一个与源图像相同大小的复数图像用于存储傅里叶变换结果
    cv::Mat planes[] = { srcFloat, cv::Mat::zeros(srcFloat.size(), CV_32F) };
    cv::Mat complexImage;
    cv::merge(planes, 2, complexImage);

    // 执行傅里叶变换
    cv::dft(complexImage, complexImage);

    // 移动频谱图像中的低频分量到图像中心
    cv::Mat tmp;
    int cx = complexImage.cols / 2;
    int cy = complexImage.rows / 2;
    cv::Mat q0(complexImage, cv::Rect(0, 0, cx, cy));   // Top-Left
    cv::Mat q1(complexImage, cv::Rect(cx, 0, cx, cy));  // Top-Right
    cv::Mat q2(complexImage, cv::Rect(0, cy, cx, cy));  // Bottom-Left
    cv::Mat q3(complexImage, cv::Rect(cx, cy, cx, cy)); // Bottom-Right
    q0.copyTo(tmp);
    q3.copyTo(q0);
    tmp.copyTo(q3);
    q1.copyTo(tmp);
    q2.copyTo(q1);
    tmp.copyTo(q2);

    //cv::split(complexImage, planes);
    //// 归一化实部和虚部到0-255范围
    //cv::Mat magI, real, imag;
    //real = planes[0];
    //real += cv::Scalar::all(1); // 防止对数变换中的log(0)
    //cv::log(real, real);
    //cv::normalize(real, real, 0, 255, cv::NORM_MINMAX);
    //// 将浮点数矩阵转换为8位无符号整型
    //real.convertTo(real, CV_8U);
    //return real;

    // 创建一个掩码，中心为0（去除低频），其余为1（保留高频）
    cv::Mat mask = cv::Mat::ones(complexImage.rows, complexImage.cols, CV_32F);
    cv::circle(mask, cv::Point(cx, cy), radius, cv::Scalar(0), -1);

    // 将掩码转换为复数形式，并与频率域图像相乘
    cv::Mat maskPlanes[] = { mask, mask };
    cv::Mat complexMask;
    cv::merge(maskPlanes, 2, complexMask);
    cv::mulSpectrums(complexImage, complexMask, complexImage, 0);

    // 移动频谱图像中的低频分量回到图像角落
    q0.copyTo(tmp);
    q3.copyTo(q0);
    tmp.copyTo(q3);
    q1.copyTo(tmp);
    q2.copyTo(q1);
    tmp.copyTo(q2);

    // 执行反傅里叶变换
    cv::idft(complexImage, complexImage, cv::DFT_SCALE | cv::DFT_REAL_OUTPUT);


    cv::Mat dst;
    // 提取实部
    cv::split(complexImage, planes);
    cv::normalize(planes[0], dst, 0, 255, cv::NORM_MINMAX);
    dst.convertTo(dst, CV_8U);
    return dst;
}

void showImg(const cv::Mat& img, std::string name)
{
    cv::namedWindow(name, cv::WINDOW_NORMAL);
    cv::imshow(name, img);
    cv::waitKey(0);
}

void saveFrequencySpectrum(const cv::Mat& complexI, const std::string& realFileName, const std::string& imagFileName) {
    // 分离复数频域图像的实部和虚部
    cv::Mat planes[2];
    cv::split(complexI, planes);

    // 归一化实部和虚部到0-255范围
    cv::Mat magI, real, imag;
    cv::normalize(planes[0], real, 0, 255, cv::NORM_MINMAX);
    cv::normalize(planes[1], imag, 0, 255, cv::NORM_MINMAX);

    // 将浮点数矩阵转换为8位无符号整型
    real.convertTo(real, CV_8U);
    imag.convertTo(imag, CV_8U);

    // 保存实部和虚部图像
    cv::imwrite(realFileName, real);
    cv::imwrite(imagFileName, imag);

    std::cout << "实部图像保存为: " << realFileName << std::endl;
    std::cout << "虚部图像保存为: " << imagFileName << std::endl;
}

// 泛洪填充算法，用于标记连通组件
void floodFill_(cv::Mat& image, cv::Mat& labels, int x, int y, int label, int threshold) 
{
    std::queue<cv::Point> q;
    q.push(cv::Point(x, y));
    int originalValue = image.at<uchar>(y, x);
    labels.at<int>(y, x) = label;

    while (!q.empty()) {
        cv::Point p = q.front();
        q.pop();

        for (int dy = -1; dy <= 1; ++dy) {
            for (int dx = -1; dx <= 1; ++dx) {
                int nx = p.x + dx;
                int ny = p.y + dy;

                if (nx >= 0 && nx < image.cols && ny >= 0 && ny < image.rows &&
                    labels.at<int>(ny, nx) == 0 &&
                    std::abs(image.at<uchar>(ny, nx) - originalValue) <= threshold) {

                    labels.at<int>(ny, nx) = label;
                    q.push(cv::Point(nx, ny));
                }
            }
        }
    }
}

// 函数：根据灰度值差异进行连通域分割
cv::Mat segmentConnectedComponents(cv::Mat& image, int threshold) {
    // 检查输入图像是否为灰度图
    if (image.channels() != 1 || image.type() != CV_8U) {
        std::cerr << "Input image must be a grayscale image." << std::endl;
        return cv::Mat();
    }

    // 初始化标签矩阵
    cv::Mat labels = cv::Mat::zeros(image.size(), CV_32S);
    int labelCount = 1;

    // 遍历图像，进行泛洪填充
    for (int y = 0; y < image.rows; ++y) {
        for (int x = 0; x < image.cols; ++x) {
            if (labels.at<int>(y, x) == 0) {  // 尚未标记的像素
                floodFill_(image, labels, x, y, labelCount, threshold);
                ++labelCount;
            }
        }
    }

    // 将标签映射到灰度值
    cv::Mat result = cv::Mat::zeros(image.size(), CV_8U);
    for (int y = 0; y < result.rows; ++y) {
        for (int x = 0; x < result.cols; ++x) {
            result.at<uchar>(y, x) = static_cast<uchar>(255 * (labels.at<int>(y, x) / double(labelCount)));
        }
    }

    return result;
}

std::vector<cv::Point> connectionDetect(cv::Mat& image, cv::Mat& labels, int x, int y, int label, int threshold)
{
    std::queue<cv::Point> q;
    q.push(cv::Point(x, y));
    labels.at<int>(y, x) = label;
    std::vector<cv::Point> connection_points;
    int originalValue = image.at<uchar>(y, x);

    while (!q.empty()) 
    {
        cv::Point p = q.front();
        q.pop();
        connection_points.push_back(cv::Point(p.x, p.y));

        for (int dy = -1; dy <= 1; ++dy) {
            for (int dx = -1; dx <= 1; ++dx) {
                int nx = p.x + dx;
                int ny = p.y + dy;

                if (nx >= 0 && nx < image.cols && ny >= 0 && ny < image.rows &&
                    labels.at<int>(ny, nx) == 0 
                    && std::abs(image.at<uchar>(ny, nx) - originalValue) <= threshold
                    ) 
                {
                    //std::cout << (int)image.at<uchar>(ny, nx) << std::endl;
                    //std::cout << (int)image.at<uchar>(p.y, p.x) << std::endl;
                    //std::cout << threshold << std::endl;
                    labels.at<int>(ny, nx) = label;
                    q.push(cv::Point(nx, ny));
                }
            }
        }
    }
    return connection_points;
}

cv::Mat segmentConnectedComponents(cv::Mat& image, int threshold, int min_size, int max_size)
{
    // 检查输入图像是否为灰度图
    if (image.channels() != 1 || image.type() != CV_8U) {
        std::cerr << "Input image must be a grayscale image." << std::endl;
        return cv::Mat();
    }
    // 初始化标签矩阵
    cv::Mat labels = cv::Mat::zeros(image.size(), CV_32S);
    int labelCount = 1;
    for (int y = 0; y < image.rows; ++y) {
        for (int x = 0; x < image.cols; ++x) {
            if (labels.at<int>(y, x) == 0) {  // 尚未标记的像素
                auto connection = connectionDetect(image, labels, x, y, labelCount, threshold);
                if (connection.size() > max_size || connection.size() < min_size)
                {
                    for (auto& p : connection)
                    {
                        labels.at<int>(p.y, p.x) = -1;
                    }
                    labelCount--;
                }
                ++labelCount;
            }
        }
    }
    // 将标签映射到灰度值
    cv::Mat result = cv::Mat::zeros(image.size(), CV_8U);
    for (int y = 0; y < result.rows; ++y) {
        for (int x = 0; x < result.cols; ++x) {
            result.at<uchar>(y, x) = static_cast<uchar>(255 * (labels.at<int>(y, x) / double(labelCount)));
        }
    }

    return result;
}

// 计算梯度图
cv::Mat computeGradient(const cv::Mat& src) {
    cv::Mat grad_x, grad_y, abs_grad_x, abs_grad_y, grad;
    cv::Sobel(src, grad_x, CV_16S, 1, 0);
    cv::Sobel(src, grad_y, CV_16S, 0, 1);
    cv::convertScaleAbs(grad_x, abs_grad_x);
    cv::convertScaleAbs(grad_y, abs_grad_y);
    cv::addWeighted(abs_grad_x, 0.5, abs_grad_y, 0.5, 0, grad);
    return grad;
}

cv::Mat modeFilter(const cv::Mat& src, int n) {
    cv::Mat dst = src.clone();
    int rows = src.rows;
    int cols = src.cols;

    auto getMode = [](const std::map<uchar, int>& histogram) {
        uchar mode = 0;
        int maxCount = 0;
        for (const auto& h : histogram) {
            if (h.second > maxCount) {
                maxCount = h.second;
                mode = h.first;
            }
        }
        return mode;
    };

    for (int y = 0; y < rows; y += n) {
        for (int x = 0; x < cols; x += n) {
            std::map<uchar, int> histogram;

            int regionHeight = std::min(n, rows - y);
            int regionWidth = std::min(n, cols - x);

            for (int j = 0; j < regionHeight; ++j) {
                for (int i = 0; i < regionWidth; ++i) {
                    int ny = y + j;
                    int nx = x + i;
                    uchar value = src.at<uchar>(ny, nx);
                    histogram[value]++;
                }
            }

            uchar mode = getMode(histogram);

            for (int j = 0; j < regionHeight; ++j) {
                for (int i = 0; i < regionWidth; ++i) {
                    int ny = y + j;
                    int nx = x + i;
                    dst.at<uchar>(ny, nx) = mode;
                }
            }
        }
    }
    return dst;
}

// 图像融合
cv::Mat fuseImages(const std::vector<cv::Mat>& images)
{
    std::vector<cv::Mat> gradients;
    for (const auto& img : images) {
        gradients.push_back(computeGradient(img));
        //gradients.push_back(stdConv(img, 60));
    }

    cv::Mat fusedImage = cv::Mat::zeros(images[0].size(), images[0].type());
    cv::Mat indexImage = cv::Mat::zeros(images[0].size(), images[0].type());
    for (int y = 0; y < images[0].rows; ++y) {
        for (int x = 0; x < images[0].cols; ++x) {
            int maxIdx = 0;
            double maxGrad = 0;
            for (int i = 0; i < gradients.size(); ++i) {
                if (gradients[i].at<uchar>(y, x) > maxGrad) {
                    maxGrad = gradients[i].at<uchar>(y, x);
                    maxIdx = i;
                }
            }
            //fusedImage.at<uchar>(y, x) = images[maxIdx].at<uchar>(y, x);
            indexImage.at<uchar>(y, x) = maxIdx;
        }
    }
    indexImage = modeFilter(indexImage, 30);
    for (int y = 0; y < images[0].rows; ++y) {
        for (int x = 0; x < images[0].cols; ++x)
        {
            fusedImage.at<uchar>(y, x) = images[indexImage.at<uchar>(y, x)].at<uchar>(y, x);
        }
    }
    return fusedImage;
}

// HDR Function
Mat createHDR(const std::vector<Mat>& images, std::vector<float>& times) 
{
    //// 对齐图像
    //Ptr<AlignMTB> alignMTB = createAlignMTB();
    //std::vector<Mat> alignedImages;
    //alignMTB->process(images, alignedImages);

    // 估计相机响应函数
    Mat responseDebevec;
    Ptr<CalibrateDebevec> calibrateDebevec = createCalibrateDebevec();
    calibrateDebevec->process(images, responseDebevec, times);

    // 合并图像
    Mat hdrDebevec;
    Ptr<MergeDebevec> mergeDebevec = createMergeDebevec();
    mergeDebevec->process(images, hdrDebevec, times, responseDebevec);
    //showImg(hdrDebevec);
    return hdrDebevec;

    //色调调整
    Mat ldrDrago;
    Ptr<TonemapDrago> tonemapDrago = createTonemapDrago(1.0, 0.7);
    tonemapDrago->process(hdrDebevec, ldrDrago);
    showImg(ldrDrago);
    ldrDrago = 3 * ldrDrago;

    //showImg(ldrDrago);
    return ldrDrago;
}

void dimple(cv::Mat& img, int kernel_size, int alpha)
{
    auto std = stdConv(img, kernel_size);
    img = std - img / 2;
    img = img * alpha;
}

/// <summary>
/// 提取复杂背景中的凹坑
/// </summary>
/// <param name="img">输入图像</param>
void dimple(cv::Mat& img)
{
    auto std = stdConv(img, 30);
    img = std - img / 2;
    img = img * 10;
}

void applyRangeToImage(cv::Mat& image) 
{
    // 计算图像的最大值和最小值
    double minVal, maxVal;
    cv::minMaxLoc(image, &minVal, &maxVal);

    // 计算极差
    double range = maxVal - minVal;

    // 将极差值直接赋给图像的每个像素
    image.setTo(cv::Scalar(range));
}

// 计算图像中某一区域的平均浓度（灰度值）， 未考虑超界问题
double calculateAverageIntensity(const cv::Mat& image, const cv::Rect& region) {
    return cv::mean(image(region))[0]; // 灰度图像只需第一个通道
}

// 检测段中的瑕疵等级
double calculateFlawLevel(const cv::Mat& image, const cv::Rect& segment, int n) {
    std::vector<double> intensities;

    int moveStep_x = segment.width / n;
    int moveStep_y = segment.height / n;
    // 滑动窗口法，每次移动段大小的1/n，计算n个段的平均浓度
    for (int i = 0; i < n; ++i) 
        for (int j = 0; j < n; ++j)
        {
            cv::Rect movedSegment(segment.x + i * moveStep_x, segment.y + j * moveStep_y, segment.width, segment.height);

            // 计算每个段的平均浓度
            double avgIntensity = calculateAverageIntensity(image, movedSegment);
            intensities.push_back(avgIntensity);
        }

    double maxIntensity = *std::max_element(intensities.begin(), intensities.end());
    double minIntensity = *std::min_element(intensities.begin(), intensities.end());
    return maxIntensity - minIntensity;

}

cv::Mat flawLevels(const cv::Mat& image, int segmentWidth, int segmentHeight)
{
    int n = 4;
    int step_x = segmentWidth / n;
    int step_y = segmentHeight / n;

    // 创建一个与原图像相同大小的空白图像用于存放瑕疵等级
    cv::Mat newImage = cv::Mat::zeros(image.size(), CV_8UC1);

    int max_y = image.rows - (n - 1) * step_y - segmentHeight;
    int max_x = image.cols - (n - 1) * step_x - segmentWidth;

    // 遍历图像中的每个段
    for (int y = 0; y < max_y; y += segmentHeight) {
        for (int x = 0; x < max_x; x += segmentWidth) {
            // 定义当前段
            cv::Rect segment(x, y, segmentWidth, segmentHeight);

            // 计算该段的瑕疵等级
            double flawLevel = calculateFlawLevel(image, segment, n);

            // 将该段中的所有像素设为瑕疵等级值
            newImage(segment).setTo(cv::Scalar(flawLevel));
        }
    }

    // 显示结果图像
    //showImg(newImage);
    return newImage;
}

uchar calculateMaxDifferenceFromCenter(const cv::Mat& image) {
    int width = image.cols;
    int height = image.rows;

    // 计算图像中心的坐标
    int centerX = width / 2;
    int centerY = height / 2;

    // 获取中心像素的灰度值
    uchar centerPixelValue = image.at<uchar>(centerY, centerX);

    uchar maxDifference = 0;

    // 遍历图像中的所有像素
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            uchar pixelValue = image.at<uchar>(y, x);
            int difference = std::abs(centerPixelValue - pixelValue);
            //int difference = (centerPixelValue - pixelValue);
            if (difference > maxDifference) {
                maxDifference = difference;
            }
        }
    }
    //if (maxDifference < 20) maxDifference = 0;
    return maxDifference;
}

uchar calculateMaxSignDifferenceFromCenter(const cv::Mat& image) {
    int width = image.cols;
    int height = image.rows;

    // 计算图像中心的坐标
    int centerX = width / 2;
    int centerY = height / 2;

    // 获取中心像素的灰度值
    uchar centerPixelValue = image.at<uchar>(centerY, centerX);

    uchar maxDifference = 0;

    // 遍历图像中的所有像素
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            uchar pixelValue = image.at<uchar>(y, x);
            int difference = (centerPixelValue - pixelValue);
            if (difference > maxDifference) {
                maxDifference = difference;
            }
        }
    }
    //if (maxDifference < 20) maxDifference = 0;
    return maxDifference;
}

Mat rangeConv(const cv::Mat& img_in, int box_width, int box_height)
{
    ConvImageProcessor cip(img_in, box_height, box_width);
    cip.registerProcessor(calculateMaxDifferenceFromCenter);
    cip.process();
    auto img = cip.getResultImage();
    return img;
}

Mat rangeConvSign(const cv::Mat& img_in, int box_width, int box_height)
{
    ConvImageProcessor cip(img_in, box_height, box_width);
    cip.registerProcessor(calculateMaxSignDifferenceFromCenter);
    cip.process();
    auto img = cip.getResultImage();
    return img;
}

cv::Vec3i getCircleByRange(const cv::Mat& img_)
{
    auto img = rangeConvSign(img_, 5, 5);
    //cv::threshold(img, img, 50, 50, cv::THRESH_TOZERO);
    cv::threshold(img, img, 50, 255, cv::THRESH_BINARY);
    std::vector<cv::Vec3f> circles;
    cv::HoughCircles(img, circles, cv::HOUGH_GRADIENT, 1, 10000, 200, 50, 100, 200);

    //drawCircle(img, circles);
    //drawCircle(img_, circles);
    if (circles.size() > 0) return circles[0];
    else
        throw std::logic_error("找不到圆");
}

// 绘制红色圆的函数
void drawCircle(const cv::Mat& grayImage, const std::vector<cv::Vec3f>& circles) {
    cv::Mat outputImage;
    // 将单通道的灰度图转换为三通道彩色图 (BGR)
    cv::cvtColor(grayImage, outputImage, cv::COLOR_GRAY2BGR);

    // 在彩色图像上绘制红色圆
    for (const auto& circle : circles) {
        int centerX = cvRound(circle[0]);
        int centerY = cvRound(circle[1]);
        int radius = cvRound(circle[2]);

        // 红色: (0, 0, 255) in BGR
        cv::circle(outputImage, cv::Point(centerX, centerY), radius, cv::Scalar(0, 0, 255), 2);
    }
    showImg(outputImage);
}

cv::Mat whiteGroundSearchBlack(cv::Mat& image_in, int bias, int threshold)
{
    Mat ret = Mat::zeros(image_in.size(), CV_8UC3);
    for (int x = 300; x < 600; x++)
    {
        for (int y = 0; y < image_in.rows; y++)
        {
            int y_new_up = (y - bias + image_in.rows) % image_in.rows;
            int y_new_down = (y + bias + image_in.rows) % image_in.rows;
            int dif_up = image_in.at<uchar>(y_new_up, x) - image_in.at<uchar>(y, x);
            int dif_down = image_in.at<uchar>(y_new_down, x) - image_in.at<uchar>(y, x);
            int dif = max(dif_up, dif_down);
            if (dif > threshold)
            {
                cv::Vec3b color(0, 0, 255);  // BGR: Blue, Green, Red
                ret.at<cv::Vec3b>(y, x) = color;
            }
        }
    }
    return ret;
}

PROCESSFUNCTION constructionGlobalFunction(PROCESSFUNCTION local_func, int width, int height)
{
    BoxImageProcessor bip(width, height);
    PROCESSFUNCTION func = std::bind(static_cast<cv::Mat(BoxImageProcessor::*)
        (const cv::Mat & image, const std::function<cv::Mat(cv::Mat&)>&processFunction)>
        (&BoxImageProcessor::processImage), &bip, placeholders::_1, local_func);
    return func;
}



cv::Mat BoxOperator::std(const cv::Mat& img)
{
    cv::Mat stdImage = cv::Mat::ones(img.size(), CV_8UC1);
    cv::Scalar mean, stddev;
    cv::meanStdDev(img, mean, stddev);
    stdImage = stddev[0] * stdImage;
    //cout << stddev[0] << endl;
    return stdImage;
}

cv::Mat BoxOperator::range(const cv::Mat& img)
{
    // 计算最大值和最小值
    double minVal, maxVal;
    cv::minMaxLoc(img, &minVal, &maxVal);

    // 计算极差
    double range = maxVal - minVal;
    cv::Mat retImage = cv::Mat::ones(img.size(), CV_8UC1);
    retImage = range * retImage;
    return retImage;
}

cv::Mat BoxOperator::rangeRatio(const cv::Mat& image, float ratio)
{
    // 检查图像是否为单通道灰度图
    if (image.channels() != 1) {
        throw std::invalid_argument("输入图像必须是单通道灰度图");
    }

    // 将图像中的像素值复制到一个向量
    std::vector<uchar> pixels;
    pixels.reserve(image.total());
    pixels.assign(image.begin<uchar>(), image.end<uchar>());

    // 对像素值进行排序
    std::sort(pixels.begin(), pixels.end());

    // 计算需要的像素数量
    int pixel_count = pixels.size();
    int num_pixels = static_cast<int>(pixel_count * ratio);

    // 计算最小像素值的平均值
    double min_pixel_sum = std::accumulate(pixels.begin(), pixels.begin() + num_pixels, 0.0);
    double min_pixel_avg = min_pixel_sum / num_pixels;

    // 计算最大像素值的平均值
    double max_pixel_sum = std::accumulate(pixels.end() - num_pixels, pixels.end(), 0.0);
    double max_pixel_avg = max_pixel_sum / num_pixels;

    // 计算差值
    double difference = max_pixel_avg - min_pixel_avg;

    // 创建输出图像，大小与输入图像相同，每个像素都设为该差值
    cv::Mat output_image(image.size(), CV_8UC1, cv::Scalar(static_cast<uchar>(difference)));

    return output_image;
}


// 计算图像的熵
cv::Mat BoxOperator::calculateEntropy(const cv::Mat& image) {
    // 创建256级灰度的直方图
    std::vector<int> histogram(256, 0);
    int totalPixels = image.rows * image.cols;

    // 计算直方图
    for (int i = 0; i < image.rows; ++i) {
        for (int j = 0; j < image.cols; ++j) {
            histogram[image.at<uchar>(i, j)]++;
        }
    }

    // 计算熵
    double entropy = 0.0;
    for (int i = 0; i < 256; ++i) {
        if (histogram[i] > 0) {
            double probability = (double)histogram[i] / totalPixels;
            entropy -= probability * std::log2(probability);
        }
    }

    cv::Mat output_image(image.size(), CV_8UC1, cv::Scalar(static_cast<uchar>(entropy * 256 / 8)));

    return output_image;
}

cv::Mat BoxOperator::calculateEntropyWithRange(const cv::Mat& image, int n) {
    // 创建256级灰度的直方图
    std::vector<int> histogram(256, 0);
    int totalPixels = image.rows * image.cols;

    // 计算直方图
    for (int i = 0; i < image.rows; ++i) {
        for (int j = 0; j < image.cols; ++j) {
            int pixelValue = image.at<uchar>(i, j);
            histogram[pixelValue]++;
        }
    }

    // 更新直方图以考虑范围分组
    std::vector<int> groupedHistogram(256 / n, 0); // 压缩后的直方图
    for (int i = 0; i < 256; i += n) {
        int sum = 0; // 计算范围内的总数
        for (int j = 0; j < n && (i + j) < 256; ++j) {
            sum += histogram[i + j];
        }
        groupedHistogram[i / n] = sum; // 更新压缩后的直方图
    }

    // 计算熵
    double entropy = 0.0;
    for (int i = 0; i < groupedHistogram.size(); ++i) {
        if (groupedHistogram[i] > 0) {
            double probability = static_cast<double>(groupedHistogram[i]) / totalPixels;
            entropy -= probability * std::log2(probability);
        }
    }

    cv::Mat output_image(image.size(), CV_8UC1, cv::Scalar(static_cast<uchar>(entropy * 256 / std::log2(256.0 / n))));
    return output_image;
}


// 将 cv::Rect 转换为字符串
std::string roiToString(const cv::Rect& roi) {
    std::ostringstream oss;
    oss << roi.x << "," << roi.y << "," << roi.width << "," << roi.height;
    return oss.str();
}

// 从字符串恢复 cv::Rect
cv::Rect stringToRoi(const std::string& roiString) {
    std::istringstream iss(roiString);
    int x, y, width, height;
    char delimiter;

    // 按逗号分隔并解析
    if ((iss >> x >> delimiter >> y >> delimiter >> width >> delimiter >> height) && delimiter == ',') {
        return cv::Rect(x, y, width, height);
    }
    else {
        throw std::invalid_argument("Invalid ROI string format");
    }
}