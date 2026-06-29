# PointPoint plugin
if(NOT Qt5_FOUND)
    return()
endif()

rxs_add_qmacvisual_plugin(PointPoint
    QT_MODULES  Core Gui Widgets
    LIB_DEFINE  POINTPOINT_LIB
)
