#include "ParameterTuner.h"

using namespace cv;
BloBParameterTuner::BloBParameterTuner(Mat img, Mat binary_img):ImageViewer(img), blob_and_methods(img, binary_img)
{

}

void BloBParameterTuner::selectRectangleDown()
{
	cout << "select" << endl;
	auto& blob = blob_and_methods.blob;

	//blob_and_methods.blob.showBlob(blob_and_methods.valid_contour);
	rois.push_back(select_roi);
}

void BloBParameterTuner::onEnter()
{
	cout << "enter presss" << endl;
	blob_and_methods.updateValidContour(rois);
	blob_and_methods.evaluateMethods();
	auto cfs = blob_and_methods.methods2CheckFunction();
	blob_and_methods.blob.check_contour_functions = cfs;
	blob_and_methods.blob.process();
	blob_and_methods.blob.showBlob();
	//blob_and_methods.blob.showBlob(blob_and_methods.valid_contour);

	//for (auto& method : blob_and_methods.methods)
	//{
	//	cout << method.toString() << endl;
	//}
}

BlobAndMethods::BlobAndMethods(cv::Mat img_in, cv::Mat bin_img):blob(img_in, bin_img)
{
	blob.calculateAllBlob();
}

void BlobAndMethods::updateValidContour(vector<cv::Rect> rois)
{
	for (auto& contour : blob.getContours())
	{
		for (auto& roi : rois)
		{
			if (blob.isInside(contour, roi))
			{
				valid_contour.push_back(contour);
				break;
			}
		}
	}
}

void BlobAndMethods::evaluateMethods()
{
	std::vector<std::pair<string, FeatureFunction>> items(blob.feature_functions.begin(), blob.feature_functions.end());
	#pragma omp parallel for
	for (int i = 0; i < items.size(); ++i) 
	{
		auto func = items[i];
		FunctionAndRange fandr;
		//CzxTimer _(func.first);
		fandr.function = func.first;
		{
			double min = std::numeric_limits<double>::max();
			double max_ = std::numeric_limits<double>::lowest();
			for (auto& contour : blob.getContours())
			{
				double val = func.second(contour);
				min = std::min(val, min);
				max_ = std::max(val, max_);
			}
			fandr.global_max = max_;
			fandr.global_min = min;
		}

		{
			double min = std::numeric_limits<double>::max();
			double max = std::numeric_limits<double>::lowest();
			for (auto& contour : valid_contour)
			{
				double val = func.second(contour);
				min = std::min(val, min);
				max = std::max(val, max);
			}
			fandr.valid_max = max;
			fandr.valid_min = min;
		}
		#pragma omp critical
		{
			methods.push_back(fandr);
			cout << fandr.toString() << endl;
		}
	}
}

vector<ContourCheckFunction> BlobAndMethods::methods2CheckFunction()
{
	vector<ContourCheckFunction> ret;
	for (auto& method : methods)
	{
		float range = method.valid_max - method.valid_min;
		float global_range = method.global_max - method.global_min;
		if (range > 0.8 * global_range) continue;//无效条件
		if (range == 0) range = 0.1;
		double max = method.valid_max + range * 0.2;
		double min = method.valid_min - range * 0.2;
		ContourCheckFunction cf = blob.betweenFunction(blob.feature_functions[method.function], min, max);
		ret.push_back(cf);
	}
	return ret;
}
std::string FunctionAndRange::toString() const
{
	std::stringstream ss;
	ss << function << "," << valid_min << "," << valid_max << "," << global_min << "," << global_max;
	return ss.str();
}

void FunctionAndRange::fromString(const std::string& str)
{
	std::stringstream ss(str);
	std::string temp;

	std::getline(ss, function, ','); // 读取字符串
	std::getline(ss, temp, ','); // 读取第一个 float
	valid_min = std::stof(temp);     // 转换为 float
	std::getline(ss, temp, ','); // 读取第二个 float
	valid_max = std::stof(temp);     // 转换为 float
	std::getline(ss, temp, ','); // 读取第三个 float
	global_min = std::stof(temp);     // 转换为 float
	std::getline(ss, temp, ','); // 读取第四个 float
	global_max = std::stof(temp);     // 转换为 float
}

RLModel::RLModel()
{
	process_function_map["std"] = constructionGlobalFunction(BoxOperator::std);
	process_function_map["range"] = constructionGlobalFunction(BoxOperator::range);
	process_function_map["rangeRatio"] = constructionGlobalFunction(bind(BoxOperator::rangeRatio, placeholders::_1, 0.1));
	process_function_map["calculateEntropy"] = constructionGlobalFunction(BoxOperator::calculateEntropy);
	process_function_map["calculateEntropyWithRange"] = constructionGlobalFunction(bind(BoxOperator::calculateEntropyWithRange, placeholders::_1, 5));
}

RLModel::RLModel(string root):RLModel()
{
	vector<string> files = czx_file::pathGather(root);
	for (auto& path : files)
	{
		auto img = cv::imread(path, cv::IMREAD_GRAYSCALE);
		pictures.push_back(img);
		rois_for_pictures.push_back({});
	}
	if (pictures.size() == 0)
		throw logic_error("未读取到图片");
	cur_index = 0;
	setImage(pictures[cur_index]);
}

void RLModel::selectRectangleDown()
{
	cout << __func__ << endl;
	rois_for_pictures[cur_index].push_back(select_roi);
	rectangle(color_img, select_roi, Scalar(0, 255, 0), 8);
	updateImage();
}

void RLModel::onCtrlZ()
{
	cout << __func__ << endl;
	if(!rois_for_pictures[cur_index].empty())
		rois_for_pictures[cur_index].pop_back();

	auto img = getImage();
	cv::cvtColor(img, color_img, cv::COLOR_GRAY2BGR);

	for (auto& roi : rois_for_pictures[cur_index])
	{
		rectangle(color_img, roi, Scalar(0, 255, 0), 8);
	}
	updateImage();
}

void RLModel::onLeftArrow()
{
	cout << __func__ << endl;
	if (cur_index == 0) return;
	cur_index--;
	setImage(pictures[cur_index]);

	for (auto& roi : rois_for_pictures[cur_index])
	{
		rectangle(color_img, roi, Scalar(0, 255, 0), 8);
	}
	updateImage();
}

void RLModel::onRightArrow()
{
	cout << __func__ << endl;
	if (cur_index == pictures.size()-1) return;
	cur_index++;
	setImage(pictures[cur_index]);

	for (auto& roi : rois_for_pictures[cur_index])
	{
		rectangle(color_img, roi, Scalar(0, 255, 0), 8);
	}
	updateImage();
}

void RLModel::onEnter()
{
	auto range = initBinaryThreshold(ROIHist());
	cout << range.first << "," << range.second << endl;
	for (auto& img : pictures)
	{
		cv::Mat binaryImage;
		cv::inRange(img, range.first, range.second, binaryImage);
		color_img = binaryImage;
		updateImage();
	}
}

cv::Mat RLModel::singleROIHist(cv::Mat img, vector<cv::Rect> rois)
{
	// 定义直方图参数
	int histSize = 256;  // 256 个灰度级
	float range[] = { 0, 256 };
	const float* histRange = { range };

	// 创建用于存储累积直方图的变量
	cv::Mat cumulativeHist = cv::Mat::zeros(histSize, 1, CV_32F);

	// 对每个矩形区域进行处理
	for (const auto& rect : rois) {
		// 裁剪矩形区域
		cv::Mat roi = img(rect);

		// 计算该区域的直方图
		cv::Mat hist;
		cv::calcHist(&roi, 1, 0, cv::Mat(), hist, 1, &histSize, &histRange);

		// 累加直方图
		cumulativeHist += hist;
	}
	return cumulativeHist;
}

cv::Mat RLModel::multipleROIHist(vector<cv::Mat> imgs, vector<vector<cv::Rect>> mul_rois)
{
	if (imgs.size() != mul_rois.size())
		throw logic_error("图片数目和ROI数目不匹配");
	
	int histSize = 256;  // 256 个灰度级
	cv::Mat cumulativeHist = cv::Mat::zeros(histSize, 1, CV_32F);
	for (int i=0;i<imgs.size();i++)
	{
		cumulativeHist += singleROIHist(imgs[i], mul_rois[i]);
	}
	return cumulativeHist;
}

cv::Mat RLModel::ROIHist()
{
	return multipleROIHist(pictures, rois_for_pictures);
}

pair<int, int> RLModel::initBinaryThreshold(cv::Mat hist)
{
	int range = 20;
	pair<int, int> max_range = { 0, range };
	float max_mean = 0;
	float sum = 0;
	for (int i = 0; i < range; i++)
	{
		sum += hist.at<float>(i);
	}
	max_mean = sum / range;

	for (int i = range; i < 256; i++)
	{
		sum += hist.at<float>(i);
		sum -= hist.at<float>(i - range);
		float mean = sum / range;
		if (mean > max_mean)
		{
			max_mean = mean;
			max_range = { i - range,i };
			cout << i << ":" << max_mean << endl;
		}
	}
	return max_range;
}
