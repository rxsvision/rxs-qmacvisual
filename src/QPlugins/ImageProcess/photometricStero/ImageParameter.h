#pragma once

#include <QWidget>
#include "ui_ImageParameter.h"
#include "dependence/Toolnterface.h"
#include "dependence/utils.h"

class ImageParameterData
{
public:
	QString img_link;
	QString slant;
	QString tilt;

friend QDataStream& operator << (QDataStream& dataStream, const ImageParameterData& data)
{
	dataStream << data.img_link << data.slant << data.tilt;
	return dataStream;
}

friend QDataStream& operator >> (QDataStream& dataStream, ImageParameterData& data)
{
	dataStream >> data.img_link >> data.slant >> data.tilt;
	return dataStream;
}
};

class ImageParameter : public Toolnterface
{
	Q_OBJECT

public:
	ImageParameter(QString toolName, QToolBase* toolBase, QWidget* parent = nullptr);
	~ImageParameter();

	void contextMenuEvent(QContextMenuEvent* event) override;

	int SubExecuteLink(const int int_link, const QString str_link, const QMap<QString, gVariable::Global_Var> variable_link);

	ImageParameterData getData();
	void setData(ImageParameterData data);


	Ui::ImageParameterClass ui;
	QStringList strs;


public slots:
	void on_btnLinkImage_clicked();
	void onLinkChanged(QString text);

signals:
	void linkClicked();

private:

	gVariable gvariable;

	int image_index;
	int tool_index;
	cv::Mat dstImage;
};

////全局变量控制
//class QConfig
//{
//public:
//	static int nFormState;
//};
