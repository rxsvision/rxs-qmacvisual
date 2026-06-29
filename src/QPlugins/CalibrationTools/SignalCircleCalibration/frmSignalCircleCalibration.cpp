#include "frmSingleCircleCalibration.h"
#include <QMessageBox>

frmSingleCircleCalibration::frmSingleCircleCalibration(QString toolName, QToolBase* toolBase, QWidget* parent)
	: Toolnterface(toolName, toolBase, parent)
{
	ui.setupUi(this);
	initConnect();
	view = new QGraphicsViews;
	ui.imageLayout->addWidget(view);
	concentric_circle_item = new ConcentricCircleItem(220, 220, 100, 200);

}

frmSingleCircleCalibration::~frmSingleCircleCalibration()
{
	view->deleteLater();
	concentric_circle_item->deleteLater();
	this->deleteLater();
}

void frmSingleCircleCalibration::initConnect()
{
	//connect(ui.btnLinkImage_2, SIGNAL(clicked()), this, SLOT(on_btnLinkImage_clicked()));
	//bool state = connect(ui.btnAddROI_2, SIGNAL(clicked()), this, SLOT(on_btnAddROI_clicked()));
}

void frmSingleCircleCalibration::on_btnLinkImage_clicked()
{
	QConfig::nFormState = 1;
}

void frmSingleCircleCalibration::on_btnAddROI_clicked()
{
	view->ClearObj();
	concentric_circle_item = new ConcentricCircleItem(220, 220, 100, 200);
	concentric_circle_item->concentric_circle_init_state = true;
	switch (ui.comboROIShape_2->currentIndex()) {
	case 0:
		concentric_circle_item->segment_line_num = ui.spinSegment->value();
		view->AddItems(concentric_circle_item);
		break;
	}
}

void frmSingleCircleCalibration::on_btnExecute_clicked()
{
	ui.btnExecute->setEnabled(false);
	QApplication::processEvents();
	//view_points = true;
	Execute(GetToolName());
	ui.txtMsg->clear();
	ui.txtMsg->append("-> 圆的中心坐标为：" + QString::number(fit_circle[0]) + "," + QString::number(fit_circle[1]));
	ui.txtMsg->append("-> 圆的半径为：" + QString::number(fit_circle[2]));
	QImage img(Mat2QImage(dstImage));
	view->DispImage(img);
	ui.btnExecute->setEnabled(true);
}

QImage frmSingleCircleCalibration::Mat2QImage(const cv::Mat& mat)
{
	if (mat.empty())
	{
		return QImage();
	}
	if (mat.type() == CV_8UC1)
	{
		QImage image(mat.cols, mat.rows, QImage::Format_Indexed8);
		image.setColorCount(256);
		for (int i = 0; i < 256; i++)
		{
			image.setColor(i, qRgb(i, i, i));
		}
		uchar* pSrc = mat.data;
		for (int row = 0; row < mat.rows; row++)
		{
			uchar* pDest = image.scanLine(row);
			memcpy(pDest, pSrc, mat.cols);
			pSrc += mat.step;
		}
		return image;
	}
	else if (mat.type() == CV_8UC3)
	{
		const uchar* pSrc = (const uchar*)mat.data;
		QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
		return image.rgbSwapped();
	}
	else if (mat.type() == CV_8UC4)
	{
		const uchar* pSrc = (const uchar*)mat.data;
		QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_ARGB32);
		return image.copy();
	}
	else
	{
		return QImage();
	}
}

int frmSingleCircleCalibration::Execute(const QString toolname)
{
	try
	{
		QString str = ui.txtLinkImage_2->text();
		strs.reserve(100);
		strs.clear();
		strs = str.split(".");
		if (strs.size() == 1)
		{
			GetToolBase()->m_Tools[tool_index].PublicResult.State = false;
			return -1;
		}
		for (int i = 0; i < GetToolBase()->m_Tools.size(); i++)
		{
			if (GetToolBase()->m_Tools[i].PublicToolName == strs[0])
			{
				//获取的图像在工具数组中的索引
				image_index = i;
				srcImage = GetToolBase()->m_Tools[image_index].PublicImage.OutputImage;
				dstImage = cv::Mat();
				srcImage.copyTo(dstImage);
				QImage img(Mat2QImage(dstImage));
				view->DispImage(img);
			}
			if (GetToolBase()->m_Tools[i].PublicToolName == toolname)
			{
				//工具在工具数组中的索引
				tool_index = i;
			}
		}

		if (concentric_circle_item == nullptr)
		{
			GetToolBase()->m_Tools[tool_index].PublicResult.State = false;
			return -1;
		}
		switch (ui.comboScanDirection->currentIndex()) {
		case 0:
			concentric_circle_item->in_out_circle = 0;
			break;
		case 1:
			concentric_circle_item->in_out_circle = 1;
			break;
		}
		switch (ui.comboGrayDirection->currentIndex()) {
		case 0:
			concentric_circle_item->direction = 0;
			break;
		case 1:
			concentric_circle_item->direction = 1;
			break;
		}

		concentric_circle_item->threshold_delta = ui.spinThreshold->value();

		for (int i = 0; i < concentric_circle_item->ring_small_points.size(); i++)
		{
			if (concentric_circle_item->ring_big_points[i].x() < 0 || concentric_circle_item->ring_small_points[i].x() < 0 || concentric_circle_item->ring_big_points[i].y() < 0 || concentric_circle_item->ring_small_points[i].y() < 0)
			{
				GetToolBase()->m_Tools[tool_index].PublicResult.State = false;
				return -1;
			}
			if (concentric_circle_item->ring_big_points[i].x() > dstImage.cols || concentric_circle_item->ring_small_points[i].x() > dstImage.cols || concentric_circle_item->ring_big_points[i].y() > dstImage.rows || concentric_circle_item->ring_small_points[i].y() > dstImage.rows)
			{
				GetToolBase()->m_Tools[tool_index].PublicResult.State = false;
				return -1;
			}
		}

	}
	catch (...)
	{
		GetToolBase()->m_Tools[tool_index].PublicResult.State = false;
		return -1;
	}

	RunToolPro();

	return 0;
}

int frmSingleCircleCalibration::RunToolPro()
{
	try
	{
		concentric_circle_item->FindBorderPoints(srcImage, xy);
		//拟合圆    
		vector<cv::Point2f> new_xy(xy.size());
		for (int i = 0; i < xy.size(); i++)
		{
			new_xy[i].x = xy[i].x();
			new_xy[i].y = xy[i].y();
		}
		fit_circle = concentric_circle_item->fitCircle(new_xy);

		select_xy.clear();
		cull_xy.clear();
		for (int i = 0; i < new_xy.size(); i++)
		{
			//计算两点之间的距离
			float distance;
			distance = powf((new_xy[i].x - fit_circle[0]), 2) + powf((new_xy[i].y - fit_circle[1]), 2);
			distance = sqrtf(distance);
			if (fit_circle[2] - distance > ui.spinCullDistance->value())
			{
				cull_xy.push_back(new_xy[i]);
			}
			else
			{
				select_xy.push_back(new_xy[i]);
			}
		}
		fit_circle = concentric_circle_item->fitCircle(select_xy);

		if (srcImage.channels() == 1)
		{
			cv::cvtColor(srcImage, dstImage, cv::COLOR_GRAY2BGR);
		}
		else if (srcImage.channels() == 4)
		{
			cv::cvtColor(srcImage, dstImage, cv::COLOR_RGBA2BGR);
		}
		else
		{
			cv::cvtColor(srcImage, dstImage, cv::COLOR_BGR2BGRA);
		}
		cv::circle(dstImage, cv::Point(fit_circle[0], fit_circle[1]), fit_circle[2], cv::Scalar(37, 193, 255), 2);
	}
	catch (...)
	{
		GetToolBase()->m_Tools[tool_index].PublicResult.State = false;
		return -1;
	}
	return 0;
}

int frmSingleCircleCalibration::ExecuteLink(const int int_link, const QString str_link, const QMap<QString, gVariable::Global_Var> variable_link)
{
	try
	{
		gvariable.global_variable_link = variable_link;
		if (int_link == 1)
		{
			strs.clear();
			strs = str_link.split(".");
			if (strs.size() == 1 || strs[1] != "图像")
			{
				QMessageBox msgBox(QMessageBox::Icon::NoIcon, "错误", "输入的链接错误！");
				msgBox.setWindowIcon(QIcon(":/resource/error.png"));
				msgBox.exec();
				return -1;
			}
			ui.txtLinkImage_2->setText(str_link);
			
			for (int i = 0; i < GetToolBase()->m_Tools.size(); i++)
			{
				if (GetToolBase()->m_Tools[i].PublicToolName == strs[0])
				{
					//获取的图像在工具数组中的索引
					image_index = i;
					srcImage = GetToolBase()->m_Tools[image_index].PublicImage.OutputImage;
					dstImage = cv::Mat();
					srcImage.copyTo(dstImage);
					QImage img(Mat2QImage(dstImage));
					view->DispImage(img);
				}
			}


		}
		return 0;
	}
	catch (...)
	{
		return -1;
	}
}

int frmSingleCircleCalibration::InitSetToolData(const QVariant data)
{
	try
	{
		InitSCCData init_data;
		init_data = data.value<InitSCCData>();
		view->ClearObj();
		concentric_circle_item = new ConcentricCircleItem(init_data.x, init_data.y, init_data.RadiusMin, init_data.RadiusMax);
		concentric_circle_item->concentric_circle_init_state = true;
		concentric_circle_item->segment_line_num = ui.spinSegment->value();
		view->AddItems(concentric_circle_item);
		concentric_circle_item->ring_small_points.clear();
		concentric_circle_item->ring_big_points.clear();
		for (int i = 0; i < init_data.ring_s_points.size(); i++)
		{
			concentric_circle_item->ring_small_points.push_back(init_data.ring_s_points[i]);
			concentric_circle_item->ring_big_points.push_back(init_data.ring_b_points[i]);
		}
		return 0;
	}
	catch (...)
	{
		return -1;
	}
	return 0;
}

QVariant frmSingleCircleCalibration::InitGetToolData()
{
	InitSCCData init_data;

	//ret.setValue(init_data);
	//QFile czx_data("test.czx");
	//if (!czx_data.open(QIODevice::WriteOnly)) {
	//}
	//QDataStream out(&czx_data);
	//ret.save(out);

	CCircle c_circle;
	concentric_circle_item->GetConcentricCircle(c_circle);
	init_data.x = c_circle.col;
	init_data.y = c_circle.row;
	init_data.RadiusMin = c_circle.small_radius;
	init_data.RadiusMax = c_circle.big_radius;

	QVariant ret = QVariant::fromValue(init_data);
	return ret;
	//return QVariant();
}

//全局变量控制
int QConfig::nFormState = 0;