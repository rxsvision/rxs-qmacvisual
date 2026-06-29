# FindLine plugin
if(NOT Qt5_FOUND)
    return()
endif()

rxs_add_qmacvisual_plugin(FindLine
    QT_MODULES  Core Gui Widgets
    LIB_DEFINE  FINDLINE_LIB
)
