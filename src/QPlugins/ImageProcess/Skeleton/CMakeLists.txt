# Skeleton plugin
if(NOT Qt5_FOUND)
    return()
endif()

rxs_add_qmacvisual_plugin(Skeleton
    QT_MODULES  Core Gui Widgets
    LIB_DEFINE  SKELETON_LIB
)
