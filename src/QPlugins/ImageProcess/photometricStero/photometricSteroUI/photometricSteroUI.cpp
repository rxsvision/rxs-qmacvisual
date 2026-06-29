#include "photometricSteroUI.h"

photometricStero::photometricStero()
{
    qRegisterMetaType<frmPhotometricSteroData>("frmPhotometricSteroData");
    qRegisterMetaTypeStreamOperators<frmPhotometricSteroData>("frmPhotometricSteroData");
}

QString photometricStero::name()
{
    return "光度立体法";
}

QString photometricStero::information()
{
    return "图像处理";
}

QDialog* showDialog(QString toolName, QToolBase* toolBase)
{
    frmPhotometricStero* ss = new frmPhotometricStero(toolName, toolBase);
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