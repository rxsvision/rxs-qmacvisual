#pragma once

#include <QWidget>
#include "ui_frmFlatness3D.h"
#include "dependence/Toolnterface.h"
#include "dependence/QGraphicsViews.h"
#include "dependence/utils.h"
#include <QMessageBox>

class frmFlatness3DData
{
public:
	qreal x;
	qreal y;
	qreal width;
	qreal height;

	friend QDataStream& operator << (QDataStream& dataStream, const frmFlatness3DData& data)
	{
		dataStream << data.x << data.y << data.width << data.height;
		return dataStream;
	}

	friend QDataStream& operator >> (QDataStream& dataStream, frmFlatness3DData& data)
	{
		dataStream >> data.x >> data.y >> data.width >> data.height;
		return dataStream;
	}

};
Q_DECLARE_METATYPE(frmFlatness3DData);


class frmFlatness3D : public Toolnterface
{
	Q_OBJECT

public:
	frmFlatness3D(QString toolName, QToolBase* toolBase, QWidget* parent = nullptr);
	~frmFlatness3D();

	virtual int ExecuteLink(const int int_link, const QString str_link, const QMap<QString, gVariable::Global_Var> variable_link);
	virtual int InitSetToolData(const QVariant data);
	virtual QVariant InitGetToolData();
	virtual int Execute(const QString toolname);
	VectorXf planeFitLSM(CP cloud);
	float Flatness(CP cloud, PointT& min_point, PointT& max_point);
	int RunToolPro();


public slots:
	void on_btnAddROI_clicked();
	void on_btnLinkFollow_clicked();
	void on_btnLinkImage_clicked();
	void on_btnExecute_clicked();
	void on_btnDeleteROI_clicked();



private:
	Ui::frmFlatness3DClass ui;
	gVariable gvariable;
	QGraphicsViews* view;
	cv::Mat dstImage;
	int tool_index = 0;
	QStringList strs;


	cv::Rect roi;
	RectangleItem* rectangle_item;
};


//全局变量控制
class QConfig
{
public:
	static int nFormState;
};
