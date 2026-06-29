# Z_EdgeWidthMeasure plugin
if(NOT Qt5_FOUND)
    return()
endif()

rxs_add_qmacvisual_plugin(Z_EdgeWidthMeasure
    QT_MODULES  Core Gui Widgets
    LIB_DEFINE  Z_EDGEWIDTHMEASURE_LIB
)
