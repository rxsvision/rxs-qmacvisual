#include"utils.h"

QImage Mat2QImage(const cv::Mat& mat)
{
	if (mat.empty())
	{
		return QImage();
	}
	if (mat.type() == CV_8UC1)
	{
		QImage image(mat.cols, mat.rows, QImage::Format_Indexed8);
		image.setColorCount(256);
		for (int i = 0; i < 256; i++)
		{
			image.setColor(i, qRgb(i, i, i));
		}
		uchar* pSrc = mat.data;
		for (int row = 0; row < mat.rows; row++)
		{
			uchar* pDest = image.scanLine(row);
			memcpy(pDest, pSrc, mat.cols);
			pSrc += mat.step;
		}
		return image;
	}
	else if (mat.type() == CV_8UC3)
	{
		const uchar* pSrc = (const uchar*)mat.data;
		QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
		return image.rgbSwapped();
	}
	else if (mat.type() == CV_8UC4)
	{
		const uchar* pSrc = (const uchar*)mat.data;
		QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_ARGB32);
		return image.copy();
	}
	else if (mat.type() == CV_32FC1)
	{
		// 归一化到 [0, 255]
		cv::Mat normalizedMat;
		cv::normalize(mat, normalizedMat, 0, 255, cv::NORM_MINMAX);

		// 转换为 CV_8UC1
		cv::Mat ucharMat;
		normalizedMat.convertTo(ucharMat, CV_8UC1);

		// 创建 QImage
		return QImage(ucharMat.data, ucharMat.cols, ucharMat.rows, ucharMat.step, QImage::Format_Grayscale8).copy();
	}
	else if (mat.type() == CV_16UC1) {
		// 规范化为 8 位图像
		cv::Mat normalized;
		double minVal, maxVal;
		cv::minMaxLoc(mat, &minVal, &maxVal); // 获取最小和最大值
		mat.convertTo(normalized, CV_8U, 255.0 / maxVal); // 映射到 0-255

		// 转换为 QImage
		return QImage(normalized.data, normalized.cols, normalized.rows, normalized.step, QImage::Format_Grayscale8).copy();
	}
	else
	{
		//cout<< mat.type();
		QMessageBox::information(nullptr, "type", QString::number(mat.type())); // 一行代码显示消息框
		return QImage();
	}
}

CP img2pcl(const cv::Mat& img, float x_scale, float y_scale, float z_scale, float z_invalid)
{
	CP ret(new CloudT);
	//cv::Mat new_img;
	//img.convertTo(new_img, CV_32FC1);
	//QMessageBox::information(nullptr, "type", QString::number(img.type())); // 一行代码显示消息框
	//QMessageBox::information(nullptr, "type", QString::number(new_img.type())); // 一行代码显示消息框

	for (int r = 0; r < img.rows; r++)
	{
		for (int c = 0; c < img.cols; c++)
		{
			ushort z = img.at<ushort>(r, c);
			//float z_f = z / 255.0;
			if (z <= z_invalid) continue;
			PointT p(c * x_scale, r * y_scale, z * z_scale);
			ret->push_back(p);
		}
	}
	return ret;
}
