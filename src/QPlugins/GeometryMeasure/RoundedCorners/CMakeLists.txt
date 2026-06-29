# RoundedCorners plugin
if(NOT Qt5_FOUND)
    return()
endif()

rxs_add_qmacvisual_plugin(RoundedCorners
    QT_MODULES  Core Gui Widgets
    LIB_DEFINE  ROUNDEDCORNERS_LIB
)
