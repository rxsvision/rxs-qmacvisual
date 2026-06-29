#include "blobAnalyze.h"
#include"../czxTool2D.h"
#include"../czxTool_std.h"

using namespace cv;

BlobAnalyze::BlobAnalyze(cv::Mat img_in):origin_image(img_in)
{
	processed_image = img_in;
	feature_functions["meanInsideGray"] = (bind(&BlobAnalyze::meanInsideGray, this, placeholders::_1));
	feature_functions["boundingBoxAreaPercentage"] = (boundingBoxAreaPercentage);
	feature_functions["calculateAspectRatio"] = (calculateAspectRatio);
	feature_functions["calculateMaxSideOfBoundingBox"] = (calculateMaxSideOfBoundingBox);
	feature_functions["area"] = (area);
	feature_functions["calculateContourEntropy"] = (bind(&BlobAnalyze::calculateContourEntropy, this, placeholders::_1));
	feature_functions["calculateMostPixelRatio"] = (bind(&BlobAnalyze::calculateMostPixelRatio, this, placeholders::_1, 5));//这个执行较慢，单个contour需要4ms左右
}

BlobAnalyze::BlobAnalyze(cv::Mat img_in, cv::Mat bin_img):BlobAnalyze(img_in)
{
	binary_image = bin_img;
}

void BlobAnalyze::binaryThreshold(int min, int max)
{
	cv::threshold(processed_image, binary_image, min, max, cv::THRESH_BINARY);
}

void BlobAnalyze::dilate(int kernel_size)
{
	cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(kernel_size, kernel_size));
	cv::dilate(binary_image, binary_image, kernel);
}

void BlobAnalyze::process()
{
	CzxTimer _(__func__);
	// 查找轮廓
	vector<cv::Vec4i> hierarchy;
	vector<vector<cv::Point>> contours_candidate;
	findContours(binary_image, contours_candidate, hierarchy, RETR_LIST, CHAIN_APPROX_NONE);

	//// 遍历每个轮廓
	//for (const auto& contour : contours_candidate) {
	//	// 计算最小外接矩形
	//	cv::RotatedRect minRect = cv::minAreaRect(contour);

	//	// 获取包围盒的四个角点
	//	cv::Point2f corners[4];
	//	minRect.points(corners);

	//	// 绘制包围盒
	//	for (int i = 0; i < 4; i++) {
	//		cv::line(origin_image, corners[i], corners[(i + 1) % 4], cv::Scalar(0, 255, 0), 2); // 绿色线条
	//	}
	//}

	//showImg(origin_image);

	for (size_t i = 0; i < contours_candidate.size(); ++i)
	{
		bool is_valid = true;
		for (auto& func : check_contour_functions)
		{
			if (!func(contours_candidate[i]))
			{
				is_valid = false;
				break;
			}
		}
		if (is_valid)
		{
			contours_.push_back(contours_candidate[i]);
		}
	}

}

void BlobAnalyze::showBlob()
{
	cv::Mat color_image;
	cv::cvtColor(origin_image, color_image, cv::COLOR_GRAY2BGR);

	for (size_t i = 0; i < contours_.size(); ++i)
	{
		Scalar color(0, 255, 0); // 绿色
		drawContours(color_image, contours_, static_cast<int>(i), color, 2);

		if (text_func)
		{
			cv::Moments moments = cv::moments(contours_[i]);
			cv::Point2f centroid(moments.m10 / moments.m00, moments.m01 / moments.m00);
			cv::putText(color_image, text_func(contours_[i]), centroid, cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 255), 2);
		}
	}
	showImg(color_image);
}

void BlobAnalyze::showBlob(vector<BoundType> contours_)
{
	cv::Mat color_image;
	cv::cvtColor(origin_image, color_image, cv::COLOR_GRAY2BGR);

	for (size_t i = 0; i < contours_.size(); ++i)
	{
		Scalar color(0, 255, 0); // 绿色
		drawContours(color_image, contours_, static_cast<int>(i), color, 2);

		if (text_func)
		{
			cv::Moments moments = cv::moments(contours_[i]);
			cv::Point2f centroid(moments.m10 / moments.m00, moments.m01 / moments.m00);
			cv::putText(color_image, text_func(contours_[i]), centroid, cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 255), 2);
		}
	}
	showImg(color_image);
}

cv::Mat BlobAnalyze::getBlob()
{
	cv::Mat color_image;
	cv::cvtColor(origin_image, color_image, cv::COLOR_GRAY2BGR);

	for (size_t i = 0; i < contours_.size(); ++i)
	{
		Scalar color(0, 255, 0); // 绿色
		drawContours(color_image, contours_, static_cast<int>(i), color, 2);

		if (text_func)
		{
			cv::Moments moments = cv::moments(contours_[i]);
			cv::Point2f centroid(moments.m10 / moments.m00, moments.m01 / moments.m00);
			cv::putText(color_image, text_func(contours_[i]), centroid, cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 255), 2);
		}
	}
	return color_image;
}

void BlobAnalyze::calculateAllBlob()
{
	// 查找轮廓
	vector<cv::Vec4i> hierarchy;
	vector<BoundType> contours;
	findContours(binary_image, contours, hierarchy, RETR_LIST, CHAIN_APPROX_NONE);
	for (auto& contour : contours)
	{
		if (cv::contourArea(contour) > 35)
		{
			contours_candidate_.push_back(contour);
		}
	}
}

vector<BoundType> BlobAnalyze::filterBlob(vector<ContourCheckFunction> valid_funcs)
{
	vector<BoundType> filtered_blob;
	for (size_t i = 0; i < contours_candidate_.size(); ++i)
	{
		bool is_valid = true;
		for (auto& func : valid_funcs)
		{
			if (!func(contours_candidate_[i]))
			{
				is_valid = false;
				break;
			}
		}
		if (is_valid)
		{
			filtered_blob.push_back(contours_candidate_[i]);
		}
	}
	return filtered_blob;
}

double BlobAnalyze::meanInsideGray(BoundTypeCF contour)
{
	double totalGrayValue = 0.0;
	int count = 0;

	for (size_t i = 0; i < contour.size()/2; ++i) {
		// 获取当前点和下一个点
		cv::Point p1 = contour[i];
		cv::Point p2 = contour[(i + contour.size() / 2)];

		// 计算两点中点
		cv::Point mid = (p1 + p2) * 0.5;

		{
			uchar grayValue = origin_image.at<uchar>(mid);
			totalGrayValue += grayValue;
			count++;
		}
	}

	return (count > 0) ? (totalGrayValue / count) : 0.0;
}

double BlobAnalyze::boundingBoxAreaPercentage(const std::vector<cv::Point>& contour)
{
	//CzxTimer _(__func__);
	double contourArea = cv::contourArea(contour);

	// 计算最小包围盒
	cv::RotatedRect minRect = cv::minAreaRect(contour);
	float width = minRect.size.width;
	float height = minRect.size.height;
	float boundingBoxArea = width * height;

	// 计算占比
	double ratio = contourArea / boundingBoxArea;
	return ratio;
}

double BlobAnalyze::calculateMostPixelRatio(const std::vector<cv::Point>& contour, int range) 
{
	//CzxTimer _(__func__);
	// 创建掩膜
	cv::Mat mask = cv::Mat::zeros(origin_image.size(), CV_8UC1);
	cv::drawContours(mask, std::vector<std::vector<cv::Point>>{contour}, -1, cv::Scalar(255), cv::FILLED);

	// 统计掩膜区域的像素值
	int pixelCount[255] = { 0 };
	int totalPixels = 0;

	// 遍历掩膜区域，统计每个像素值
	for (int y = 0; y < mask.rows; y++) {
		for (int x = 0; x < mask.cols; x++) {
			if (mask.at<uchar>(y, x) == 255) { // 仅统计轮廓内部的像素
				int pixelValue = origin_image.at<uchar>(y, x); // 获取对应的图像像素值
				pixelCount[pixelValue]++;
				totalPixels++;
			}
		}
	}

	// 找出出现次数最多的像素值
	int max_count = 0;
	for (int i = 0; i <= 255 - range; i++)
	{
		int cur_count = 0;
		for (int off = 0; off < range; off++)
		{
			cur_count += pixelCount[i + off];
		}
		if (cur_count > max_count)
		{
			max_count = cur_count;
		}
	}

	// 计算占比
	double ratio = totalPixels > 0 ? static_cast<double>(max_count) / totalPixels : 0.0;

	return ratio; // 返回最多像素的占比
}

double BlobAnalyze::calculateAspectRatio(BoundTypeCF contour) {
	// 计算轮廓的最小旋转包围盒
	cv::RotatedRect minRect = cv::minAreaRect(contour);

	// 获取包围盒的尺寸（宽和高）
	float width = minRect.size.width;
	float height = minRect.size.height;

	// 计算长宽比（确保长宽比始终 >= 1）
	return (width > height) ? width / height : height / width;
}

double BlobAnalyze::calculateMaxSideOfBoundingBox(BoundTypeCF contour) {
	// 计算轮廓的最小旋转包围盒
	cv::RotatedRect minRect = cv::minAreaRect(contour);

	// 获取包围盒的尺寸（宽和高）
	float width = minRect.size.width;
	float height = minRect.size.height;

	// 返回最长的一边
	return std::max(width, height);
}

double BlobAnalyze::area(BoundTypeCF contour)
{
	return cv::contourArea(contour);
}

double calculateEntropy(const cv::Mat& image) {
	std::vector<int> histogram(256, 0);
	int totalPixels = 0;

	// 计算直方图
	for (int i = 0; i < image.rows; ++i) {
		for (int j = 0; j < image.cols; ++j) {
			int pixelValue = image.at<uchar>(i, j);
			histogram[pixelValue]++;
			totalPixels++; // 计数总像素
		}
	}

	// 计算熵
	double entropy = 0.0;
	for (int i = 0; i < histogram.size(); ++i) {
		if (histogram[i] > 0) {
			double probability = static_cast<double>(histogram[i]) / totalPixels;
			entropy -= probability * std::log2(probability);
		}
	}
	return entropy;
}

double calculateEntropyWithRange(const cv::Mat& image, int n) {
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

	return entropy;
}

double BlobAnalyze::calculateContourEntropy(BoundTypeCF contour) {
	cv::Rect boundingBox = cv::boundingRect(contour);

	// 检查包围盒是否在图像范围内
	if (boundingBox.x < 0) boundingBox.x = 0;
	if (boundingBox.y < 0) boundingBox.y = 0;
	if (boundingBox.x + boundingBox.width > origin_image.cols) {
		boundingBox.width = origin_image.cols - boundingBox.x;
	}
	if (boundingBox.y + boundingBox.height > origin_image.rows) {
		boundingBox.height = origin_image.rows - boundingBox.y;
	}

	// 提取包围盒区域
	cv::Mat boundingBoxRegion = origin_image(boundingBox);

	// 计算包围盒区域的熵
	double entropy = calculateEntropy(boundingBoxRegion);
	//double entropy = calculateEntropyWithRange(boundingBoxRegion, 10);

	return entropy;
}

bool BlobAnalyze::rot(const std::vector<cv::Point>& contour)
{
	int area = contourArea(contour);
	cv::Rect boundingBox = cv::boundingRect(contour);
	float ratio = boundingBox.width / boundingBox.height;
	if (ratio < 1) ratio = 1 / ratio;
	bool flag = area > 400 && area < 2000 && ratio < 3;
	if (!flag) return flag;
	
	double mean_gray = meanInsideGray(contour);
	flag = mean_gray > 200;

	return flag;
}

ContourCheckFunction BlobAnalyze::lessFunction(std::function<double(BoundTypeCF)> func, double threshold)
{
	return [func, threshold](BoundTypeCF contour)
	{
		return func(contour) < threshold;
	};
}

ContourCheckFunction BlobAnalyze::greaterFunction(std::function<double(BoundTypeCF)> func, double threshold)
{
	return [func, threshold](BoundTypeCF contour)
	{
		return func(contour) > threshold;
	};
}

ContourCheckFunction BlobAnalyze::betweenFunction(std::function<double(BoundTypeCF)> func, double min, double max)
{
	return [func, min, max](BoundTypeCF contour)
	{
		double val = func(contour);
		return val >= min && val <= max;
	};
}

ContourCheckFunction BlobAnalyze::multipleCondition(const std::vector<ContourCheckFunction>& functions)
{
	return [functions](BoundTypeCF contour)
	{
		for (const auto& func : functions)
		{
			if (!func(contour))
			{
				return false;
			}
		}
		return true;
	};
}

bool BlobAnalyze::isInside(BoundTypeCF contour, Rect roi)
{
	float ratio = calculateAreaRatioInRect(contour, roi);
	return ratio > 0.9;
}

double BlobAnalyze::calculateAreaRatioInRect(BoundTypeCF contour, const cv::Rect& rect)
{
	// 将普通矩形转换为四边形的顶点
	std::vector<cv::Point2f> rectPoints = {
		cv::Point2f(rect.x, rect.y),
		cv::Point2f(rect.x + rect.width, rect.y),
		cv::Point2f(rect.x + rect.width, rect.y + rect.height),
		cv::Point2f(rect.x, rect.y + rect.height)
	};

	// 计算交集
	std::vector<cv::Point2f> intersection;
	float intersectionArea = cv::intersectConvexConvex(
		contour,
		rectPoints,
		intersection
	);

	// 如果有交集，计算面积比率
	if (intersectionArea > 0) {
		double rotatedRectArea = cv::contourArea(contour);
		return intersectionArea / rotatedRectArea;
	}
	else {
		return 0.0; // 无交集时，面积比率为 0
	}
}

vector<BoundType>& BlobAnalyze::getContours()
{
	return contours_candidate_;
}
