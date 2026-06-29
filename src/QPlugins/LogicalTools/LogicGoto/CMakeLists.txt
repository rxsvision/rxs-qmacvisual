# LogicGoto plugin
if(NOT Qt5_FOUND)
    return()
endif()

rxs_add_qmacvisual_plugin(LogicGoto
    QT_MODULES  Core Gui Widgets
    LIB_DEFINE  LOGICGOTO_LIB
)
