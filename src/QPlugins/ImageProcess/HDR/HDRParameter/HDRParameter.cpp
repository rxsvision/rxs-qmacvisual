#include "HDRParameter.h"

FocusFusionParameter::FocusFusionParameter(QString toolName, QToolBase* toolBase, QWidget* parent)
	: Toolnterface(toolName, toolBase, parent)
{
	ui.setupUi(this);

	connect(ui.txtLinkImage, &QLineEdit::textChanged, this, &FocusFusionParameter::onLinkChanged);

}

FocusFusionParameter::~FocusFusionParameter()
{}

void FocusFusionParameter::onLinkChanged(QString text)
{
	strs.clear();
	strs = text.split(".");
}


void FocusFusionParameter::on_btnLinkImage_clicked()
{
	//QConfig::nFormState = 1;
	emit linkClicked();
}

int FocusFusionParameter::SubExecuteLink(const int int_link, const QString str_link, const QMap<QString, gVariable::Global_Var> variable_link)
{
	try
	{
		gvariable.global_variable_link = variable_link;
		//if (int_link == 1)
		{
			strs.clear();
			strs = str_link.split(".");
			if (strs.size() == 1 || strs[1] != "Í¼Ïñ")
			{
				QMessageBox msgBox(QMessageBox::Icon::NoIcon, "´íÎó", "ÊäÈëµÄÁ´½Ó´íÎó£¡");
				msgBox.setWindowIcon(QIcon(":/resource/error.png"));
				msgBox.exec();
				return -1;
			}
			ui.txtLinkImage->setText(str_link);
		}
		return 0;
	}
	catch (...)
	{
		return -1;
	}
}
