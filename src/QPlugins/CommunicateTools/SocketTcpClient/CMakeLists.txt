# SocketTcpClient plugin
if(NOT Qt5_FOUND)
    return()
endif()

rxs_add_qmacvisual_plugin(SocketTcpClient
    QT_MODULES  Core Network Gui Widgets
    LIB_DEFINE  SOCKETTCPCLIENT_LIB
)
