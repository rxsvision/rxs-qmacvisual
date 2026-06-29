# LinearCalculation plugin
if(NOT Qt5_FOUND)
    return()
endif()

rxs_add_qmacvisual_plugin(LinearCalculation
    QT_MODULES  Core Gui Widgets
    LIB_DEFINE  LINEARCALCULATION_LIB
)
