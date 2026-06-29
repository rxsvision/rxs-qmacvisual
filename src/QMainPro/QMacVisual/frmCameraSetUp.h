#pragma once

#include <QDialog>
#include "ui_frmCameraSetUp.h"
#include <QButtonGroup>
#include "mytitlebar.h"
#include "gvariable.h"

#ifdef _WIN64
#pragma comment(lib, "./CommTools/Camera/MindVision/MVCAMSDK_X64.lib")
#else
#pragma comment(lib, "./CommTools/Camera/MindVision/MVCAMSDK.lib")
#endif

class frmCameraSetUp : public QDialog
{
	Q_OBJECT

public:
	frmCameraSetUp(QWidget* parent = Q_NULLPTR);
	~frmCameraSetUp();

private:
	Ui::frmCameraSetUpClass ui;

private:
	void initTitleBar();
	void paintEvent(QPaintEvent* event);	
	void AddRightButtonMenu();   //右键菜单	
	void moveRow(QTableWidget* pTable, int nFrom, int nTo);

private slots:
	void onButtonCloseClicked();
	void on_btnAddCamera_clicked();
	void on_btnSearchCamera_clicked();
	void on_btnConnect_clicked();
	void on_btnDisconnect_clicked();	
	void on_btnSave_clicked();
	void slot_MoveUp();
	void slot_MoveDown();
	void slot_DeleteName();
	void slot_DoubleClicked(int, int);

public slots:
	void InitCameraConnect(QString camera_key_value);
	
private:		
	QList<QString> camera_keys;
	//迈德威视
	CameraHandle mindvision_haldle;
	BYTE* mindvision_framebuffer = nullptr;	
	int mindvision_camera_state;	
	QVector<tSdkCameraDevInfo> sCameraList;
	CameraSdkStatus status;
	tSdkCameraCapbility sCameraInfo;
	int iCameraNums;

public:	
	//相机
	typedef struct CAMERACONTENT
	{		
		int ccd_index;
		int global_exposure;
		double global_gain;
		double global_gamma;
		int global_contrast;
		QString global_trigger_mode;
		int global_timeout;
		QString global_remark;
	} Camera_Content;
	Camera_Content CameraContent;
	QMap<QString, Camera_Content> global_camera_content;
};
