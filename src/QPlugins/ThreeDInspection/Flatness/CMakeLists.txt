# Flatness plugin
if(NOT Qt5_FOUND)
    return()
endif()

rxs_add_qmacvisual_plugin(Flatness
    QT_MODULES  Core Gui Widgets
    LIB_DEFINE  FLATNESS_LIB
)
