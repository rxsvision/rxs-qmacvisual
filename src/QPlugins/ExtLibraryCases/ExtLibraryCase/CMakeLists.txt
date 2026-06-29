# ExtLibraryCase plugin
if(NOT Qt5_FOUND)
    return()
endif()

rxs_add_qmacvisual_plugin(ExtLibraryCase
    QT_MODULES  Core Gui Widgets
    LIB_DEFINE  EXTLIBRARYCASE_LIB
)
