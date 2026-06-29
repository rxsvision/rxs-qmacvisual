# ImageMorphology plugin
if(NOT Qt5_FOUND)
    return()
endif()

rxs_add_qmacvisual_plugin(ImageMorphology
    QT_MODULES  Core Gui Widgets
    LIB_DEFINE  IMAGEMORPHOLOGY_LIB
)
