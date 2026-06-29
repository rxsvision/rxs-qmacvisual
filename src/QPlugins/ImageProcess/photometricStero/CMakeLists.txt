# photometricStero plugin
if(NOT Qt5_FOUND)
    return()
endif()

rxs_add_qmacvisual_plugin(photometricStero
    QT_MODULES  Core Gui Widgets
    LIB_DEFINE  PHOTOMETRICSTERO_LIB
)
