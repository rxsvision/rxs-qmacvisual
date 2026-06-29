#include "frmFlatness3D.h"

frmFlatness3D::frmFlatness3D(QString toolName, QToolBase* toolBase, QWidget* parent)
	: Toolnterface(toolName, toolBase, parent)
{
	ui.setupUi(this);

	view = new QGraphicsViews;
	ui.imageLayout->addWidget(view);

	rectangle_item = new RectangleItem(10, 10, 200, 200);
	rectangle_item->rect_init_state = true;
	view->AddItems(rectangle_item);
}

frmFlatness3D::~frmFlatness3D()
{
	view->deleteLater();
	rectangle_item->deleteLater();
	this->deleteLater();
}

int frmFlatness3D::ExecuteLink(const int int_link, const QString str_link, const QMap<QString, gVariable::Global_Var> variable_link)
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
			ui.txtLinkImage->setText(str_link);

			for (int i = 0; i < GetToolBase()->m_Tools.size(); i++)
			{
				if (GetToolBase()->m_Tools[i].PublicToolName == strs[0])
				{
					//获取的图像在工具数组中的索引
					int image_index = i;
					auto srcImage = GetToolBase()->m_Tools[image_index].PublicImage.OutputImage;
					dstImage = cv::Mat();
					srcImage.copyTo(dstImage);
					QImage img(Mat2QImage(dstImage));
					view->DispImage(img);
				}
			}
		}
		else if (int_link == 3)
		{
			auto strs = str_link.split(".");
			if (strs.size() == 1 || strs[1] != "匹配基准中心")
			{
				QMessageBox msgBox(QMessageBox::Icon::NoIcon, "错误", "输入的链接错误！");
				msgBox.setWindowIcon(QIcon(":/resource/error.png"));
				msgBox.exec();
				return -1;
			}
			ui.txtLinkFollow->setText(str_link);
		}
		return 0;
	}
	catch (...)
	{
		return -1;
	}
}

int frmFlatness3D::InitSetToolData(const QVariant data)
{
	try
	{
		frmFlatness3DData init_data;
		init_data = data.value<frmFlatness3DData>();
		
		rectangle_item->deleteLater();
		rectangle_item = new RectangleItem(init_data.x, init_data.y, init_data.width, init_data.height);
		rectangle_item->rect_init_state = true;
		view->AddItems(rectangle_item);
		ui.txtMsg->clear();
		return 0;
	}
	catch (...)
	{
		return -1;
	}
}

QVariant frmFlatness3D::InitGetToolData()
{
	frmFlatness3DData data;
	MRectangle MRect;
	rectangle_item->GetRect(MRect);
	data.x = MRect.col;
	data.y = MRect.row;
	data.width = MRect.width;
	data.height = MRect.height;
	QVariant ret = QVariant::fromValue(data);
	return ret;
}

int frmFlatness3D::Execute(const QString toolname)
{
	for (int i = 0; i < GetToolBase()->m_Tools.size(); i++)
	{
		if (GetToolBase()->m_Tools[i].PublicToolName == toolname)
		{
			tool_index = i;
		}
	}

	strs.clear();
	strs = ui.txtLinkImage->text().split(".");

	for (int i = 0; i < GetToolBase()->m_Tools.size(); i++)
	{
		if (GetToolBase()->m_Tools[i].PublicToolName == strs[0])
		{
			//获取的图像在工具数组中的索引
			auto srcImage = GetToolBase()->m_Tools[i].PublicImage.OutputImage;
			dstImage = cv::Mat();
			srcImage.copyTo(dstImage);
		}
	}

	{
		if (rectangle_item->rect_init_state)
		{
			MRectangle m_rectangle;
			rectangle_item->GetRect(m_rectangle);
			roi = cv::Rect(m_rectangle.col, m_rectangle.row, m_rectangle.width, m_rectangle.height);


			//位置跟随
			if (ui.checkUseFollow->isChecked() == true)
			{
				//strs.clear();
				auto strs = ui.txtLinkFollow->text().split(".");
				if (strs.size() == 1)
				{
					GetToolBase()->m_Tools[tool_index].PublicResult.State = false;
					return -1;
				}
				bool link_state = false;
				int f_index = 0;
				for (int i = 0; i < GetToolBase()->m_Tools.size(); i++)
				{
					if (GetToolBase()->m_Tools[i].PublicToolName == strs[0])
					{
						f_index = i;
						link_state = true;
					}
				}
				if (link_state == false)
				{
					GetToolBase()->m_Tools[tool_index].PublicResult.State = false;
					return -2;
				}
				if (strs[1] == "匹配基准中心")
				{
					if (GetToolBase()->m_Tools[f_index].PublicTPosition.Center.size() == 0)
					{
						GetToolBase()->m_Tools[tool_index].PublicResult.State = false;
						return -1;
					}
					cv::Point2f match_origin_point = GetToolBase()->m_Tools[f_index].PublicTPosition.DatumCenter;
					cv::Point2f match_current_point = GetToolBase()->m_Tools[f_index].PublicTPosition.Center[0];
					roi.x += match_current_point.x - match_origin_point.x;
					roi.y += match_current_point.y - match_origin_point.y;
				}
				else
				{
					GetToolBase()->m_Tools[tool_index].PublicResult.State = false;
					return -2;
				}
			}
		}
		else
		{
			roi = cv::Rect(0, 0, dstImage.cols, dstImage.rows);
		}
	}
	return RunToolPro();
}

VectorXf frmFlatness3D::planeFitLSM(CP cloud)
{
	MatrixXf xyz = cloud->getMatrixXfMap(3, 4, 0);
	VectorXf z = xyz.row(2);
	xyz.row(2).setOnes();
	Eigen::VectorXf coff = Eigen::JacobiSVD<Eigen::MatrixXf>(xyz.transpose(), Eigen::ComputeThinU | Eigen::ComputeThinV).solve(z);
	return coff;
}

float frmFlatness3D::Flatness(CP cloud, PointT& min_point, PointT& max_point)
{
	CzxTimer dsghfa(__func__);
	auto coff = planeFitLSM(cloud);
	Vector3f normal = Vector3f(coff[0], coff[1], -1).normalized();
	VectorXf z = normal.transpose() * cloud->getMatrixXfMap(3, 4, 0);
	int max_index = 0;
	float max_value = z.maxCoeff(&max_index);
	int min_index = 0;
	float min_value = z.minCoeff(&min_index);
	min_point = cloud->points[min_index];
	max_point = cloud->points[max_index];
	CP tmp(new CloudT);
	tmp->push_back(min_point);
	tmp->push_back(max_point);
	Tool::showComparison(cloud, tmp, 1, 5);
	return max_value - min_value;
}

int frmFlatness3D::RunToolPro()
{
	try
	{
		if (dstImage.type() == CV_8UC3)
			cv::cvtColor(dstImage, dstImage, cv::COLOR_BGR2GRAY);
		//auto img = fuseImages(imgs);
		//img.copyTo(dstImage(roi));

		CP cloud = img2pcl(dstImage(roi), ui.txtXScale->text().toFloat(), ui.txtYScale->text().toFloat()
			, ui.txtZScale->text().toFloat(), ui.txtZInvalid->text().toFloat());
		pcl::io::savePCDFileBinary("test.pcd", *cloud);
		PointT min_point, max_point;
		float flat = Flatness(cloud, min_point, max_point);
		ui.txtMsg->append(QString("平面度是：")+QString::number(flat));
		QString max_str = QString("%1 的坐标是 (%2, %3, %4)")
			.arg("最高点")
			.arg(max_point.x, 0, 'f', 2) // 保留两位小数
			.arg(max_point.y, 0, 'f', 2)
			.arg(max_point.z, 0, 'f', 2);
		ui.txtMsg->append(max_str);
		QString min_str = QString("%1 的坐标是 (%2, %3, %4)")
			.arg("最低点")
			.arg(min_point.x, 0, 'f', 2) // 保留两位小数
			.arg(min_point.y, 0, 'f', 2)
			.arg(min_point.z, 0, 'f', 2);
		ui.txtMsg->append(min_str);
		GetToolBase()->m_Tools[tool_index].PublicImage.OutputImage = dstImage;
		GetToolBase()->m_Tools[tool_index].PublicResult.State = true;
		return 0;
	}
	catch (exception e)
	{
		GetToolBase()->m_Tools[tool_index].PublicResult.State = false;
		string mes = e.what();
		return -1;
	}
}

void frmFlatness3D::on_btnAddROI_clicked()
{
	view->ClearObj();
	rectangle_item->deleteLater();
	rectangle_item = new RectangleItem(10, 10, 200, 200);
	rectangle_item->rect_init_state = true;
	view->AddItems(rectangle_item);
}

void frmFlatness3D::on_btnLinkFollow_clicked()
{
	QConfig::nFormState = 3;
}

void frmFlatness3D::on_btnLinkImage_clicked()
{
	QConfig::nFormState = 1;
}

void frmFlatness3D::on_btnExecute_clicked()
{
	ui.btnExecute->setEnabled(false);
	QApplication::processEvents();
	Execute(GetToolName());
	QImage img(Mat2QImage(dstImage));
	view->DispImage(img);
	ui.btnExecute->setEnabled(true);
}

void frmFlatness3D::on_btnDeleteROI_clicked()
{
	rectangle_item->rect_init_state = false;
	view->ClearObj();
}

//全局变量控制
int QConfig::nFormState = 0;