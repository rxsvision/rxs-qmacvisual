#pragma once

#include "focusfusion_d_global.h"
#include "dependence/maininterface.h"
#include "dependence/QToolBase.h"
#include "qdialog.h"
#include"frmFocusFusion.h"


class FOCUSFUSION_D_EXPORT FocusFusion_D
    : public QObject
    , public MainInterface
{
    Q_OBJECT
    Q_INTERFACES(MainInterface)
    Q_PLUGIN_METADATA(IID MainInterface_iid)

public:
    FocusFusion_D();
    virtual QString name() override;
    virtual QString information() override;
};


extern "C" Q_DECL_EXPORT QDialog * showDialog(QString toolName, QToolBase * toolBase);

extern "C" Q_DECL_EXPORT int ShowFormState();

extern "C" Q_DECL_EXPORT void SetFormState();