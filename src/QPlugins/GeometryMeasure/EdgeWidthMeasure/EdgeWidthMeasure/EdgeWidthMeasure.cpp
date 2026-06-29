#include "EdgeWidthMeasure.h"
#include "frmEdgeWidthMeasure.h"

EdgeWidthMeasure::EdgeWidthMeasure()
{
}

QString EdgeWidthMeasure::name()
{
    return "边缘宽度测量";
}

QString EdgeWidthMeasure::information()
{
    return "几何测量";
}

QDialog* showDialog(QString toolName, QToolBase* toolBase)
{
    frmEdgeWidthMeasure* ss = new frmEdgeWidthMeasure(toolName, toolBase);
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
