# ShapeMatch plugin
if(NOT Qt5_FOUND)
    return()
endif()

rxs_add_qmacvisual_plugin(ShapeMatch
    QT_MODULES  Core Gui Widgets
    LIB_DEFINE  SHAPEMATCH_LIB
)
