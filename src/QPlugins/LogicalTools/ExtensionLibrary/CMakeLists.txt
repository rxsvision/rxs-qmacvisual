# ExtensionLibrary plugin
if(NOT Qt5_FOUND)
    return()
endif()

rxs_add_qmacvisual_plugin(ExtensionLibrary
    QT_MODULES  Core Gui Widgets
    LIB_DEFINE  EXTENSIONLIBRARY_LIB
)
