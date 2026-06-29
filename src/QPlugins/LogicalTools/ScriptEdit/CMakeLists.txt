# ScriptEdit plugin
if(NOT Qt5_FOUND)
    return()
endif()

rxs_add_qmacvisual_plugin(ScriptEdit
    QT_MODULES  Core Gui Widgets Script
    LIB_DEFINE  SCRIPTEDIT_LIB
)
