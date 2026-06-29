#include "HDR.h"

FocusFusion_D::FocusFusion_D()
{
    qRegisterMetaType<frmHDRData>("frmHDRData");
    qRegisterMetaTypeStreamOperators<frmHDRData>("frmHDRData");
}
QString FocusFusion_D::name()
{
    return "HDR";
}

QString FocusFusion_D::information()
{
    return "Í¼Ïñ´¦Àí";
}

QDialog* showDialog(QString toolName, QToolBase* toolBase)
{
    frmHDR* ss = new frmHDR(toolName, toolBase);
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