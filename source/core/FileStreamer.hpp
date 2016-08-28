#ifndef FILESTREAMER_HPP
#define FILESTREAMER_HPP

#include <QFile>
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include <iostream>

#define BUFFER_THRESHOLD 4194304 // 4096 KB
#define PAYLOAD_SIZE 524288 // 512 KB

class FileStreamer : public QObject {
    Q_OBJECT

public:
    FileStreamer(const QString& fileName, QObject* parent);
    ~FileStreamer();

    bool listen(const QHostAddress& address, quint16 port);
    void close();

protected:
private slots:
    void newConnection();
    void clientDisconnected();
    void closeConnection();
    void writeData();
    void readData();
    void bytesWritten(qint64 bytes);

private:
    QTcpServer* mServer;
    QTcpSocket* mClient;
    QTimer mTimer;
    QFile* mFile;
    bool mReadMore;
    qint64 mTotalWrittenBytes;
    qint64 mTotalReadBytes;
};

#endif
