#pragma once

#include <QWidget>
#include "ui_frmPhotometricStero.h"
#include "dependence/Toolnterface.h"
#include "dependence/QGraphicsViews.h"
#include "dependence/utils.h"
#include"ImageParameter.h"

class frmPhotometricSteroData
{
public:
	QString img_link;
	QVector<ImageParameterData> img_parameters;
	qreal x;
	qreal y;
	qreal width;
	qreal height;

friend QDataStream& operator << (QDataStream& dataStream, const frmPhotometricSteroData& data)
{
	dataStream << data.img_link << data.img_parameters << data.x << data.y << data.width << data.height;
	return dataStream;
}

friend QDataStream& operator >> (QDataStream& dataStream, frmPhotometricSteroData& data)
{
	dataStream >> data.img_link >> data.img_parameters >> data.x >> data.y >> data.width >> data.height;
	return dataStream;
}
};

Q_DECLARE_METATYPE(frmPhotometricSteroData);

class frmPhotometricStero : public Toolnterface
{
	Q_OBJECT

public:
	frmPhotometricStero(QString toolName, QToolBase* toolBase, QWidget *parent = nullptr);
	bool eventFilter(QObject* obj, QEvent* event) override;
	~frmPhotometricStero();

	virtual int ExecuteLink(const int int_link, const QString str_link, const QMap<QString, gVariable::Global_Var> variable_link);
	virtual int InitSetToolData(const QVariant data);
	virtual QVariant InitGetToolData();
	virtual int Execute(const QString toolname);

	int RunToolPro();

public slots:
	void onTabClicked(int index);
	void onTabClose(bool);
	void on_subBtnLinkImage_clicked();

	void on_btnAddROI_clicked();
	void on_btnLinkFollow_clicked();
	void on_btnLinkImage_clicked();
	void on_btnExecute_clicked();
	void onLinkChanged(QString text);
	void on_btnDeleteROI_clicked();

signals:
	void clickTab(int value);

private:
	Ui::frmPhotometricSteroClass ui;

	gVariable gvariable;
	QGraphicsViews* view;
	cv::Mat dstImage;

	QStringList strs;

	RectangleItem* rectangle_item;
	//RectangleItem* rectangle_item_visual;//TODO

	cv::Rect roi;

	int tool_index = 0;
};


//全局变量控制
class QConfig
{
public:
	static int nFormState;
};
