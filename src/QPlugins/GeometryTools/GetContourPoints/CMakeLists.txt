# GetContourPoints plugin
if(NOT Qt5_FOUND)
    return()
endif()

rxs_add_qmacvisual_plugin(GetContourPoints
    QT_MODULES  Core Gui Widgets
    LIB_DEFINE  GETCONTOURPOINTS_LIB
)
