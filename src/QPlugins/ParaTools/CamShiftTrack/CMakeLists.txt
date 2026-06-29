# CamShiftTrack plugin
if(NOT Qt5_FOUND)
    return()
endif()

rxs_add_qmacvisual_plugin(CamShiftTrack
    QT_MODULES  Core Gui Widgets
    LIB_DEFINE  CAMSHIFTTRACK_LIB
)
