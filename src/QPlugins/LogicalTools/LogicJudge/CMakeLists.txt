# LogicJudge plugin
if(NOT Qt5_FOUND)
    return()
endif()

rxs_add_qmacvisual_plugin(LogicJudge
    QT_MODULES  Core Gui Widgets
    LIB_DEFINE  LOGICJUDGE_LIB
)
