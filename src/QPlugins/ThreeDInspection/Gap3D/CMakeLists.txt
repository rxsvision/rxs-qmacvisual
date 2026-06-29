# Gap3D plugin
if(NOT Qt5_FOUND)
    return()
endif()

rxs_add_qmacvisual_plugin(Gap3D
    QT_MODULES  Core Gui Widgets
    LIB_DEFINE  GAP3D_LIB
)
