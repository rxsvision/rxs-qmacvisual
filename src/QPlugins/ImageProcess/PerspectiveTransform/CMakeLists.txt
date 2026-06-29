# PerspectiveTransform plugin
if(NOT Qt5_FOUND)
    return()
endif()

rxs_add_qmacvisual_plugin(PerspectiveTransform
    QT_MODULES  Core Gui Widgets
    LIB_DEFINE  PERSPECTIVETRANSFORM_LIB
)
