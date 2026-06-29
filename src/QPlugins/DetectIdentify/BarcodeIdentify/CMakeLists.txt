# BarcodeIdentify plugin
if(NOT Qt5_FOUND)
    return()
endif()

rxs_add_qmacvisual_plugin(BarcodeIdentify
    QT_MODULES  Core Gui Widgets
    LIB_DEFINE  BARCODEIDENTIFY_LIB
)
