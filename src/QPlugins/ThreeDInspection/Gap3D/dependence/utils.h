#pragma once
#include<opencv2/opencv.hpp>
#include<qimage.h>
#include<czxTool.h>
#include<qmessagebox.h>

QImage Mat2QImage(const cv::Mat& mat);

CP img2pcl(const cv::Mat& img, float x_scale, float y_scale, float z_scale, float z_invalid);