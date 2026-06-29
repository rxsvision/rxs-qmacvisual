# QRcodeIdentify plugin
if(NOT Qt5_FOUND)
    return()
endif()

rxs_add_qmacvisual_plugin(QRcodeIdentify
    QT_MODULES  Core Gui Widgets
    LIB_DEFINE  QRCODEIDENTIFY_LIB
)
