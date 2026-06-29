#include "frmCameraSetUp.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QGraphicsPixmapItem>
#include <QDesktopWidget>
#include <QPainter>
#include <QWidgetAction>
#include <QElapsedTimer>
#include "datavar.h"

frmCameraSetUp::frmCameraSetUp(QWidget* parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	//FramelessWindowHint属性设置窗口去除边框
	//WindowMinimizeButtonHint 属性设置在窗口最小化时，点击任务栏窗口可以显示出原窗口
	this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
	//设置窗体在屏幕中间位置
	QDesktopWidget* desktop = QApplication::desktop();
	move((desktop->width() - this->width()) / 2, (desktop->height() - this->height()) / 2);
	//设置窗口背景透明
	setAttribute(Qt::WA_TranslucentBackground);
	this->setWindowIcon(QIcon(":/res/ico/cam.png"));
	//初始化标题栏
	initTitleBar();
	//设置表格列宽	
	ui.tableWidget->setColumnWidth(0, 256);
	//隐藏水平header
	ui.tableWidget->verticalHeader()->setVisible(false);
	ui.tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);  //设置选择行
	ui.tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);  //设置只能单选
	connect(ui.tableWidget, SIGNAL(cellDoubleClicked(int, int)), this, SLOT(slot_DoubleClicked(int, int)));
	ReadOnlyDelegate* readOnlyDelegate = new ReadOnlyDelegate(this);
	ui.tableWidget->setItemDelegateForColumn(0, readOnlyDelegate); //设置第1列只读	
	//添加右键菜单
	AddRightButtonMenu();
	mindvision_camera_state = 0;
}

frmCameraSetUp::~frmCameraSetUp()
{	
	this->deleteLater();
}

void frmCameraSetUp::initTitleBar()
{
	MyTitleBar* m_titleBar = new MyTitleBar(this);
	m_titleBar->move(0, 0);
	connect(m_titleBar, SIGNAL(signalButtonCloseClicked()), this, SLOT(onButtonCloseClicked()));
	m_titleBar->setBackgroundColor(3, 110, 95);
	m_titleBar->setStyleSheet("background-color: rgba(0, 0, 0,0);color:white;font-size:16px");
	m_titleBar->setTitleIcon(":/res/ico/cam.png");
	m_titleBar->setTitleContent("相机设置");
	m_titleBar->setButtonType(ONLY_CLOSE_BUTTON);
	m_titleBar->setTitleWidth(this->width());
}

void frmCameraSetUp::paintEvent(QPaintEvent* event)
{
	//设置背景色
	QPainter painter(this);
	QPainterPath pathBack;
	pathBack.setFillRule(Qt::WindingFill);
	pathBack.addRoundedRect(QRect(0, 0, this->width(), this->height()), 3, 3);
	painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
	painter.fillPath(pathBack, QBrush(QColor("#bcc2bc")));
	return QWidget::paintEvent(event);
}

void frmCameraSetUp::onButtonCloseClicked()
{
	this->close();
}

//右键菜单
void frmCameraSetUp::AddRightButtonMenu()
{
	QMenu* m_menu = new QMenu(this);
	//菜单样式
	QString menuStyle(
		"QMenu::item{"
		"color: rgb(0, 0, 0);"
		"}"
		"QMenu::item:hover{"
		"background-color: rgb(176, 190, 225);"
		"}"
		"QMenu::item:selected{"
		"background-color: rgb(176, 190, 225);"
		"}"
	);
	QString buttonStyle(
		"QPushButton{"
		"color: rgb(0, 0, 0);"
		"border: none;"
		"text-align: left;"
		"font: 9pt Microsoft YaHei;"
		"padding-left: 5px;"
		"padding-right: 5px;"
		"min-height: 23px;"
		"}"
		"QPushButton::hover{"
		"background-color: rgba(176, 190, 225, 200);"
		"}"
		"QPushButton::selected{"
		"background-color: rgba(176, 190, 255, 255);"
		"}"
	);
	m_menu->setStyleSheet(menuStyle);
	QPushButton* moveUpAction_buf = new QPushButton(m_menu);
	moveUpAction_buf->setStyleSheet(buttonStyle);
	moveUpAction_buf->setText("上移");
	moveUpAction_buf->setIconSize(QSize(20, 20));
	moveUpAction_buf->setIcon(QIcon(":/res/ico/previous.png"));
	QWidgetAction* moveUpAction = new QWidgetAction(this);
	moveUpAction->setDefaultWidget(moveUpAction_buf);
	connect(moveUpAction_buf, &QPushButton::clicked, this, &frmCameraSetUp::slot_MoveUp);
	connect(moveUpAction_buf, &QPushButton::clicked, m_menu, &QMenu::close);
	m_menu->addAction(moveUpAction);
	QPushButton* moveDownAction_buf = new QPushButton(m_menu);
	moveDownAction_buf->setStyleSheet(buttonStyle);
	moveDownAction_buf->setText("下移");
	moveDownAction_buf->setIconSize(QSize(20, 20));
	moveDownAction_buf->setIcon(QIcon(":/res/ico/next.png"));
	QWidgetAction* moveDownAction = new QWidgetAction(this);
	moveDownAction->setDefaultWidget(moveDownAction_buf);
	connect(moveDownAction_buf, &QPushButton::clicked, this, &frmCameraSetUp::slot_MoveDown);
	connect(moveDownAction_buf, &QPushButton::clicked, m_menu, &QMenu::close);
	m_menu->addAction(moveDownAction);
	QPushButton* deleteNameAction_buf = new QPushButton(m_menu);
	deleteNameAction_buf->setStyleSheet(buttonStyle);
	deleteNameAction_buf->setText("删除");
	deleteNameAction_buf->setIconSize(QSize(21, 21));
	deleteNameAction_buf->setIcon(QIcon(":/res/ico/del.png"));
	QWidgetAction* deleteNameAction = new QWidgetAction(this);
	deleteNameAction->setDefaultWidget(deleteNameAction_buf);
	connect(deleteNameAction_buf, &QPushButton::clicked, this, &frmCameraSetUp::slot_DeleteName);
	connect(deleteNameAction_buf, &QPushButton::clicked, m_menu, &QMenu::close);
	m_menu->addAction(deleteNameAction);
	//给控件设置上下文菜单策略
	ui.tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
	//给信号设置相应的槽函数
	connect(ui.tableWidget, &QTableWidget::customContextMenuRequested, [=](const QPoint& pos)
		{
			//参数pos用来传递右键点击时的鼠标的坐标，这个坐标一般是相对于控件左上角
			m_menu->exec(QCursor::pos());
		});
}

void frmCameraSetUp::on_btnAddCamera_clicked()
{
	if (ui.comboCameraName->currentText() == QString())
	{
		emit dataVar::fProItemTab->sig_WarnClick();
		emit dataVar::fProItemTab->sig_Log("未找到或选择相机！");
		QMessageBox msgBox(QMessageBox::Icon::NoIcon, "警告", "未找到或选择相机！");
		msgBox.setWindowIcon(QIcon(":/res/ico/warn.png"));
		msgBox.exec();
		return;
	}
	else
	{
		int count = ui.tableWidget->rowCount();
		QString cam_name = ui.comboCamera->currentText() + "_" + ui.comboCameraName->currentText();
		for (int m = 0; m < count; m++)
		{
			if (cam_name == ui.tableWidget->item(m, 0)->text())
			{
				emit dataVar::fProItemTab->sig_ErrorClick();
				emit dataVar::fProItemTab->sig_Log("已存在该相机名称！");
				QMessageBox msgBox(QMessageBox::Icon::NoIcon, "错误", "已存在该相机名称！");
				msgBox.setWindowIcon(QIcon(":/resource/error.png"));
				msgBox.exec();
				return;
			}
		}
		ui.tableWidget->setRowCount(count + 1);  //设置行数		
		QTableWidgetItem* item_type = new QTableWidgetItem(cam_name);
		item_type->setFlags(item_type->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
		ui.tableWidget->setItem(count, 0, item_type);
	}
}

void frmCameraSetUp::on_btnSearchCamera_clicked()
{
	//搜索相机
	ui.comboCameraName->clear();	
	switch (ui.comboCamera->currentIndex()) {
	case 0:	
		tSdkCameraDevInfo sCameraList_buf[9];
		iCameraNums = 9;		
		//返回的相机信息列表，会根据acFriendlyName排序
		if (CameraEnumerateDevice(sCameraList_buf, &iCameraNums) != CAMERA_STATUS_SUCCESS || iCameraNums == 0)
		{
			emit dataVar::fProItemTab->sig_ErrorClick();
			emit dataVar::fProItemTab->sig_Log("未搜索到迈德威视相机！");
			QMessageBox msgBox(QMessageBox::Icon::NoIcon, "错误", "未搜索到迈德威视相机！");
			msgBox.setWindowIcon(QIcon(":/res/ico/error.png"));
			msgBox.exec();
			return;
		}
		sCameraList.clear();
		for (int i = 0; i < iCameraNums; i++)
		{
			sCameraList.append(sCameraList_buf[i]);
			ui.comboCameraName->addItem(sCameraList_buf[i].acFriendlyName);
		}
		break;
	case 1:
		//海康威视

		break;
	}
}

void frmCameraSetUp::on_btnConnect_clicked()
{
	//连接相机	
	if (ui.lblType->text() == QString())
	{
		emit dataVar::fProItemTab->sig_ErrorClick();
		emit dataVar::fProItemTab->sig_Log("选择相机名称为空！");
		QMessageBox msgBox(QMessageBox::Icon::NoIcon, "错误", "选择相机名称为空！");
		msgBox.setWindowIcon(QIcon(":/res/ico/error.png"));
		msgBox.exec();
		return;
	}
	QStringList strs = ui.lblType->text().split("_");
	if (strs[0] == "MindVision")
	{
		//迈德威视
		camera_keys.clear();
		camera_keys = global_camera_content.uniqueKeys();
		int count = 0;
		int ccd_Index = 0;
		for (int k = 0; k < camera_keys.length(); k++)
		{
			QString key = camera_keys[k];
			if (key == ui.lblType->text())
			{
				ccd_Index = k;
				count = 1;
				ui.spinExposure->setValue(global_camera_content.value(key).global_exposure);
				ui.spinGain->setValue(global_camera_content.value(key).global_gain);
				ui.spinGamma->setValue(global_camera_content.value(key).global_gamma);
				ui.spinContrast->setValue(global_camera_content.value(key).global_contrast);
				ui.comboTriggerMode->setCurrentText(global_camera_content.value(key).global_trigger_mode);
				ui.spinTimeOut->setValue(global_camera_content.value(key).global_timeout);
				CameraContent.ccd_index = ccd_Index;
				global_camera_content.insert(key, CameraContent);
				break;
			}
		}
		if (count == 0)
		{
			emit dataVar::fProItemTab->sig_ErrorClick();
			emit dataVar::fProItemTab->sig_Log("该相机参数未保存！");
			QMessageBox msgBox(QMessageBox::Icon::NoIcon, "错误", "该相机参数未保存！");
			msgBox.setWindowIcon(QIcon(":/res/ico/error.png"));
			msgBox.exec();
			return;
		}
		for (int n = 0; n < dataVar::camera_state.count(); n++)
		{
			if (dataVar::camera_state[n] == ui.lblType->text())
			{
				emit dataVar::fProItemTab->sig_ErrorClick();
				emit dataVar::fProItemTab->sig_Log(ui.lblType->text() + "相机已连接！");
				QMessageBox msgBox(QMessageBox::Icon::NoIcon, "错误", ui.lblType->text() + "相机已连接！");
				msgBox.setWindowIcon(QIcon(":/res/ico/error.png"));
				msgBox.exec();
				return;
			}
		}
		//初始化相机，(-1,-1)表示加载上次退出前保存的参数，如果是第一次使用该相机，则加载默认参数
		if ((status = CameraInit(&sCameraList[ccd_Index], -1, -1, &mindvision_haldle)) != CAMERA_STATUS_SUCCESS)
		{
			emit dataVar::fProItemTab->sig_ErrorClick();
			emit dataVar::fProItemTab->sig_Log("初始化相机失败！");
			QMessageBox msgBox(QMessageBox::Icon::NoIcon, "错误", "初始化相机失败！");
			msgBox.setWindowIcon(QIcon(":/res/ico/error.png"));
			msgBox.exec();
			return;
		}
		//获得该相机的特性描述
		CameraGetCapability(mindvision_haldle, &sCameraInfo);
		mindvision_framebuffer = (BYTE*)CameraAlignMalloc(sCameraInfo.sResolutionRange.iWidthMax * sCameraInfo.sResolutionRange.iWidthMax * 3, 16);
		//设置曝光
		CameraSetAeState(mindvision_haldle, false);
		CameraSetExposureTime(mindvision_haldle, ui.spinExposure->value());
		//设置增益
		CameraSetAnalogGain(mindvision_haldle, ui.spinGain->value());
		//设置伽马
		CameraSetGamma(mindvision_haldle, ui.spinGamma->value() * 100);
		//设置对比度
		CameraSetContrast(mindvision_haldle, ui.spinContrast->value());
		//设置触发模式
		int iModeSel = 0;
		if (ui.comboTriggerMode->currentIndex() == 0)
		{
			iModeSel = 0;
		}
		else if (ui.comboTriggerMode->currentIndex() == 1)
		{
			iModeSel = 1;
		}
		else if (ui.comboTriggerMode->currentIndex() == 2)
		{
			iModeSel = 2;
		}
		CameraSetTriggerMode(mindvision_haldle, iModeSel);
		//保存当前相机参数到指定的参数组中
		CameraSaveParameter(mindvision_haldle, 0);
		if (sCameraInfo.sIspCapacity.bMonoSensor)
		{
			CameraSetIspOutFormat(mindvision_haldle, CAMERA_MEDIA_TYPE_MONO8);
		}
		CameraPlay(mindvision_haldle);
		gVariable::CameraVar.camera_type = "MindVision";
		gVariable::CameraVar.mindvision_haldle_value = mindvision_haldle;
		gVariable::CameraVar.mindvision_framebuffer_value = mindvision_framebuffer;
		gVariable::CameraVar.time_out = ui.spinTimeOut->value();
		gVariable::camera_variable_link.insert(ui.lblType->text(), gVariable::CameraVar);
		dataVar::camera_state.append(ui.lblType->text());
		emit dataVar::fProItemTab->sig_InfoClick();
		emit dataVar::fProItemTab->sig_Log(ui.lblType->text() + "相机连接成功！");
		QMessageBox msgBox(QMessageBox::Icon::NoIcon, "提示", ui.lblType->text() + "相机连接成功！");
		msgBox.setWindowIcon(QIcon(":/res/ico/info.png"));
		msgBox.exec();
	}
	else if (strs[0] == "HIKVision")
	{
		//海康威视

	}
	//更新获取图像工具
	QList<int> link_keys = dataVar::all_link_process.uniqueKeys();
	for (int i = 0; i < link_keys.length(); i++)
	{
		int key = link_keys[i];
		for (int j = 0; j < 20; j++)
		{
			if (j == key)
			{
				QConfig::ToolBase[key]->RunCommunicationLink("获取图像");
			}
		}
	}
}

//初始化相机连接
void frmCameraSetUp::InitCameraConnect(QString camera_key_value)
{	
	QStringList strs = camera_key_value.split("_");
	if (strs[0] == "MindVision")
	{		
		tSdkCameraDevInfo sCameraList_buf[9];
		iCameraNums = 9;
		//返回的相机信息列表，会根据acFriendlyName排序
		if (CameraEnumerateDevice(sCameraList_buf, &iCameraNums) != CAMERA_STATUS_SUCCESS || iCameraNums == 0)
		{
			emit dataVar::fProItemTab->sig_ErrorClick();
			emit dataVar::fProItemTab->sig_Log("未搜索到迈德威视相机！");			
			dataVar::camera_state.clear();
			return;
		}
		int ccd_Index = global_camera_content.value(camera_key_value).ccd_index;
		bool cam_state = false;
		sCameraList.clear();
		for (int i = 0; i < iCameraNums; i++)
		{
			sCameraList.append(sCameraList_buf[i]);
			ui.comboCameraName->addItem(sCameraList_buf[i].acFriendlyName);
			if (camera_key_value.mid(11, camera_key_value.length() - 1) == sCameraList_buf[i].acFriendlyName)
			{
				if (i == ccd_Index)
				{
					cam_state = true;
				}
			}
		}
		if (cam_state == false)
		{
			emit dataVar::fProItemTab->sig_ErrorClick();
			emit dataVar::fProItemTab->sig_Log(camera_key_value + "相机索引错误！");
			dataVar::camera_state.removeOne(camera_key_value);
			return;
		}
		//初始化相机，(-1,-1)表示加载上次退出前保存的参数，如果是第一次使用该相机，则加载默认参数
		if ((status = CameraInit(&sCameraList[ccd_Index], -1, -1, &mindvision_haldle)) != CAMERA_STATUS_SUCCESS)
		{
			emit dataVar::fProItemTab->sig_ErrorClick();
			emit dataVar::fProItemTab->sig_Log("初始化" + camera_key_value + "相机失败！");
			dataVar::camera_state.removeOne(camera_key_value);
			return;
		}
		//获得该相机的特性描述
		CameraGetCapability(mindvision_haldle, &sCameraInfo);
		mindvision_framebuffer = (BYTE*)CameraAlignMalloc(sCameraInfo.sResolutionRange.iWidthMax * sCameraInfo.sResolutionRange.iWidthMax * 3, 16);
		//设置曝光
		CameraSetAeState(mindvision_haldle, false);
		CameraSetExposureTime(mindvision_haldle, global_camera_content.value(camera_key_value).global_exposure);
		//设置增益
		CameraSetAnalogGain(mindvision_haldle, global_camera_content.value(camera_key_value).global_gain);
		//设置伽马
		CameraSetGamma(mindvision_haldle, global_camera_content.value(camera_key_value).global_gamma * 100);
		//设置对比度
		CameraSetContrast(mindvision_haldle, global_camera_content.value(camera_key_value).global_contrast);
		//设置触发模式
		int iModeSel = 0;
		if (global_camera_content.value(camera_key_value).global_trigger_mode == "连续采集")
		{
			iModeSel = 0;
		}
		else if (global_camera_content.value(camera_key_value).global_trigger_mode == "软件触发")
		{
			iModeSel = 1;
		}
		else if (global_camera_content.value(camera_key_value).global_trigger_mode == "硬件触发")
		{
			iModeSel = 2;
		}
		CameraSetTriggerMode(mindvision_haldle, iModeSel);
		//保存当前相机参数到指定的参数组中
		CameraSaveParameter(mindvision_haldle, 0);
		if (sCameraInfo.sIspCapacity.bMonoSensor)
		{
			CameraSetIspOutFormat(mindvision_haldle, CAMERA_MEDIA_TYPE_MONO8);
		}
		CameraPlay(mindvision_haldle);
		gVariable::CameraVar.camera_type = "MindVision";
		gVariable::CameraVar.mindvision_haldle_value = mindvision_haldle;
		gVariable::CameraVar.mindvision_framebuffer_value = mindvision_framebuffer;		
		gVariable::camera_variable_link.insert(camera_key_value, gVariable::CameraVar);
		dataVar::camera_state.append(camera_key_value);
		emit dataVar::fProItemTab->sig_InfoClick();
		emit dataVar::fProItemTab->sig_Log(camera_key_value + "相机连接成功！");
	}
	//更新获取图像工具
	QList<int> link_keys = dataVar::all_link_process.uniqueKeys();
	for (int i = 0; i < link_keys.length(); i++)
	{
		int key = link_keys[i];
		for (int j = 0; j < 20; j++)
		{
			if (j == key)
			{
				QConfig::ToolBase[key]->RunCommunicationLink("获取图像");
			}
		}
	}
}

void frmCameraSetUp::on_btnDisconnect_clicked()
{
	//断开相机
	if (ui.lblType->text() == QString())
	{
		emit dataVar::fProItemTab->sig_ErrorClick();
		emit dataVar::fProItemTab->sig_Log("选择相机名称为空！");
		QMessageBox msgBox(QMessageBox::Icon::NoIcon, "错误", "选择相机名称为空！");
		msgBox.setWindowIcon(QIcon(":/res/ico/error.png"));
		msgBox.exec();
		return;
	}
	QStringList strs = ui.lblType->text().split("_");
	if (strs[0] == "MindVision")
	{
		//迈德威视		
		camera_keys.clear();
		camera_keys = gVariable::camera_variable_link.uniqueKeys();
		int count = 0;
		for (int k = 0; k < camera_keys.length(); k++)
		{
			QString key = camera_keys[k];
			if (key == ui.lblType->text())
			{
				if (gVariable::camera_variable_link.value(key).mindvision_haldle_value > 0)
				{
					CameraStop(gVariable::camera_variable_link.value(key).mindvision_haldle_value);
					//相机反初始化，释放资源
					CameraUnInit(gVariable::camera_variable_link.value(key).mindvision_haldle_value);
					CameraAlignFree(gVariable::camera_variable_link.value(key).mindvision_framebuffer_value);
					dataVar::camera_state.removeOne(ui.lblType->text());
					gVariable::camera_variable_link.remove(key);
					count = 1;
					emit dataVar::fProItemTab->sig_InfoClick();
					emit dataVar::fProItemTab->sig_Log(ui.lblType->text() + "已断开连接！");
					QMessageBox msgBox(QMessageBox::Icon::NoIcon, "提示", "该相机已断开连接！");
					msgBox.setWindowIcon(QIcon(":/res/ico/info.png"));
					msgBox.exec();
				}
			}
		}
		if (count == 0)
		{
			emit dataVar::fProItemTab->sig_ErrorClick();
			emit dataVar::fProItemTab->sig_Log("该相机未连接！");
			QMessageBox msgBox(QMessageBox::Icon::NoIcon, "错误", "该相机未连接！");
			msgBox.setWindowIcon(QIcon(":/res/ico/error.png"));
			msgBox.exec();
			return;
		}
	}
	else if (strs[0] == "HIKVision")
	{
		//海康威视

	}
	//更新获取图像工具
	QList<int> link_keys = dataVar::all_link_process.uniqueKeys();
	for (int i = 0; i < link_keys.length(); i++)
	{
		int key = link_keys[i];
		for (int j = 0; j < 20; j++)
		{
			if (j == key)
			{
				QConfig::ToolBase[key]->RunCommunicationLink("获取图像");
			}
		}
	}
}

void frmCameraSetUp::on_btnSave_clicked()
{
	//保存参数
	if (ui.lblType->text() == QString())
	{
		emit dataVar::fProItemTab->sig_ErrorClick();
		emit dataVar::fProItemTab->sig_Log("选择相机名称为空！");
		QMessageBox msgBox(QMessageBox::Icon::NoIcon, "错误", "选择相机名称为空！");
		msgBox.setWindowIcon(QIcon(":/res/ico/error.png"));
		msgBox.exec();
		return;
	}
	CameraContent.global_exposure = ui.spinExposure->value();
	CameraContent.global_gain = ui.spinGain->value();
	CameraContent.global_gamma = ui.spinGamma->value();
	CameraContent.global_contrast = ui.spinContrast->value();
	CameraContent.global_trigger_mode = ui.comboTriggerMode->currentText();
	CameraContent.global_timeout = ui.spinTimeOut->value();
	CameraContent.global_remark = ui.txtRemark->text();
	global_camera_content.insert(ui.lblType->text(), CameraContent);
	emit dataVar::fProItemTab->sig_InfoClick();
	emit dataVar::fProItemTab->sig_Log(ui.lblType->text() + "保存参数完成！");
	QMessageBox msgBox(QMessageBox::Icon::NoIcon, "提示", "保存参数完成！");
	msgBox.setWindowIcon(QIcon(":/res/ico/info.png"));
	msgBox.exec();
}

void frmCameraSetUp::slot_DoubleClicked(int row, int column)
{
	QString selectInstrument = ui.tableWidget->item(row, 0)->text();
	ui.lblType->setText(selectInstrument);
	camera_keys.reserve(100);
	camera_keys.clear();
	camera_keys = global_camera_content.uniqueKeys();
	int count = 0;
	for (int k = 0; k < camera_keys.length(); k++)
	{
		QString key = camera_keys[k];
		if (key == selectInstrument)
		{
			count = 1;
			ui.spinExposure->setValue(global_camera_content.value(key).global_exposure);
			ui.spinGain->setValue(global_camera_content.value(key).global_gain);
			ui.spinGamma->setValue(global_camera_content.value(key).global_gamma);
			ui.spinContrast->setValue(global_camera_content.value(key).global_contrast);
			ui.comboTriggerMode->setCurrentText(global_camera_content.value(key).global_trigger_mode);
			ui.spinTimeOut->setValue(global_camera_content.value(key).global_timeout);
			ui.txtRemark->setText(global_camera_content.value(key).global_remark);
		}
	}
	if (count == 0)
	{
		emit dataVar::fProItemTab->sig_WarnClick();
		emit dataVar::fProItemTab->sig_Log("该相机参数未保存！");
		QMessageBox msgBox(QMessageBox::Icon::NoIcon, "警告", "该相机参数未保存！");
		msgBox.setWindowIcon(QIcon(":/res/ico/warn.png"));
		msgBox.exec();
	}
}

void frmCameraSetUp::slot_MoveUp()
{
	int nRow = ui.tableWidget->currentRow();
	moveRow(ui.tableWidget, nRow, nRow - 1);
}

void frmCameraSetUp::slot_MoveDown()
{
	int nRow = ui.tableWidget->currentRow();
	moveRow(ui.tableWidget, nRow, nRow + 2);
}

void frmCameraSetUp::slot_DeleteName()
{
	int rowIndex = ui.tableWidget->currentRow();
	if (rowIndex != -1)
	{
		QString str = ui.tableWidget->item(rowIndex, 0)->text();
		global_camera_content.remove(str);
		gVariable::camera_variable_link.remove(str);
		ui.tableWidget->removeRow(rowIndex);
	}
	int count = ui.tableWidget->rowCount();
	if (count == 0)
	{
		ui.lblType->clear();
		global_camera_content.clear();
		gVariable::camera_variable_link.clear();
	}
	//更新获取图像工具
	QList<int> link_keys = dataVar::all_link_process.uniqueKeys();
	for (int i = 0; i < link_keys.length(); i++)
	{
		int key = link_keys[i];
		for (int j = 0; j < 20; j++)
		{
			if (j == key)
			{
				QConfig::ToolBase[key]->RunCommunicationLink("获取图像");
			}
		}
	}
}

void frmCameraSetUp::moveRow(QTableWidget* pTable, int nFrom, int nTo)
{
	if (pTable == NULL) return;
	pTable->setFocus();
	if (nFrom == nTo) return;
	if (nFrom < 0 || nTo < 0) return;
	int nRowCount = pTable->rowCount();
	if (nFrom >= nRowCount || nTo > nRowCount) return;
	if (nTo < nFrom) nFrom++; pTable->insertRow(nTo);
	int nCol = pTable->columnCount();
	for (int i = 0; i < nCol; i++)
	{
		pTable->setItem(nTo, i, pTable->takeItem(nFrom, i));
	}
	if (nFrom < nTo)
	{
		nTo--;
	}
	pTable->removeRow(nFrom); pTable->selectRow(nTo);
}
