# CreateRoi plugin
if(NOT Qt5_FOUND)
    return()
endif()

rxs_add_qmacvisual_plugin(CreateRoi
    QT_MODULES  Core Gui Widgets
    LIB_DEFINE  CREATEROI_LIB
)
