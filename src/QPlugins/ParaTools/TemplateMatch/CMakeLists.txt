# TemplateMatch plugin
if(NOT Qt5_FOUND)
    return()
endif()

rxs_add_qmacvisual_plugin(TemplateMatch
    QT_MODULES  Core Gui Widgets
    LIB_DEFINE  TEMPLATEMATCH_LIB
)
