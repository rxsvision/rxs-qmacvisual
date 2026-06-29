#pragma once
#include"opencv2/opencv.hpp"

cv::Mat brightnessCorrection(const cv::Mat& inputImage, int kernelSize = 15);

cv::Mat stdConv(const cv::Mat& grayImage, int kernelSize, int alpha = 1);

cv::Mat stdConvDif(const cv::Mat& grayImage, int kernelSize, int kernelSize_min);

cv::Mat meanConvDif(const cv::Mat& grayImage, int kernelSize, int kernelSize_small);

cv::Mat binaryStd(const cv::Mat& grayImage, int kernelSize, double varianceThreshold);

cv::Mat minusMean(const cv::Mat& inputImage);

cv::Mat difImage(const cv::Mat& image, int n);

std::vector<cv::Mat> extractBinaryImages(const cv::Mat& grayImage, int step=5);

cv::Mat maxFilter(const cv::Mat& src, int ksize);

cv::Mat noiseConv(const cv::Mat& grayImage, int kernelSize);

cv::Mat countConv(const cv::Mat& grayImage, int kernelSize);

cv::Mat applyLowPassFilter(cv::Mat& src, int radius);

cv::Mat applyHighPassFilter(cv::Mat& src, int radius);

void showImg(const cv::Mat& img, std::string name="main");

cv::Mat segmentConnectedComponents(cv::Mat& image, int threshold);

cv::Mat segmentConnectedComponents(cv::Mat& image, int threshold, int min_size, int max_size);

cv::Mat computeGradient(const cv::Mat& src);

cv::Mat fuseImages(const std::vector<cv::Mat>& images);

/// <summary>
/// 合并多幅不同曝光的图像
/// </summary>
/// <param name="images">图像</param>
/// <param name="times">曝光时间，单位s</param>
/// <returns></returns>
cv::Mat createHDR(const std::vector<cv::Mat>& images, std::vector<float>& times);

/// <summary>
/// 找出图像中异于背景的区域
/// </summary>
/// <param name="img">输入图像</param>
/// <param name="kernel_size">缺陷越大个这个值就需要越大</param>
/// <param name="alpha">输出图像灰度值线性放大，图像太暗可以设置成2亮度值放大两倍</param>
void dimple(cv::Mat& img, int kernel_size, int alpha);

void dimple(cv::Mat& img);

/// <summary>
/// 计算整个图像的瑕疵等级
/// </summary>
/// <param name="image">输入图</param>
/// <param name="segmentWidth">盒子宽</param>
/// <param name="segmentHeight">盒子高</param>
/// <returns>输出图</returns>
cv::Mat flawLevels(const cv::Mat& image, int segmentWidth, int segmentHeight);

cv::Mat rangeConv(const cv::Mat& img_in, int box_width=3, int box_height=3);

cv::Mat rangeConvSign(const cv::Mat& img_in, int box_width, int box_height);

cv::Vec3i getCircleByRange(const cv::Mat& img_);

void drawCircle(const cv::Mat& image, const std::vector<cv::Vec3f>& circles);

/// <summary>
/// 同列中，纯白背景下找出黑色的块
/// </summary>
/// <param name="image_in"></param>
/// <param name="bias">对比的偏移</param>
/// <param name="threshold">黑白差异的阈值</param>
/// <returns></returns>
cv::Mat whiteGroundSearchBlack(cv::Mat& image_in, int bias=40,int threshold=100);


using PROCESSFUNCTION = std::function<cv::Mat(const cv::Mat&)>;

PROCESSFUNCTION constructionGlobalFunction(PROCESSFUNCTION local_func, int width=20, int height=20);

std::string roiToString(const cv::Rect& roi);

cv::Rect stringToRoi(const std::string& roiString);

namespace BoxOperator
{
	cv::Mat std(const cv::Mat& img);
	cv::Mat range(const cv::Mat& img);
	//前后一定比率像素值的极差
	cv::Mat rangeRatio(const cv::Mat& image, float ratio);
	cv::Mat calculateEntropy(const cv::Mat& image);
	cv::Mat calculateEntropyWithRange(const cv::Mat& image, int n);
}


