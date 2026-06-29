# CropImage plugin
if(NOT Qt5_FOUND)
    return()
endif()

rxs_add_qmacvisual_plugin(CropImage
    QT_MODULES  Core Gui Widgets
    LIB_DEFINE  CROPIMAGE_LIB
)
