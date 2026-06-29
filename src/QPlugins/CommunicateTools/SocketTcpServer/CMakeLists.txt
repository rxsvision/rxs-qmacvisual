# SocketTcpServer plugin
if(NOT Qt5_FOUND)
    return()
endif()

rxs_add_qmacvisual_plugin(SocketTcpServer
    QT_MODULES  Core Network Gui Widgets
    LIB_DEFINE  SOCKETTCPSERVER_LIB
)
