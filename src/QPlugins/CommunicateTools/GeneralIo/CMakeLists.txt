# GeneralIo plugin
if(NOT Qt5_FOUND)
    return()
endif()

rxs_add_qmacvisual_plugin(GeneralIo
    QT_MODULES  Core Network Gui Widgets
    LIB_DEFINE  GENERALIO_LIB
)
