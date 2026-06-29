# Z_Classifier plugin
if(NOT Qt5_FOUND)
    return()
endif()

rxs_add_qmacvisual_plugin(Z_Classifier
    QT_MODULES  Core Gui Widgets
    LIB_DEFINE  Z_CLASSIFIER_LIB
)
