#include "frmFocusFusion.h"
#include <QMenu>
#include <QContextMenuEvent>
#include<czxTool2D.h>


frmFocusFusion::frmFocusFusion(QString toolName, QToolBase* toolBase, QWidget* parent)
	: Toolnterface(toolName, toolBase, parent)
{
	ui.setupUi(this);

	view = new QGraphicsViews;
	ui.imageLayout->addWidget(view);

	connect(this, &frmFocusFusion::clickTab, this, &frmFocusFusion::onTabClicked);
	QTabBar* tabBar = ui.img_parameters->tabBar();
	tabBar->installEventFilter(this);  //安装事件过滤器
	rectangle_item = new RectangleItem(10, 10, 200, 200);
	rectangle_item->rect_init_state = true;
	view->AddItems(rectangle_item);
}

frmFocusFusion::~frmFocusFusion()
{
	view->deleteLater();
	rectangle_item->deleteLater();
	this->deleteLater();
}

int frmFocusFusion::ExecuteLink(const int int_link, const QString str_link, const QMap<QString, gVariable::Global_Var> variable_link)
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
		else if (int_link == 2)
		{
			FocusFusionParameter* sub_widget = dynamic_cast<FocusFusionParameter*>(ui.img_parameters->currentWidget());
			sub_widget->SubExecuteLink(int_link, str_link, variable_link);
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

int frmFocusFusion::InitSetToolData(const QVariant data)
{
	try
	{
		frmFocusFusionData init_data;
		init_data = data.value<frmFocusFusionData>();

		for (int i = 0; i < init_data.img_links.size(); i++)
		{
			FocusFusionParameter * ip = new FocusFusionParameter("sub", GetToolBase(), this);
			ip->ui.txtLinkImage->setObjectName(QString("txtLinkImage") + QString::number(i));
			connect(ip, &FocusFusionParameter::linkClicked, this, &frmFocusFusion::on_subBtnLinkImage_clicked);
			ui.img_parameters->insertTab(i, ip, QString::number(i));
		}
		rectangle_item->deleteLater();
		rectangle_item = new RectangleItem(init_data.x, init_data.y, init_data.width, init_data.height);
		rectangle_item->rect_init_state = true;
		view->AddItems(rectangle_item);
		return 0;
	}
	catch (...)
	{
		return -1;
	}
}

QVariant frmFocusFusion::InitGetToolData()
{
	frmFocusFusionData data;
	for (int i = 0; i < ui.img_parameters->count() - 1; i++)
	{
		FocusFusionParameter* sub_widget = dynamic_cast<FocusFusionParameter*>(ui.img_parameters->widget(i));
		data.img_links.push_back(sub_widget->ui.txtLinkImage->text());
	}
	MRectangle MRect;
	rectangle_item->GetRect(MRect);
	data.x = MRect.col;
	data.y = MRect.row;
	data.width = MRect.width;
	data.height = MRect.height;
	QVariant ret = QVariant::fromValue(data);
	return ret;
}

int frmFocusFusion::Execute(const QString toolname)
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

int frmFocusFusion::RunToolPro()
{
	try
	{
		vector<cv::Mat> imgs;
		for (int i = 0; i < ui.img_parameters->count() - 1; i++)
		{
			FocusFusionParameter* ip = dynamic_cast<FocusFusionParameter*>(ui.img_parameters->widget(i));
			if (ip->strs.size() != 2 || ip->strs[1] != "图像")
			{
				GetToolBase()->m_Tools[tool_index].PublicResult.State = false;
				return -1;
			}
			for (int j = 0; j < GetToolBase()->m_Tools.size(); j++)
			{
				if (GetToolBase()->m_Tools[j].PublicToolName == ip->strs[0])
				{
					//获取的图像在工具数组中的索引
					auto srcImage = GetToolBase()->m_Tools[j].PublicImage.OutputImage;
					cv::Mat dstImage = cv::Mat();
					if (srcImage.type() == CV_8UC3)
						cv::cvtColor(srcImage(roi), dstImage, cv::COLOR_BGR2GRAY);
					else if (srcImage.type() == CV_8U)
						srcImage(roi).copyTo(dstImage);
					//if (srcImage.type() == CV_8UC3)
					//	cv::cvtColor(srcImage, dstImage, cv::COLOR_BGR2GRAY);
					//else if (srcImage.type() == CV_8U)
					//	srcImage.copyTo(dstImage);
					imgs.push_back(dstImage);
				}
			}
		}
		
		if (imgs.size() == 0)
		{
			GetToolBase()->m_Tools[tool_index].PublicResult.State = false;
			return -1;
		}

		if (dstImage.type() == CV_8UC3)
			cv::cvtColor(dstImage, dstImage, cv::COLOR_BGR2GRAY);
		auto img = fuseImages(imgs);
		img.copyTo(dstImage(roi));
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

bool frmFocusFusion::eventFilter(QObject* obj, QEvent* event)
{
	if (obj == ui.img_parameters->tabBar() && event->type() == QEvent::MouseButtonPress) {
		QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
		if (mouseEvent->button() == Qt::LeftButton)
		{
			// 获取点击的标签页索引
			int index = ui.img_parameters->tabBar()->tabAt(mouseEvent->pos());
			if (index != -1)
			{
				emit clickTab(index);
			}
		}
		else if (mouseEvent->button() == Qt::RightButton)
		{
			QMenu menu(this);

			// 在菜单中添加选项
			QAction* action1 = menu.addAction("删除参数页");
			//QAction* action2 = menu.addAction("Show Message");

			//// 处理菜单项的点击事件
			connect(action1, &QAction::triggered, this, &frmFocusFusion::onTabClose);
			//connect(action2, &QAction::triggered, this, &MyWidget::onShowMessage);
			menu.exec(mouseEvent->globalPos());
		}
	}
	return QWidget::eventFilter(obj, event);
}

void frmFocusFusion::onTabClicked(int index)
{
	if (index >= 0)
	{
		//QString tabName = ui.im->tabText(index);
		//QMessageBox::information(this, "Tab Clicked", "You clicked: " + tabName);
		QString tab_name = ui.img_parameters->tabText(index);
		if (tab_name == "+")
		{
			int new_index = ui.img_parameters->count() - 1;  // 获取倒数第三个标签页的索引
			if (new_index >= 0) {  // 确保索引有效
				FocusFusionParameter* ip = new FocusFusionParameter("sub", GetToolBase(), this);
				connect(ip, &FocusFusionParameter::linkClicked, this, &frmFocusFusion::on_subBtnLinkImage_clicked);
				ip->ui.txtLinkImage->setObjectName(QString("txtLinkImage") + QString::number(new_index));
				ui.img_parameters->insertTab(new_index, ip, QString::number(new_index));
				ui.img_parameters->setCurrentIndex(new_index);
			}
		}
	}
}

void frmFocusFusion::on_btnAddROI_clicked()
{
	view->ClearObj();
	rectangle_item->deleteLater();
	rectangle_item = new RectangleItem(10, 10, 200, 200);
	rectangle_item->rect_init_state = true;
	view->AddItems(rectangle_item);
}

void frmFocusFusion::on_btnLinkFollow_clicked()
{
	QConfig::nFormState = 3;
}

void frmFocusFusion::on_btnLinkImage_clicked()
{
	QConfig::nFormState = 1;
}

void frmFocusFusion::on_btnExecute_clicked()
{
	ui.btnExecute->setEnabled(false);
	QApplication::processEvents();
	Execute(GetToolName());
	QImage img(Mat2QImage(dstImage));
	view->DispImage(img);
	ui.btnExecute->setEnabled(true);
}

void frmFocusFusion::onTabClose(bool)
{
	ui.img_parameters->removeTab(ui.img_parameters->currentIndex());
	for (int i = ui.img_parameters->currentIndex(); i < ui.img_parameters->count() - 1; ++i) {
		ui.img_parameters->setTabText(i, QString::number(i));  // 修改标签页的标签名称
		FocusFusionParameter* ip = dynamic_cast<FocusFusionParameter*>(ui.img_parameters->widget(i));
		ip->ui.txtLinkImage->setObjectName(QString("txtLinkImage") + QString::number(i));
	}
}

void frmFocusFusion::on_subBtnLinkImage_clicked()
{
	QConfig::nFormState = 2;
}

void frmFocusFusion::on_btnDeleteROI_clicked()
{
	rectangle_item->rect_init_state = false;
	view->ClearObj();
}

//全局变量控制
int QConfig::nFormState = 0;
