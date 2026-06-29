# ColorIdentify plugin
if(NOT Qt5_FOUND)
    return()
endif()

rxs_add_qmacvisual_plugin(ColorIdentify
    QT_MODULES  Core Gui Widgets
    LIB_DEFINE  COLORIDENTIFY_LIB
)
