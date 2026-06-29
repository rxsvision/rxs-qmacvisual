#include "FocusFusion_D.h"

FocusFusion_D::FocusFusion_D()
{
    qRegisterMetaType<frmFocusFusionData>("frmFocusFusionData");
    qRegisterMetaTypeStreamOperators<frmFocusFusionData>("frmFocusFusionData");
}
QString FocusFusion_D::name()
{
    return "多焦距合成";
}

QString FocusFusion_D::information()
{
    return "图像处理";
}

QDialog* showDialog(QString toolName, QToolBase* toolBase)
{
    frmFocusFusion* ss = new frmFocusFusion(toolName, toolBase);
    return ss;
}

int ShowFormState()
{
    return QConfig::nFormState;
}

void SetFormState()
{
    QConfig::nFormState = 0;
}