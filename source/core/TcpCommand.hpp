#ifndef TCPCOMMAND_HPP
#define TCPCOMMAND_HPP

#include "Defines.hpp"
#include <QFile>
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>

enum CommandState { INIT,
    CLIENT,
    SERVER };

class TcpCommand : public QObject {
    Q_OBJECT

public:
    TcpCommand(QObject* parent);
    ~TcpCommand();

    QString serverPassword();
    void setServerPassword(const QString& password);
    int serverListenPort();

    void setServerListenPort(int port);
    int serverStreamPort();

    void setServerStreamPort(int port);

    void connectTo(const QString& ip, int port, int ms);

    bool listen();
    void sendMessage(QByteArray& block);

    QString peerName(); // rename

    quint16 getPort(); // rename

    void disconnect();
    CommandState getState();
    void clearMessage();
    bool loadSettingsBinary(const QString& fileName);
    bool saveSettingsBinary(const QString& fileName);

signals:
    void tcpConnected();
    void tcpDisconnected();
    void tcpError(const QString& msg);
    void messageReceived(RemoteMsg& msg);

protected:
private slots:
    void connected();
    void checkConnectionStatus();
    void disconnected();
    void readData();
    void error(QAbstractSocket::SocketError socketError);

private:
    CommandState m_state;
    QTcpSocket* m_tcpSocket;
    QTcpServer* m_tcpServer;
    int m_nrClients;
    bool m_msgRead;
    bool m_error;
    RemoteMsg m_msg;
    int mServerListenPort;
    int mServerStreamPort;
    QString mServerPassword;
    bool mAuth;
};

#endif
