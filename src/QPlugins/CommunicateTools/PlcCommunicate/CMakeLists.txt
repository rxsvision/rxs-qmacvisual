# PlcCommunicate plugin
if(NOT Qt5_FOUND)
    return()
endif()

rxs_add_qmacvisual_plugin(PlcCommunicate
    QT_MODULES  Core Network Gui Widgets
    LIB_DEFINE  PLCCOMMUNICATE_LIB
)
