#include "Flatness3D.h"

Flatness3D::Flatness3D()
{
    qRegisterMetaType<frmFlatness3DData>("frmFlatness3DData");
    qRegisterMetaTypeStreamOperators<frmFlatness3DData>("frmFlatness3DData");
}
QString Flatness3D::name()
{
    return "平面度计算";
}

QString Flatness3D::information()
{
    return "三维检测";
}

QDialog* showDialog(QString toolName, QToolBase* toolBase)
{
    frmFlatness3D* ss = new frmFlatness3D(toolName, toolBase);
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