#pragma once
#include <opencv2/opencv.hpp>
#include <iostream>
#include <windows.h> 

using namespace std;

class ImageViewer
{
public:
    ImageViewer();
	ImageViewer(const string& imagePath);
	ImageViewer(const cv::Mat& image);
	void run();

    std::string pointsToString();
    void stringToPoints(const std::string& str);

    cv::Rect getRoi() {
        return select_roi;}

    cv::Point pt1, pt2;//左上角右下角的两个点
protected:
    int scale;
    bool drawing;//是否正在框选
    cv::Rect select_roi;//框选的roi

    static void onMouseStatic(int event, int x, int y, int flags, void* userdata);
    void MouseWheel(int event, int x, int y, int flags);
    void handleZoom(int delta);
    //两点移动在框选中
    virtual void selectRectangle(int event, int x, int y, int, void*);
    int getMouseWheelDelta(int flags);

    void setImage(cv::Mat img);
    void setColorImage(cv::Mat img);
    cv::Mat getImage();

    void updateImage();
    void updateImage(cv::Mat img);

    void drawPoints(vector<cv::Point2f> points);
    cv::Mat drawPoints(cv::Mat img, vector<cv::Point2f> points);

    /// <summary>
    /// 重载函数，指定框选完成后的操作
    /// </summary>
    virtual void selectRectangleDown();
    virtual void onEnter() {};
    virtual void onCtrlZ() {};
    virtual void onLeftArrow() {};
    virtual void onRightArrow() {};
    cv::Mat color_img;

private:
    cv::Mat origin_image, img_resized, img_color_clone;
};

