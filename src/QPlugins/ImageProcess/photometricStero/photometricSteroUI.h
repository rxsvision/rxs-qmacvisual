#pragma once

#include "photometricstero_global.h"
#include "dependence/maininterface.h"
#include "dependence/QToolBase.h"
#include "qdialog.h"
#include"frmPhotometricStero.h"

class PHOTOMETRICSTERO_EXPORT photometricStero
    : public QObject
    , public MainInterface
{
    Q_OBJECT
    Q_INTERFACES(MainInterface)
    Q_PLUGIN_METADATA(IID MainInterface_iid)

public:
    photometricStero();
    virtual QString name() override;
    virtual QString information() override;
};


extern "C" Q_DECL_EXPORT QDialog * showDialog(QString toolName, QToolBase * toolBase);

extern "C" Q_DECL_EXPORT int ShowFormState();

extern "C" Q_DECL_EXPORT void SetFormState();