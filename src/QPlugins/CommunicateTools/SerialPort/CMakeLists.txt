# SerialPort plugin
if(NOT Qt5_FOUND)
    return()
endif()

rxs_add_qmacvisual_plugin(SerialPort
    QT_MODULES  Core Gui Widgets SerialPort
    LIB_DEFINE  SERIALPORT_LIB
)
