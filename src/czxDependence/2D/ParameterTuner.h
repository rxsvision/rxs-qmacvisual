#pragma once
#include "ImageViewer.h"
#include"blobAnalyze.h"
#include"../czxTool_std.h"
#include"../czxTool2D.h"
#include<list>


class FunctionAndRange
{
public:
    string function;
    float valid_min;
    float valid_max;

    float global_min;
    float global_max;

    std::string toString() const;

    void fromString(const std::string& str);
};


class BlobAndMethods
{
public:
    BlobAndMethods(cv::Mat img_in, cv::Mat bin_img);

    void updateValidContour(vector<cv::Rect> rois);
    void evaluateMethods();
    vector<ContourCheckFunction> methods2CheckFunction();

    BlobAnalyze blob;
    vector<BoundType> valid_contour;
    list<FunctionAndRange> methods;
};


class BloBParameterTuner:
    public ImageViewer
{
public:
    BloBParameterTuner(cv::Mat img, cv::Mat binary_img);

private:
    void selectRectangleDown() override;
    void onEnter() override;


    vector<cv::Rect> rois;

protected:
    BlobAndMethods blob_and_methods;
};


class RLModel:
    public ImageViewer
{
public:
    RLModel();
    RLModel(string root);

private:
    map<string, PROCESSFUNCTION> process_function_map;

    void selectRectangleDown() override;
    void onCtrlZ() override;
    void onLeftArrow() override;
    void onRightArrow() override;
    void onEnter() override;

    cv::Mat singleROIHist(cv::Mat img, vector<cv::Rect> rois);
    cv::Mat multipleROIHist(vector<cv::Mat> imgs, vector<vector<cv::Rect>> mul_rois);
    cv::Mat ROIHist();

    pair<int, int> initBinaryThreshold(cv::Mat hist);

private:
    vector<cv::Mat> pictures;
    vector<vector<cv::Rect>> rois_for_pictures;
    int cur_index;
};


