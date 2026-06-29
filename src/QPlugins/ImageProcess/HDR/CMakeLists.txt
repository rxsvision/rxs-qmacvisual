# HDR plugin
if(NOT Qt5_FOUND)
    return()
endif()

rxs_add_qmacvisual_plugin(HDR
    QT_MODULES  Core Gui Widgets
    LIB_DEFINE  FOCUSFUSION_D_LIB
)
