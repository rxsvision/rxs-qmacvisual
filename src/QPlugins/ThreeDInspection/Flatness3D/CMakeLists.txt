# Flatness3D plugin
if(NOT Qt5_FOUND)
    return()
endif()

rxs_add_qmacvisual_plugin(Flatness3D
    QT_MODULES  Core Gui Widgets
    LIB_DEFINE  FLATNESS3D_LIB
)
