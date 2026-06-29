# BlobDetector plugin
if(NOT Qt5_FOUND)
    return()
endif()

rxs_add_qmacvisual_plugin(BlobDetector
    QT_MODULES  Core Gui Widgets
    LIB_DEFINE  BLOBDETECTOR_LIB
)
