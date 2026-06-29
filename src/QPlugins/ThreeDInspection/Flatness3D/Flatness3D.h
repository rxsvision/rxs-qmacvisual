#pragma once

#include "flatness3d_global.h"
#include "dependence/maininterface.h"
#include "dependence/QToolBase.h"
#include "qdialog.h"
#include"frmFlatness3D.h"

class FLATNESS3D_EXPORT Flatness3D
    : public QObject
    , public MainInterface
{
    Q_OBJECT
    Q_INTERFACES(MainInterface)
    Q_PLUGIN_METADATA(IID MainInterface_iid)

public:
    Flatness3D();
    virtual QString name() override;
    virtual QString information() override;
};

extern "C" Q_DECL_EXPORT QDialog * showDialog(QString toolName, QToolBase * toolBase);

extern "C" Q_DECL_EXPORT int ShowFormState();

extern "C" Q_DECL_EXPORT void SetFormState();