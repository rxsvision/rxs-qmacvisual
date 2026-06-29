# ExportCsv plugin
if(NOT Qt5_FOUND)
    return()
endif()

rxs_add_qmacvisual_plugin(ExportCsv
    QT_MODULES  Core Gui Widgets
    LIB_DEFINE  EXPORTCSV_LIB
)
