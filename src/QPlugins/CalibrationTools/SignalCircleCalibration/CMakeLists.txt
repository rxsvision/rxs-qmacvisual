# SignalCircleCalibration plugin
if(NOT Qt5_FOUND)
    return()
endif()

rxs_add_qmacvisual_plugin(SignalCircleCalibration
    QT_MODULES  Core Gui Widgets
    LIB_DEFINE  SIGNALCIRCLECALIBRATION_LIB
)
