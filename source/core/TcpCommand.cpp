#include "TcpCommand.hpp"
#include <QDataStream>
#include <iostream>

TcpCommand::TcpCommand(QObject* parent)
    : QObject(parent)
    , m_msgRead(true)
{
    m_nrClients = 0;
    m_error = false;
    m_state = INIT;
    m_tcpServer = NULL;
    m_tcpSocket = NULL;
    mServerListenPort = 9999;
    mServerStreamPort = 10000;
    mServerPassword = "";
    mAuth = false;
    clearMessage();
}

TcpCommand::~TcpCommand()
{
}

QString TcpCommand::serverPassword()
{
    return mServerPassword;
}

void TcpCommand::setServerPassword(const QString& password)
{
    mServerPassword = password;
}

int TcpCommand::serverListenPort()
{
    return mServerListenPort;
}

void TcpCommand::setServerListenPort(int port)
{
    mServerListenPort = port;
}

int TcpCommand::serverStreamPort()
{
    return mServerStreamPort;
}

void TcpCommand::setServerStreamPort(int port)
{
    mServerStreamPort = port;
}

void TcpCommand::connectTo(const QString& ip, int port, int ms)
{
    disconnect();
    m_error = false;
    m_state = CLIENT;
    m_tcpSocket = new QTcpSocket(this);
    connect(m_tcpSocket, SIGNAL(connected()), this, SLOT(connected()));
    connect(m_tcpSocket, SIGNAL(disconnected()), this, SLOT(disconnected()));
    connect(m_tcpSocket, SIGNAL(readyRead()), this, SLOT(readData()));
    connect(m_tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(error(QAbstractSocket::SocketError)));
    m_tcpSocket->connectToHost(ip, port);
    QTimer::singleShot(ms, this, SLOT(checkConnectionStatus()));
}

bool TcpCommand::listen()
{
    disconnect();
    m_error = false;
    m_state = SERVER;
    m_tcpServer = new QTcpServer(this);
    connect(m_tcpServer, SIGNAL(newConnection()), this, SLOT(connected()));
    bool result = m_tcpServer->listen(QHostAddress::Any, mServerListenPort);
    return result;
}

void TcpCommand::sendMessage(QByteArray& block)
{
    m_tcpSocket->write(block);
    m_tcpSocket->flush();
}

QString TcpCommand::peerName() // rename
{
    if (m_tcpSocket)
        return m_tcpSocket->peerName();
    return "";
}

quint16 TcpCommand::getPort() // rename
{
    switch (m_state) {
    case CLIENT:
        return m_tcpSocket->peerPort();
    case SERVER:
        return m_tcpServer->serverPort();
    default:
        return 0;
    }
}

void TcpCommand::disconnect()
{
    if (m_tcpSocket) {
        m_tcpSocket->abort();
        //delete m_tcpSocket;
    }
    if (m_tcpServer) {
        m_tcpServer->close();
        //delete m_tcpServer;
    }
    m_state = INIT;
}

CommandState TcpCommand::getState()
{
    return m_state;
}

void TcpCommand::clearMessage()
{
    m_msg.msgType = -1;
    m_msg.blockSize = -1;
    m_msg.dataBlock.clear();
    m_msgRead = true;
}

bool TcpCommand::loadSettingsBinary(const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        std::cout << "Failed to load " << fileName.toStdString().c_str() << "\n";
        return false;
    }
    QDataStream in(&file);

    in.setVersion(QDataStream::Qt_4_6);
    in >> mServerListenPort;
    in >> mServerStreamPort;
    in >> mServerPassword;
    file.close();
    return true;
}

bool TcpCommand::saveSettingsBinary(const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly))
        return false;
    QDataStream out(&file);

    out.setVersion(QDataStream::Qt_4_6);
    out << mServerListenPort;
    out << mServerStreamPort;
    out << mServerPassword;
    file.close();
    return true;
}

void TcpCommand::connected()
{
    if (m_state == CLIENT) {
        emit tcpConnected();
    } else if (m_state == SERVER) {
        QTcpSocket* tmp = m_tcpServer->nextPendingConnection();
        if (m_nrClients < 1) {
            m_nrClients++;
            m_tcpSocket = tmp;
            connect(m_tcpSocket, SIGNAL(disconnected()), this, SLOT(disconnected()));
            connect(m_tcpSocket, SIGNAL(readyRead()), this, SLOT(readData()));
            std::cout << "Client connected\n";
            emit tcpConnected();
        } else {
            tmp->disconnectFromHost();
            std::cout << "Client dropped\n";
        }
    }
}

void TcpCommand::checkConnectionStatus()
{ // fix for default 30 sec timeout.
    if (!m_error)
        if (m_tcpSocket->state() != QAbstractSocket::ConnectedState) {
            disconnect();
            emit tcpError("Connection timed out");
        }
}

void TcpCommand::disconnected()
{
    std::cout << "Socket disconnected\n";
    if (m_nrClients > 0)
        m_nrClients--;
}

void TcpCommand::readData()
{
    if (!m_msgRead)
        return;

    QDataStream in(m_tcpSocket);
    in.setVersion(QDataStream::Qt_4_6);

    if (m_msg.msgType == -1) {
        if (m_tcpSocket->bytesAvailable() < (int)2 * sizeof(int))
            return;
        in >> m_msg.msgType;
        in >> m_msg.blockSize;
    }

    if (m_tcpSocket->bytesAvailable() < (int)m_msg.blockSize)
        return;

    m_msg.dataBlock = m_tcpSocket->read(m_msg.blockSize);

    if (m_state == SERVER) {
        if (!mAuth) {
            if (m_msg.msgType == REQ_HANDSHAKE) {
                QDataStream in(m_msg.dataBlock);
                in.setVersion(QDataStream::Qt_4_6);
                QString sentPass;
                in >> sentPass;
                if (sentPass.compare(mServerPassword) == 0)
                    mAuth = true;
                else {
                    std::cout << "password error\n";
                    m_tcpSocket->disconnectFromHost();
                }
            } else
                m_tcpSocket->disconnectFromHost();
            clearMessage();
        } else {
            m_msgRead = false;
            emit messageReceived(m_msg);
        }
    } else {
        m_msgRead = false;
        emit messageReceived(m_msg);
    }
}

void TcpCommand::error(QAbstractSocket::SocketError /*socketError*/)
{
    std::cout << "Socket error\n";
    m_error = true;
    emit tcpError(m_tcpSocket->errorString());
}
