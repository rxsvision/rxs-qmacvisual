# ImageClarity plugin
if(NOT Qt5_FOUND)
    return()
endif()

rxs_add_qmacvisual_plugin(ImageClarity
    QT_MODULES  Core Gui Widgets
    LIB_DEFINE  IMAGECLARITY_LIB
)
