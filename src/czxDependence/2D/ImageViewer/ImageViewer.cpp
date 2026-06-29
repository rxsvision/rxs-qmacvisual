#include "ImageViewer.h"
#include"../czxTool_std.h"

using namespace cv;

bool isCtrlPressed() {
    #ifdef _WIN32
    return (GetKeyState(VK_CONTROL) & 0x8000) != 0;
    #else
    return 1/0;
    #endif
}


ImageViewer::ImageViewer()
{
    scale = 100;
    drawing = false;
}

ImageViewer::ImageViewer(const string& imagePath) :ImageViewer()
{
    setImage(imread(imagePath));
    if (origin_image.empty())
    {
        throw runtime_error("图片打不开  " + imagePath);
    }
    //namedWindow("main", WINDOW_NORMAL);
    //setMouseCallback("main", onMouseStatic, this);
    //imshow("main", origin_image);
}

ImageViewer::ImageViewer(const Mat& image) :ImageViewer()
{
    setImage(image);
}

void ImageViewer::run()
{

    namedWindow("main", WINDOW_NORMAL);
    setMouseCallback("main", onMouseStatic, this);
    imshow("main", origin_image);

    while (true) 
    {
        //std::cout << cv::waitKey() << std::endl;

        int key = waitKeyEx(10);
        if (cv::getWindowProperty("main", cv::WND_PROP_VISIBLE) < 1) {
            std::cout << "主窗口关闭" << std::endl;
            break;
        }
        if (key == 27) { // ESC to exit
            cv::destroyWindow("main");
            break;
        }
        else if (key == 13)// 回车
        {
            onEnter();
        }
        else if (key == 26)
        {
            onCtrlZ();
        }
        else if (key == 2424832)
        {
            onLeftArrow();
        }
        else if (key == 2555904)
            onRightArrow();
    }
}

void ImageViewer::selectRectangle(int event, int x, int y, int, void*)
{
    if (event == EVENT_LBUTTONDOWN) {
        drawing = true;
        pt1 = Point(x, y);
    }
    else if (event == EVENT_MOUSEMOVE) 
    {
        if (drawing)
        {
            cv::Mat copy;// = color_img.clone();
            cv::cvtColor(color_img, copy, cv::COLOR_BGR2BGRA);

            pt2 = Point(x, y);
            rectangle(copy, pt1, pt2, Scalar(0, 255, 0), 8);
            imshow("main", copy);
        }
    }
    else if (event == EVENT_LBUTTONUP && drawing)
    {
        drawing = false;
        pt2 = Point(x, y);
        if (cv::norm(pt1 - pt2) > 35)
        {
            // 确保 pt1 和 pt2 分别为左上角和右下角
            select_roi = Rect(min(pt1.x, pt2.x), min(pt1.y, pt2.y), abs(pt1.x - pt2.x), abs(pt1.y - pt2.y));
            selectRectangleDown();
        }
    }
}

int ImageViewer::getMouseWheelDelta(int flags)
{
    // Extract the mouse wheel delta from flags
    //cout << "flag" << flags<<endl;
    return flags / 7864320;
    //return (flags >> 16) & 0xffff;
}

void ImageViewer::setImage(cv::Mat img)
{
    origin_image = img;
    cv::cvtColor(origin_image, color_img, cv::COLOR_GRAY2BGR);
    img_color_clone = color_img.clone();
}

void ImageViewer::setColorImage(cv::Mat img)
{
    color_img = img;
    img_color_clone = color_img.clone();
}

cv::Mat ImageViewer::getImage()
{
    return origin_image;
}

void ImageViewer::updateImage()
{
    img_color_clone = color_img.clone();
    imshow("main", color_img);
}

void ImageViewer::updateImage(cv::Mat img)
{
    color_img = img;
    img_color_clone = color_img.clone();
    imshow("main", color_img);
}

void ImageViewer::drawPoints(vector<cv::Point2f> points)
{
    for (const auto& point : points) {
        cv::circle(color_img, point, 5, cv::Scalar(0, 255, 0), -1); // 绘制绿色圆点，半径为5
    }
    updateImage();
}

cv::Mat ImageViewer::drawPoints(cv::Mat img, vector<cv::Point2f> points)
{
    //img.convertTo(img, CV_8UC3);
    cv::cvtColor(img, img, COLOR_GRAY2BGR);
    for (const auto& point : points) {
        cv::circle(img, point, 5, cv::Scalar(0, 255, 0), -1); // 绘制绿色圆点，半径为5
    }
    return img;
}

void ImageViewer::selectRectangleDown()
{
}

void ImageViewer::onMouseStatic(int event, int x, int y, int flags, void* userdata)
{
    ImageViewer* self = static_cast<ImageViewer*>(userdata);
    //self->MouseWheel(event, x, y, flags);
    try {
        self->selectRectangle(event, x, y, flags, userdata);
    }
    catch (const exception& ex) {
        cerr << ex.what() << endl;
    }
}

void ImageViewer::MouseWheel(int event, int x, int y, int flags)
{
    if (event == EVENT_MOUSEWHEEL) {
        int delta = getMouseWheelDelta(flags);
        cout << delta << endl;
        handleZoom(delta);
    }
}

void ImageViewer::handleZoom(int delta)
{
    // `delta` is positive for zooming in and negative for zooming out
    int zoomFactor = delta > 0 ? 10 : -10;
    scale += zoomFactor;
    if (scale < 10) scale = 10;  // Set minimum scale
    if (scale > 300) scale = 300; // Set maximum scale

    // Resize image according to the new scale
    resize(origin_image, img_resized, Size(), scale / 100.0, scale / 100.0);
    imshow("image", img_resized);
}

int getMouseWheelDelta(int flags) {
    // Extract the mouse wheel delta from flags
    return (flags >> 16) & 0xffff;
}

std::string ImageViewer::pointsToString() {
    std::ostringstream oss;
    oss << pt1.x << "," << pt1.y << ";" << pt2.x << "," << pt2.y;
    return oss.str();
}

// 从字符串中恢复两个 cv::Point
void ImageViewer::stringToPoints(const std::string& str) {
    std::istringstream iss(str);
    char delimiter;
    iss >> pt1.x >> delimiter >> pt1.y >> delimiter >> pt2.x >> delimiter >> pt2.y;
}