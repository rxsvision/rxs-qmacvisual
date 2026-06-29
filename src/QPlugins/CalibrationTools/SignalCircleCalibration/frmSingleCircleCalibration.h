#pragma once

#include <QDialog>
#include<opencv2/opencv.hpp>
#include "ui_frmSingleCircleCalibration.h"
#include "Toolnterface.h"
#include "QGraphicsViews.h"

class InitSCCData
{
public:
	qreal x;
	qreal y;
	qreal RadiusMin;
	qreal RadiusMax;
	QVector<QPointF> ring_s_points;
	QVector<QPointF> ring_b_points;
	friend QDataStream& operator << (QDataStream& dataStream, const InitSCCData& data)
	{
		dataStream << data.x << data.y << data.RadiusMax << data.RadiusMin << data.ring_b_points << data.ring_s_points;
		return dataStream;
	}

	friend QDataStream& operator >> (QDataStream& dataStream, InitSCCData& data)
	{
		dataStream >> data.x >> data.y >> data.RadiusMax >> data.RadiusMin >> data.ring_b_points >> data.ring_s_points;
		return dataStream;
	}
};
Q_DECLARE_METATYPE(InitSCCData);


class frmSingleCircleCalibration : public Toolnterface
{
	Q_OBJECT

public:
	frmSingleCircleCalibration(QString toolName, QToolBase* toolBase, QWidget* parent = Q_NULLPTR);
	~frmSingleCircleCalibration();

private:
	Ui::frmSignalCircleCalibrationClass ui;

	void initConnect();
	QImage Mat2QImage(const cv::Mat& mat);
	virtual int Execute(const QString toolname);
	virtual int ExecuteLink(const int int_link, const QString str_link, const QMap<QString, gVariable::Global_Var> variable_link);
	virtual int InitSetToolData(const QVariant data);
	virtual QVariant InitGetToolData();

private:
	int RunToolPro();

private slots:
	void on_btnAddROI_clicked();
	void on_btnLinkImage_clicked();
	void on_btnExecute_clicked();


private:
	gVariable gvariable;
	QStringList strs;
	int image_index;
	int tool_index;

	QGraphicsViews* view;
	ConcentricCircleItem* concentric_circle_item;

	cv::Mat srcImage;
	cv::Mat dstImage;

	vector<QPointF> xy = vector<QPointF>(1000);//找到的边界点
	vector<cv::Point2f> select_xy = vector<cv::Point2f>(1000);
	vector<cv::Point2f> cull_xy = vector<cv::Point2f>(1000);

	cv::Vec3f fit_circle;
};

//全局变量控制
class QConfig
{
public:
	static int nFormState;
};
