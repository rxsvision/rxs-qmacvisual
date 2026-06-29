# ImageRotate plugin
if(NOT Qt5_FOUND)
    return()
endif()

rxs_add_qmacvisual_plugin(ImageRotate
    QT_MODULES  Core Gui Widgets
    LIB_DEFINE  IMAGEROTATE_LIB
)
