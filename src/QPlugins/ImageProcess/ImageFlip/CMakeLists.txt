# ImageFlip plugin
if(NOT Qt5_FOUND)
    return()
endif()

rxs_add_qmacvisual_plugin(ImageFlip
    QT_MODULES  Core Gui Widgets
    LIB_DEFINE  IMAGEFLIP_LIB
)
