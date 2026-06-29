# ImageRepair plugin
if(NOT Qt5_FOUND)
    return()
endif()

rxs_add_qmacvisual_plugin(ImageRepair
    QT_MODULES  Core Gui Widgets
    LIB_DEFINE  IMAGEREPAIR_LIB
)
