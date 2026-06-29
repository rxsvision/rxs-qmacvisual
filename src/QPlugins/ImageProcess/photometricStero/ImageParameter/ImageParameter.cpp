#include "ImageParameter.h"
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QWidget>
#include <QContextMenuEvent>
#include <QRegExpValidator>

ImageParameter::ImageParameter(QString toolName, QToolBase* toolBase, QWidget* parent)
	: Toolnterface(toolName, toolBase, parent)
{
	ui.setupUi(this);

	QRegExpValidator* pRegVld = new QRegExpValidator(this);

	QRegExp regex("^[-]?\\d+(\\.\\d+)?$");  // 正则表达式：匹配整数或浮点数
	pRegVld->setRegExp(regex);
	ui.slant->setValidator(pRegVld);
	ui.tilt->setValidator(pRegVld);

	connect(ui.txtLinkImage, &QLineEdit::textChanged, this, &ImageParameter::onLinkChanged);

}

ImageParameter::~ImageParameter()
{}

void ImageParameter::contextMenuEvent(QContextMenuEvent* event)
{
    //QMenu menu(this);

    //// 在菜单中添加选项
    //QAction* action1 = menu.addAction("Close Tab");
    //QAction* action2 = menu.addAction("Show Message");

    //// 处理菜单项的点击事件
    //connect(action1, &QAction::triggered, this, &MyWidget::onCloseTab);
    //connect(action2, &QAction::triggered, this, &MyWidget::onShowMessage);

    // 显示右键菜单
    //menu.exec(event->globalPos());
}

int ImageParameter::SubExecuteLink(const int int_link, const QString str_link, const QMap<QString, gVariable::Global_Var> variable_link)
{
	try
	{
		gvariable.global_variable_link = variable_link;
		//if (int_link == 1)
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
					image_index = i;
					auto srcImage = GetToolBase()->m_Tools[image_index].PublicImage.OutputImage;
					dstImage = cv::Mat();
					srcImage.copyTo(dstImage);
					//QImage img(Mat2QImage(dstImage));
					//parent->view->DispImage(img);
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

ImageParameterData ImageParameter::getData()
{
	ImageParameterData data;
	data.img_link = ui.btnLinkImage->text();
	data.slant = ui.slant->text();
	data.tilt = ui.tilt->text();
	return data;
}

void ImageParameter::setData(ImageParameterData data)
{
	ui.btnLinkImage->setText(data.img_link);
	ui.slant->setText(data.slant);
	ui.tilt->setText(data.tilt);
}

void ImageParameter::onLinkChanged(QString text)
{
	strs.clear();
	strs = text.split(".");
}


void ImageParameter::on_btnLinkImage_clicked()
{
    //QConfig::nFormState = 1;
	emit linkClicked();
}
