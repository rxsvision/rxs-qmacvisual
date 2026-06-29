# LineLineI plugin
if(NOT Qt5_FOUND)
    return()
endif()

rxs_add_qmacvisual_plugin(LineLineI
    QT_MODULES  Core Gui Widgets
    LIB_DEFINE  LINELINEI_LIB
)
