# SelectShape plugin
if(NOT Qt5_FOUND)
    return()
endif()

rxs_add_qmacvisual_plugin(SelectShape
    QT_MODULES  Core Gui Widgets
    LIB_DEFINE  SELECTSHAPE_LIB
)
