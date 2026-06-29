# ERTCalibration plugin
if(NOT Qt5_FOUND)
    return()
endif()

rxs_add_qmacvisual_plugin(ERTCalibration
    QT_MODULES  Core Gui Widgets
    LIB_DEFINE  ERTCALIBRATION_LIB
)
