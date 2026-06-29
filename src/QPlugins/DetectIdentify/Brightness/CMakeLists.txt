# Brightness plugin
if(NOT Qt5_FOUND)
    return()
endif()

rxs_add_qmacvisual_plugin(Brightness
    QT_MODULES  Core Gui Widgets
    LIB_DEFINE  BRIGHTNESS_LIB
)
