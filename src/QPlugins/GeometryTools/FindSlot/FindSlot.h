#pragma once
#pragma execution_character_set("utf-8")

#include "findslot_global.h"
#include "dependence/maininterface.h"
#include "dependence/QToolBase.h"
#include"qdialog.h"
#include"frmFindSlot.h"

class FINDSLOT_EXPORT FindSlot
    : public QObject
    , public MainInterface
{
    Q_OBJECT
    Q_INTERFACES(MainInterface)
    Q_PLUGIN_METADATA(IID MainInterface_iid)
public:

    FindSlot();
    virtual QString name() override;
    virtual QString information() override;
};

extern "C" Q_DECL_EXPORT QDialog * showDialog(QString toolName, QToolBase * toolBase);

extern "C" Q_DECL_EXPORT int ShowFormState();

extern "C" Q_DECL_EXPORT void SetFormState();