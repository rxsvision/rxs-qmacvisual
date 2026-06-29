#include "PointGrid.h"
#include<pcl/common/common.h>
#include <cmath>

void PointGrid::initParameter(CP& cloud, float interval_x_, float interval_y_)
{
	this->interval_x = interval_x_;
	this->interval_y = interval_y_;
	min_interval = min(interval_x, interval_y);
	PointT min_p, max_p;
	pcl::getMinMax3D(*cloud, min_p, max_p);
	min_point = min_p;
	width = static_cast<int>(ceil((max_p.x - min_p.x) / interval_x)) + 1;
	height = static_cast<int>(ceil((max_p.y - min_p.y) / interval_y)) + 1;
}

void PointGrid::initImage(const CP& cloud)
{
	// 初始化二维数组
	depth_img = new float* [height];
	depth_map = new bool* [height];
	for (int i = 0; i < height; ++i)
	{
		depth_img[i] = new float[width];
		depth_map[i] = new bool[width];
		std::fill(depth_img[i], depth_img[i] + width, empty_value);
		for (int j = 0; j < width; ++j) 
		{
			depth_map[i][j] = false;
		}
	}

	for (auto& p : *cloud)
	{
		int i, j;
		indexCalculate(p, i, j);
		depth_img[i][j] = p.z;
		depth_map[i][j] = true;
	}
}

PointGrid::PointGrid(CP cloud, float interval_x_, float interval_y_)
{
	initParameter(cloud, interval_x_, interval_y_);

	initImage(cloud);
}

PointGrid::PointGrid(CP cloud, PointGrid& other)
{
	interval_x = other.interval_x;
	interval_y = other.interval_y;
	min_interval = min(interval_x, interval_y);
	PointT min_p, max_p;
	pcl::getMinMax3D(*cloud, min_p, max_p);
	if(min_p.x>other.min_point.x || min_p.y>other.min_point.y)
		throw std::runtime_error("最低点不一致，点云尺寸无法统一");
	min_point = other.min_point;
	int width_c = static_cast<int>(ceil((max_p.x - min_point.x) / interval_x));
	int height_c = static_cast<int>(ceil((max_p.y - min_point.y) / interval_y));

	if(width_c>other.width || height_c>other.height)
		throw std::runtime_error("图像尺寸无法统一");
	width = width_c;
	height = height_c;

	initImage(cloud);
}

PointGrid::~PointGrid()
{
	for (int i = 0; i < height; ++i)
	{
		delete[] depth_img[i];
		delete[] depth_map[i];
	}
	delete[] depth_img;
	delete[] depth_map;
}

float PointGrid::nDis(int n)
{
	return sqrt(n * interval_x * n * interval_x + n * interval_y * n * interval_y);
}

float PointGrid::disRC(const int& row, const int& col)
{
	return sqrt(disRCSquare(row, col));
}

float PointGrid::disRCSquare(const int& row_dis, const int& col_dis)
{
	return (row_dis * interval_y) * (row_dis * interval_y) + (col_dis * interval_x) * (col_dis * interval_x);
}

void PointGrid::BoundDetect(int range, const int& r, const int& c, int& r_down, int& r_up, int& c_down, int& c_up)
{
	r_down = max(-range, - r);
	r_up = min(range, height - r - 1);
	c_down = max(-range, -c);
	c_up = min(range, width - c - 1);
}

//float PointGrid::getNearestPoint(const PointT& p, const int& r, const int& c, int& nearest_r, int& nearest_c)
//{
//	float end = max(height, width);
//	float min_dis = std::numeric_limits<float>::max();
//	for (int manhattan = 0; manhattan < end; manhattan++)
//	{
//		for (int r_off = 0; r_off <= manhattan; r_off++)
//		{
//			if (r - r_off >= 0 && c-(manhattan-r_off)>=0&& r - r_off<height&& c - (manhattan - r_off)<width&&depth_map[r - r_off][c - (manhattan - r_off)])
//			{
//				float z_dif = p.z - depth_img[r - r_off][c - (manhattan - r_off)];
//				float dis = sqrt(z_dif * z_dif + disRCSquare(r_off, manhattan - r_off));
//				if (dis < min_dis)
//				{
//					nearest_r = r - r_off;
//					nearest_c = c - (manhattan - r_off);
//					min_dis = dis;
//					end = min((dis / min_interval), end);
//				}
//			}
//
//			if (r - r_off >= 0 && r - r_off < height && c + (manhattan - r_off) < width && c + (manhattan - r_off) >=0 && depth_map[r - r_off][c + (manhattan - r_off)])
//			{
//				float z_dif = p.z - depth_img[r - r_off][c + (manhattan - r_off)];
//				float dis = sqrt(z_dif * z_dif + disRCSquare(r_off, manhattan - r_off));
//				if (dis < min_dis)
//				{
//					nearest_r = r - r_off;
//					nearest_c = c + (manhattan - r_off);
//					min_dis = dis;
//					end = min((dis / min_interval), end);
//				}
//			}
//
//			if (r + r_off < height && r + r_off >=0 && c - (manhattan - r_off) >= 0 && c - (manhattan - r_off) < width && depth_map[r + r_off][c - (manhattan - r_off)])
//			{
//				float z_dif = p.z - depth_img[r + r_off][c - (manhattan - r_off)];
//				float dis = sqrt(z_dif * z_dif + disRCSquare(r_off, manhattan - r_off));
//				if (dis < min_dis)
//				{
//					nearest_r = r + r_off;
//					nearest_c = c - (manhattan - r_off);
//					min_dis = dis;
//					end = min((dis / min_interval), end);
//				}
//			}
//
//			if (r + r_off < height && r + r_off >=0 && c + (manhattan - r_off) < width && c + (manhattan - r_off) >=0 && depth_map[r + r_off][c + (manhattan + r_off)])
//			{
//				float z_dif = p.z - depth_img[r + r_off][c + (manhattan - r_off)];
//				float dis = sqrt(z_dif * z_dif + disRCSquare(r_off, manhattan - r_off));
//				if (dis < min_dis)
//				{
//					nearest_r = r + r_off;
//					nearest_c = c + (manhattan - r_off);
//					min_dis = dis;
//					end = min((dis / min_interval), end);
//				}
//			}
//		}
//	}
//	return min_dis;
//}

float PointGrid::getNearestPoint(const PointT& p, const int& r, const int& c)
{
	float end = max(height, width);
	float min_dis = std::numeric_limits<float>::max();
	for (int manhattan = 0; manhattan < end; manhattan++)
	{
		for (int r_off = 0; r_off <= manhattan; r_off++)
		{
			if (r - r_off >= 0 && c - (manhattan - r_off) >= 0 && r - r_off < height && c - (manhattan - r_off) < width)
			{
				float z_dif = p.z - depth_img[r - r_off][c - (manhattan - r_off)];
				float dis = sqrt(z_dif * z_dif + disRCSquare(r_off, manhattan - r_off));
				if (dis < min_dis)
				{
					min_dis = dis;
					end = min((dis / min_interval), end);
				}
			}

			if (r - r_off >= 0 && r - r_off < height && c + (manhattan - r_off) < width && c + (manhattan - r_off) >= 0)
			{
				float z_dif = p.z - depth_img[r - r_off][c + (manhattan - r_off)];
				float dis = sqrt(z_dif * z_dif + disRCSquare(r_off, manhattan - r_off));
				if (dis < min_dis)
				{
					min_dis = dis;
					end = min((dis / min_interval), end);
				}
			}

			if (r + r_off < height && r + r_off >= 0 && c - (manhattan - r_off) >= 0 && c - (manhattan - r_off) < width)
			{
				float z_dif = p.z - depth_img[r + r_off][c - (manhattan - r_off)];
				float dis = sqrt(z_dif * z_dif + disRCSquare(r_off, manhattan - r_off));
				if (dis < min_dis)
				{
					min_dis = dis;
					end = min((dis / min_interval), end);
				}
			}

			if (r + r_off < height && r + r_off >= 0 && c + (manhattan - r_off) < width && c + (manhattan - r_off) >= 0)
			{
				float z_dif = p.z - depth_img[r + r_off][c + (manhattan - r_off)];
				float dis = sqrt(z_dif * z_dif + disRCSquare(r_off, manhattan - r_off));
				if (dis < min_dis)
				{
					min_dis = dis;
					end = min((dis / min_interval), end);
				}
			}
		}
	}
	return min_dis;
}

void PointGrid::indexCalculate(const PointT& p, int& i, int& j)
{
	j = (p.x - min_point.x) / interval_x;
	i = (p.y - min_point.y) / interval_y;
}

