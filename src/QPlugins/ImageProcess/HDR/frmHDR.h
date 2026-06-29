#pragma once

#include <QDialog>
#include "ui_frmHDR.h"
#include "dependence/Toolnterface.h"
#include "dependence/QGraphicsViews.h"
#include "dependence/utils.h"
#include "HDRParameter.h"
#include <QMessageBox>
#include <qtabbar.h>

class frmHDRData
{
public:
	QVector<QString> img_links;

	qreal x;
	qreal y;
	qreal width;
	qreal height;

	friend QDataStream& operator << (QDataStream& dataStream, const frmHDRData& data)
	{
		dataStream << data.img_links << data.x << data.y << data.width << data.height;
		return dataStream;
	}

	friend QDataStream& operator >> (QDataStream& dataStream, frmHDRData& data)
	{
		dataStream >> data.img_links >> data.x >> data.y >> data.width >> data.height;
		return dataStream;
	}

};

Q_DECLARE_METATYPE(frmHDRData);


class frmHDR : public Toolnterface
{
	Q_OBJECT

public:
	frmHDR(QString toolName, QToolBase* toolBase, QWidget* parent = nullptr);
	~frmHDR();

	virtual int ExecuteLink(const int int_link, const QString str_link, const QMap<QString, gVariable::Global_Var> variable_link);
	virtual int InitSetToolData(const QVariant data);
	virtual QVariant InitGetToolData();
	virtual int Execute(const QString toolname);
	int RunToolPro();

	virtual bool eventFilter(QObject* obj, QEvent* event) override;


public slots:
	void onTabClicked(int index);
	void onTabClose(bool);
	void on_subBtnLinkImage_clicked();

	void on_btnAddROI_clicked();
	void on_btnLinkFollow_clicked();
	void on_btnLinkImage_clicked();
	void on_btnExecute_clicked();
	void on_btnDeleteROI_clicked();

signals:
	void clickTab(int value);

private:
	Ui::frmFocusFusionClass ui;

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
