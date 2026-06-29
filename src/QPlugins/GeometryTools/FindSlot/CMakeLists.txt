# FindSlot plugin
if(NOT Qt5_FOUND)
    return()
endif()

rxs_add_qmacvisual_plugin(FindSlot
    QT_MODULES  Core Gui Widgets
    LIB_DEFINE  FINDSLOT_LIB
)
