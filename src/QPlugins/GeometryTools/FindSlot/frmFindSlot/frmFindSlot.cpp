#include "frmFindSlot.h"

frmFindSlot::frmFindSlot(QString toolName, QToolBase* toolBase, QWidget* parent)
	: Toolnterface(toolName, toolBase, parent)
{
	ui.setupUi(this);
	view = new QGraphicsViews;
	ui.imageLayout->addWidget(view);
	slot_item = new SlotItem(0, 0, 200, 100);
}

frmFindSlot::~frmFindSlot()
{
	view->deleteLater();
	slot_item->deleteLater();
	this->deleteLater();
}

int frmFindSlot::InitSetToolData(const QVariant data)
{
	return 0;
}

QVariant frmFindSlot::InitGetToolData()
{
	//QPointF ilt;
	//QPointF irt;
	//QPointF id;
	//QPointF olt;
	localData ld;
	ld.ilt = slot_item->getControlPoints()[0];
	ld.irt = slot_item->getControlPoints()[0];
	ld.id = slot_item->getControlPoints()[0];
	ld.olt = slot_item->getControlPoints()[0];
	return QVariant();
}

void frmFindSlot::on_btnAddROI_clicked()
{
	view->ClearObj();
	slot_item = new SlotItem(0,0,200,100);
	view->AddItems(slot_item);
}

//全局变量控制
int QConfig::nFormState = 0;