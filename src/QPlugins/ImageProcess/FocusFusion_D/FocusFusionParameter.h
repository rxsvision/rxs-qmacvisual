#pragma once

#include <QDialog>
#include "ui_FocusFusionParameter.h"
#include "dependence/Toolnterface.h"
#include "dependence/utils.h"
#include <QMessageBox>
#include<qicon.h>

class FocusFusionParameter : public Toolnterface
{
	Q_OBJECT

public:
	FocusFusionParameter(QString toolName, QToolBase* toolBase, QWidget* parent = nullptr);
	~FocusFusionParameter();

	int SubExecuteLink(const int int_link, const QString str_link, const QMap<QString, gVariable::Global_Var> variable_link);

	QStringList strs;
	Ui::FocusFusionParameterClass ui;


public slots:
	void on_btnLinkImage_clicked();
	void onLinkChanged(QString text);

signals:
	void linkClicked();

private:
	gVariable gvariable;
	cv::Mat dstImage;
};
