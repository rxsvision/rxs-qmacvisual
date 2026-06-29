# FocusFusion_D plugin
if(NOT Qt5_FOUND)
    return()
endif()

rxs_add_qmacvisual_plugin(FocusFusion_D
    QT_MODULES  Core Gui Widgets
    LIB_DEFINE  FOCUSFUSION_D_LIB
)
