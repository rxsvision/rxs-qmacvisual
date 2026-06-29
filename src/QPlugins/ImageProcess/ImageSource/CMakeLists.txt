# A_ImageSource plugin
if(NOT Qt5_FOUND)
    return()
endif()

rxs_add_qmacvisual_plugin(A_ImageSource
    QT_MODULES  Core Gui Widgets
    LIB_DEFINE  A_IMAGESOURCE_LIB
)
