# LogicJudgeEnd plugin
if(NOT Qt5_FOUND)
    return()
endif()

rxs_add_qmacvisual_plugin(LogicJudgeEnd
    QT_MODULES  Core Gui Widgets
    LIB_DEFINE  LOGICJUDGEEND_LIB
)
