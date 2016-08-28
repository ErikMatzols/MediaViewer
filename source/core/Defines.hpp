#ifndef DEFINES_HPP
#define DEFINES_HPP

#include <QByteArray>

//#define USE_DEBUG_HEAP
//#define NO_IMAGE_ALLOC
//#define NO_WEBVIEW_PLUGINS
//#define NO_WEBVIEW_LOAD
//#define WRITE_TO_CONSOLE

struct RemoteMsg {
    int msgType;
    int blockSize;
    QByteArray dataBlock;
};

enum PacketType { REQ_HANDSHAKE,
    REQ_ALBUMMODEL,
    REQ_MOVEIN,
    REQ_MOVEOUT,
    REQ_ALBUMIMAGE,
    REP_ALBUMMODEL,
    REP_ALBUMFILE,
    REP_ALBUMIMAGE };

enum ConnectionState { CONNECTION_LOCAL,
    CONNECTION_REMOTE };

#endif
