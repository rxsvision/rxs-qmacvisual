#pragma once
#include <random> 
#include <algorithm>
#include <vector>
#include <iostream>
#include<opencv2/opencv.hpp>
#include <utility>

using namespace std;
template<typename T>
class Ransac2D
{
public:
    Ransac2D(T all_data_);

    float sample_ratio = 0.5;
    int sample_num = -1;
	T sample()
	{
		int valid_range = valid_num.size() * 0.8;
        if (valid_range < 20) valid_range = valid_num.size();

        // 创建一个从0到n的整数序列
        std::vector<int> numbers;
        for (int i = 0; i < valid_range; ++i) {
            numbers.push_back(i);
        }

        // 随机生成随机数生成器
        std::random_device rd;
        std::mt19937 gen(rd());

        // 使用 random_shuffle 打乱 numbers
        std::shuffle(numbers.begin(), numbers.end(), gen);        
        int sample_count = valid_range * sample_ratio;
        if (sample_count < 5) sample_count = 5;
        if (sample_count > numbers.size()) sample_count = numbers.size();
        if (sample_num > 0) sample_count = sample_num;

        select_samples = std::vector<int>(numbers.begin(), numbers.begin() + sample_count);

        T ret;
        for (int i = 0; i < select_samples.size(); i++)
            ret.push_back(all_data[select_samples[i]]);
        sample_points = ret;
        return ret;
	}

    void run();
    T getSample() { return sample_points; };

    virtual void fit()=NULL;
    virtual bool evaluate()=NULL;//需要评估拟合结果，并更新valid_num，按照贴合计数重新排序
    virtual void useBest() = NULL;

    float max_score = std::numeric_limits<float>::min();
protected:
	vector<pair<int, int>> valid_num;//第一个数是索引，第二个数是贴合计数
    T all_data;
    T sample_points;
    std::vector<int> select_samples;
};

template<typename T>
inline Ransac2D<T>::Ransac2D(T all_data_)
{
    for (int i = 0; i < all_data_.size(); i++)
    {
        valid_num.push_back(pair<int,int>(i, 0));
    }
    all_data = all_data_;
}

template<typename T>
void Ransac2D<T>::run()
{
    for (int _ = 0; _ < 1000; _++)
    {
        fit();
        if (evaluate()) return;
    }
    useBest();
}

#pragma region LINE
//ax+by+c=0
class RansacLine :public Ransac2D<vector<cv::Point2f>>
{
public:
    RansacLine(vector<cv::Point2f> ad) :Ransac2D(ad) {};

    virtual void fit();
    float pointToLineDistance(const cv::Point2f& point);
    virtual bool evaluate();
    virtual void useBest();

    float a, b, c;
    float best_a, best_b, best_c;

    float threshold = 2;
};
#pragma endregion

#pragma region CIRCLE
//(x-a)2+(y-b)2=r2
class RansacCircle :public Ransac2D<vector<cv::Point2f>>
{
public:
    RansacCircle(vector<cv::Point2f> ad) :Ransac2D(ad) {};

    virtual void fit();
    bool fit_3Point(const cv::Point2f& p1, const cv::Point2f& p2, const cv::Point2f& p3);
    float distanceToCircle(const cv::Point2f& point);
    virtual bool evaluate();
    virtual void useBest();
    //float pointToLineDistance(const cv::Point2f& point);


    float a, b, r;

    float best_a, best_b, best_r;

    float threshold = 0.5;

    float ref_radius = -1;

};
#pragma endregion

#pragma region SLOT
class RansacSlot
{
public:
    RansacSlot(vector<cv::Point2f> line_up, vector<cv::Point2f> line_down, vector<cv::Point2f> circle_left, vector<cv::Point2f>circle_right);

    void fit();
    //bool evaluate();
    //void run();

    RansacLine line_up;
    RansacLine line_down;
    RansacCircle circle_left;
    RansacCircle circle_right;

    cv::Point2f left_up;
    cv::Point2f left_down;
    cv::Point2f right_up;
    cv::Point2f right_down;

    //vector<cv::Point2f> line_up;
    //vector<cv::Point2f> line_down;
    //vector<cv::Point2f> circle_left;
    //vector<cv::Point2f> circle_right;

    //vector<int> select_line_up;
    //vector<int> select_line_down;
    //vector<int> select_circle_left;
    //vector<int> select_circle_right;

    //vector<pair<int, int>> valid_num_line_up;
    //vector<pair<int, int>> valid_num_line_down;
    //vector<pair<int, int>> valid_num_circle_left;
    //vector<pair<int, int>> valid_num_circle_right;
};

#pragma endregion