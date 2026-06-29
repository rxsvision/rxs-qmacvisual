#include "SingleCircleCalibration.h"

SingleCircleCalibration::SingleCircleCalibration()
{
    qRegisterMetaType<InitSCCData>("InitSCCData");
    qRegisterMetaTypeStreamOperators<InitSCCData>("InitSCCData");
}

QString SingleCircleCalibration::name()
{
    return "单圆标定";
}

QString SingleCircleCalibration::information()
{
    return "标定工具";
}

QDialog* showDialog(QString toolName, QToolBase* toolBase)
{
    frmSingleCircleCalibration* ss = new frmSingleCircleCalibration(toolName, toolBase);
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