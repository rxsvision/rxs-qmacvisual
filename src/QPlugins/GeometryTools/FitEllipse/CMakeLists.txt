# FitEllipse plugin
if(NOT Qt5_FOUND)
    return()
endif()

rxs_add_qmacvisual_plugin(FitEllipse
    QT_MODULES  Core Gui Widgets
    LIB_DEFINE  FITELLIPSE_LIB
)
