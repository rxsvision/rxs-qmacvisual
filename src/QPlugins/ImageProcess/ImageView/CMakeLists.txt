# ImageView plugin
if(NOT Qt5_FOUND)
    return()
endif()

rxs_add_qmacvisual_plugin(ImageView
    QT_MODULES  Core Gui Widgets
    LIB_DEFINE  IMAGEVIEW_LIB
)
