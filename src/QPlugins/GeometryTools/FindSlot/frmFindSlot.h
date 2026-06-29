#pragma once

#include <QDialog>
#include "ui_frmFindSlot.h"
#include "dependence/Toolnterface.h"
#include "dependence/QGraphicsViews.h"

struct localData
{
	QPointF ilt;
	QPointF irt;
	QPointF id;
	QPointF olt;
	//QVector<QPointF> slot
};

class frmFindSlot : public Toolnterface
{
	Q_OBJECT

public:
	frmFindSlot(QString toolName, QToolBase* toolBase, QWidget* parent = Q_NULLPTR);
	~frmFindSlot();

	virtual int InitSetToolData(const QVariant data);
	virtual QVariant InitGetToolData();

private slots:
	void on_btnAddROI_clicked();

private:
	Ui::frmFindSlotClass ui;

	QGraphicsViews* view;
	SlotItem* slot_item;
};

//全局变量控制
class QConfig
{
public:
	static int nFormState;
};
