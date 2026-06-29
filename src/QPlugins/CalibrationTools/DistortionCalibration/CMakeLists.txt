# DistortionCalibration plugin
if(NOT Qt5_FOUND)
    return()
endif()

rxs_add_qmacvisual_plugin(DistortionCalibration
    QT_MODULES  Core Gui Widgets
    LIB_DEFINE  DISTORTIONCALIBRATION_LIB
)
