# FitLine plugin
if(NOT Qt5_FOUND)
    return()
endif()

rxs_add_qmacvisual_plugin(FitLine
    QT_MODULES  Core Gui Widgets
    LIB_DEFINE  FITLINE_LIB
)
