# FindCircle plugin
if(NOT Qt5_FOUND)
    return()
endif()

rxs_add_qmacvisual_plugin(FindCircle
    QT_MODULES  Core Gui Widgets
    LIB_DEFINE  FINDCIRCLE_LIB
)
