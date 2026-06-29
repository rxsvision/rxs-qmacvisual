#pragma once
#include<string>
#include<vector>
#include<opencv2/opencv.hpp>
#include"../czxTool.h"

using namespace std;
namespace czx
{
	class PhotometricSteroCZX
	{
	public:
		PhotometricSteroCZX(const vector<string>& img_paths, const vector<cv::Vec3f>& lights);
		PhotometricSteroCZX(const vector<string>& img_paths, const vector<float>& slants, const vector<float>& tilts);
		PhotometricSteroCZX(const vector<cv::Mat>& imgs, const vector<float>& slants, const vector<float>& tilts);

		cv::Mat globalHeights();
		VectorXf globalHeightsMLS();

		CP getCloud(float scale = 1.0f);
		cv::Mat getNormal();

		cv::Mat getNormalGray();
		cv::Mat getMean();
		cv::Mat getAlbedo();

		static void MyShow(string name, cv::Mat image);

		//cv::Mat getGlobalHeights();

	private:
		cv::Mat Normals;
		cv::Mat Pgrads;
		cv::Mat Qgrads;
		cv::Mat LightsInv;
		cv::Mat Albedo;
		vector<string> img_paths;
		vector<cv::Mat> images;
		int num_imamges;
	};

}