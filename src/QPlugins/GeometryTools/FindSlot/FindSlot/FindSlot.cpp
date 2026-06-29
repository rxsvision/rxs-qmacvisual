#include "FindSlot.h"

FindSlot::FindSlot()
{
    //qRegisterMetaType<InitSCCData>("InitSCCData");
    //qRegisterMetaTypeStreamOperators<InitSCCData>("InitSCCData");
}

QString FindSlot::name()
{
    return "寻找SLOT";
}

QString FindSlot::information()
{
    return "几何工具";
}

QDialog* showDialog(QString toolName, QToolBase* toolBase)
{
    frmFindSlot* ss = new frmFindSlot(toolName, toolBase);
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