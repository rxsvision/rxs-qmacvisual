#pragma once
#include"opencv2/opencv.hpp"
#include <map>

using BoundType = std::vector<cv::Point>;
using BoundTypeCF = const BoundType&;
typedef std::function<bool(BoundTypeCF)> ContourCheckFunction;
typedef std::function<std::string(BoundTypeCF)> ContourTextFunction;
typedef std::function<double(BoundTypeCF)> FeatureFunction;

using namespace std;
class BlobAnalyze
{
public:
	BlobAnalyze(cv::Mat img_in);
	BlobAnalyze(cv::Mat img_in, cv::Mat bin_img);
	void binaryThreshold(int min, int max);
	void dilate(int kernel_size);
	//根据合法函数群，增量选择合适的blob
	void process();
	void showBlob();
	void showBlob(vector<BoundType>);
	cv::Mat getBlob();
	//利用二值图计算出所有的blob
	void calculateAllBlob();
	//利用一系列函数群寻找合法的blob
	vector<BoundType> filterBlob(vector<ContourCheckFunction> valid_funcs);


	map<string, FeatureFunction> feature_functions;
	//内部平均灰度 中点简易版
	double meanInsideGray(BoundTypeCF contour);
	//包围盒占比
	static double boundingBoxAreaPercentage(const std::vector<cv::Point>& contour);
	//最多像素占比
	double calculateMostPixelRatio(const std::vector<cv::Point>& contour, int range);
	//计算长宽比
	static double calculateAspectRatio(BoundTypeCF contour);
	//包围盒的最长边
	static double calculateMaxSideOfBoundingBox(BoundTypeCF contour);
	static double area(BoundTypeCF contour);
	//计算熵
	double calculateContourEntropy(BoundTypeCF contour);

	bool rot(const std::vector<cv::Point>& contour);


	//将计算函数转换成的输出重新打包成输出成字符串
	template<typename ReturnType>
	static ContourTextFunction adaptToStringFunction(std::function<ReturnType(BoundTypeCF contour)> func) {
		return [func](const std::vector<cv::Point>& contour) {
			std::ostringstream oss;
			oss << func(contour);
			return oss.str();
		};
	}
	static ContourCheckFunction lessFunction(std::function<double(BoundTypeCF)> func, double threshold);
	static ContourCheckFunction greaterFunction(std::function<double(BoundTypeCF)> func, double threshold);
	static ContourCheckFunction betweenFunction(std::function<double(BoundTypeCF)> func, double min, double max);

	ContourCheckFunction multipleCondition(const std::vector<ContourCheckFunction>& functions);


	//判断blob是否在roi之内
	bool isInside(BoundTypeCF contour, cv::Rect roi);
	double calculateAreaRatioInRect(BoundTypeCF contour, const cv::Rect& rect);


	vector<ContourCheckFunction> check_contour_functions;
	ContourTextFunction text_func=nullptr;

	cv::Mat binary_image;
	cv::Mat processed_image;


	vector<BoundType>& getContours();
private:
	const cv::Mat origin_image;
	vector<vector<cv::Point>> contours_;
	vector<BoundType> contours_candidate_;
};
