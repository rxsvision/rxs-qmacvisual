#include "Ransac2D.h"
#include<opencv2/opencv.hpp>
#include <vector>

#pragma region LINE
void RansacLine::fit()
{
    auto points = sample();
    // 1. 计算点的中心 (x_mean, y_mean)
    float x_mean = 0.0f, y_mean = 0.0f;
    for (const auto& pt : points) {
        x_mean += pt.x;
        y_mean += pt.y;
    }
    x_mean /= points.size();
    y_mean /= points.size();

    // 2. 计算协方差矩阵元素
    float Sxx = 0.0f, Sxy = 0.0f, Syy = 0.0f;
    for (const auto& pt : points) {
        float dx = pt.x - x_mean;
        float dy = pt.y - y_mean;
        Sxx += dx * dx;
        Sxy += dx * dy;
        Syy += dy * dy;
    }

    // 3. 计算协方差矩阵特征值和特征向量
    cv::Mat cov = (cv::Mat_<float>(2, 2) << Sxx, Sxy, Sxy, Syy);
    cv::Mat eigenvalues, eigenvectors;
    cv::eigen(cov, eigenvalues, eigenvectors);

    // 特征向量对应最大特征值
    cv::Vec2f direction = eigenvectors.row(0); // 主方向

    // 4. 拟合结果 (a, b, c)
    a = -direction[1];  // 垂直于主方向的向量
    b = direction[0];
    c = -(a * x_mean + b * y_mean);
}

float RansacLine::pointToLineDistance(const cv::Point2f& point)
{
    // 点的坐标
    float x0 = point.x;
    float y0 = point.y;

    // 距离公式
    return std::fabs(a * x0 + b * y0 + c) / std::sqrt(a * a + b * b);
}

bool RansacLine::evaluate()
{
    int success_num = 0;
    for (auto& idx : select_samples)
    {
        if (pointToLineDistance(all_data[idx]) < threshold)
        {
            valid_num[idx].second++;
            success_num++;
        }
        else
            valid_num[idx].second--;
    }
    if (success_num > max_score)
    {
        max_score = success_num;
        best_a = a;
        best_b = b;
        best_c = c;
    }
    std::sort(valid_num.begin(), valid_num.end(), [](const std::pair<int, int>& a, const std::pair<int, int>& b) {return a.second > b.second; });
    return success_num > 0.9 * select_samples.size();
}
void RansacLine::useBest()
{
    a = best_a;
    b = best_b;
    c = best_c;
}
#pragma endregion

#pragma region CIECLE
void RansacCircle::fit()
{
    vector<cv::Point2f> points = sample();

    // 构造矩阵 A 和 B
    cv::Mat A(points.size(), 3, CV_32F);  // [x, y, 1]
    cv::Mat B(points.size(), 1, CV_32F);  // [x^2 + y^2]

    for (size_t i = 0; i < points.size(); ++i)
    {
        A.at<float>(i, 0) = points[i].x;
        A.at<float>(i, 1) = points[i].y;
        A.at<float>(i, 2) = 1.0f;

        B.at<float>(i, 0) = points[i].x * points[i].x + points[i].y * points[i].y;
    }

    // 求解最小二乘法方程 A * [D, E, F] = B
    cv::Mat X;
    cv::solve(A, B, X, cv::DECOMP_SVD);

    // 解得 D, E, F
    float D = X.at<float>(0, 0);
    float E = X.at<float>(1, 0);
    float F = X.at<float>(2, 0);

    // 计算圆心 (a, b) 和半径 r
    a = D / 2.0f;
    b = E / 2.0f;
    r = std::sqrt(a * a + b * b + F);
}

bool RansacCircle::fit_3Point(const cv::Point2f& p1, const cv::Point2f& p2, const cv::Point2f& p3) {
    // 使用三个点的坐标来构建拟合圆的方程
    float x1 = p1.x, y1 = p1.y;
    float x2 = p2.x, y2 = p2.y;
    float x3 = p3.x, y3 = p3.y;

    // 计算拟合圆的系数
    float d = 2 * (x1 * (y2 - y3) + x2 * (y3 - y1) + x3 * (y1 - y2));
    a = ((x1 * x1 + y1 * y1) * (y2 - y3) + (x2 * x2 + y2 * y2) * (y3 - y1) + (x3 * x3 + y3 * y3) * (y1 - y2)) / d;
    b = ((x1 * x1 + y1 * y1) * (x3 - x2) + (x2 * x2 + y2 * y2) * (x1 - x3) + (x3 * x3 + y3 * y3) * (x2 - x1)) / d;
    r = std::sqrt((x1 - a) * (x1 - a) + (y1 - b) * (y1 - b));  // 半径

    return true;
}



// 计算点到圆的距离
float RansacCircle::distanceToCircle(const cv::Point2f& point) {
    // 计算点到圆心的距离
    float d_center = std::sqrt(std::pow(point.x - a, 2) + std::pow(point.y - b, 2));

    // 计算点到圆的距离
    return std::abs(d_center - r);
}

bool RansacCircle::evaluate()
{
    int success_num = 0;
    if (ref_radius > 0 && abs(ref_radius - r) > ref_radius * 0.05)
    {
        for (auto& idx : select_samples)
        {
            valid_num[idx].second--;
        }
        return false;
    }

    for (int idx=0;idx<all_data.size();idx++)
    {
        if (distanceToCircle(all_data[idx]) < threshold)
        {
            valid_num[idx].second++;
            success_num++;
        }
        else
            valid_num[idx].second--;
    }
    if (success_num > max_score)
    {
        max_score = success_num;
        best_a = a;
        best_b = b;
        best_r = r;

    }
    std::sort(valid_num.begin(), valid_num.end(), [](const std::pair<int, int>& a, const std::pair<int, int>& b) {return a.second > b.second; });
    return success_num > 0.8 * all_data.size();
}
void RansacCircle::useBest()
{
    r = best_r;
    a = best_a;
    b = best_b;
}
#pragma endregion


#pragma region SLOT
RansacSlot::RansacSlot(vector<cv::Point2f> line_up_, vector<cv::Point2f> line_down_, vector<cv::Point2f> circle_left_, vector<cv::Point2f> circle_right_)
    :line_up(line_up_), line_down(line_down_), circle_left(circle_left_), circle_right(circle_right_)
{
}

//vector<cv::Point2f> sample(const vector<cv::Point2f>& all_data, vector<int>& select_samples)
//{
//    int valid_range = all_data.size() * 0.8;
//
//    // 创建一个从0到n的整数序列
//    std::vector<int> numbers;
//    for (int i = 0; i < valid_range; ++i) {
//        numbers.push_back(i);
//    }
//
//    // 随机生成随机数生成器
//    std::random_device rd;
//    std::mt19937 gen(rd());
//
//    // 使用 random_shuffle 打乱 numbers
//    std::shuffle(numbers.begin(), numbers.end(), gen);
//    int sample_count = valid_range * 0.8;
//
//    select_samples = std::vector<int>(numbers.begin(), numbers.begin() + sample_count);
//
//    vector<cv::Point2f> ret;
//    for (int i = 0; i < select_samples.size(); i++)
//        ret.push_back(all_data[select_samples[i]]);
//    return ret;
//}

cv::Point2f calculateFootOfPerpendicular(float x0, float y0, float a, float b, float c) {
    cv::Point2f foot;
    double denominator = a * a + b * b;

    // 计算垂足点的 x 和 y 坐标
    foot.x = (b * (b * x0 - a * y0) - a * c) / denominator;
    foot.y = (a * (-b * x0 + a * y0) - b * c) / denominator;

    return foot;
}

void RansacSlot::fit()
{
    line_up.sample_ratio = 0.2;
    line_up.run();
    line_down.sample_ratio = 0.2;
    line_down.run();
    circle_left.sample_ratio = 0.1;
    circle_left.run();
    circle_right.sample_ratio = 0.1;
    circle_right.run();

    //cv::Point2f center_left(circle_left.a, circle_left.b);
    //cv::Point2f center_right(circle_right.a, circle_right.b);

    left_up = calculateFootOfPerpendicular(circle_left.a, circle_left.b, line_up.a, line_up.b, line_up.c);
    left_down = calculateFootOfPerpendicular(circle_left.a, circle_left.b, line_down.a, line_down.b, line_down.c);

    right_up = calculateFootOfPerpendicular(circle_right.a, circle_right.b, line_up.a, line_up.b, line_up.c);
    //cv::Point2f right_down = calculateFootOfPerpendicular(circle_right.a, circle_right.b, line_down.a, line_down.b, line_down.c);
    right_down = right_up + left_down - left_up;
}

//bool RansacSlot::evaluate()
//{
//}
//
//void RansacSlot::run()
//{
//    for (int _ = 0; _ < 100; _++)
//    {
//        fit();
//        if (evaluate()) break;
//    }
//}

#pragma endregion