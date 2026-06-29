#pragma once
#include"czxDependence/czxTool.h"

class PointGrid
{
public:
	PointGrid(CP cloud, float interval_x=0.008, float interval_y=0.008);
	PointGrid(CP cloud, PointGrid& other);
	PointGrid(const PointGrid&) = delete; // 禁用拷贝构造函数
	PointGrid& operator=(const PointGrid&) = delete; // 禁用赋值运算符
	~PointGrid();

	float nDis(int n=5);
	float disRC(const int& row_dis, const int& col_dis);
	float disRCSquare(const int& row_dis, const int& col_dis);
	
	
	void BoundDetect(int range, const int& r, const int& c, int& r_down, int& r_up, int& c_down, int& c_up);

	//float getNearestPoint(const PointT& p, const int& r, const int& c, int& nearest_r, int& nearest_c);  //临时弃用
	float getNearestPoint(const PointT& p, const int& r, const int& c);

	inline void indexCalculate(const PointT& p, int& i, int& j);

	void initParameter(CP& cloud, float interval_x, float interval_y);
	void initImage(const CP& cloud);

	// 重载 operator[] 以返回指向 float* 的引用
	float* operator[](int index) {
		return depth_img[index];
	}

	// 重载 const 版本的 operator[]，以便在 const 对象上使用
	const float* operator[](int index) const {
		return depth_img[index];
	}


private:
	//std::vector<std::vector<std::vector<pcl::PointXYZ>>> data;
	float** depth_img;
	bool** depth_map;
	int width;
	int height;
	float interval_x;
	float interval_y;
	float min_interval;
	PointT min_point;
	float empty_value = 999.0f;
};

