# ImageSplice plugin
if(NOT Qt5_FOUND)
    return()
endif()

rxs_add_qmacvisual_plugin(ImageSplice
    QT_MODULES  Core Gui Widgets
    LIB_DEFINE  IMAGESPLICE_LIB
)
