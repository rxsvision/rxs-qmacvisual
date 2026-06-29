# FitCircle plugin
if(NOT Qt5_FOUND)
    return()
endif()

rxs_add_qmacvisual_plugin(FitCircle
    QT_MODULES  Core Gui Widgets
    LIB_DEFINE  FITCIRCLE_LIB
)
