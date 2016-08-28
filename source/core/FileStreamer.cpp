#include "FileStreamer.hpp"

FileStreamer::FileStreamer(const QString& fileName, QObject* parent)
    : QObject(parent)
{
    mServer = new QTcpServer(this);
    mClient = NULL;
    mFile = new QFile(fileName);
    mReadMore = true;
    mTotalWrittenBytes = 0;
    mTotalReadBytes = 0;
    connect(mServer, SIGNAL(newConnection()), this, SLOT(newConnection()));
}

FileStreamer::~FileStreamer()
{
}

bool FileStreamer::listen(const QHostAddress& address, quint16 port)
{
    std::cout << "Server listening\n";
    return mServer->listen(address, port);
}

void FileStreamer::close()
{
    mServer->close();
    closeConnection();
}

void FileStreamer::newConnection()
{
    std::cout << "Incoming connection\n";
    QTcpSocket* tmp = mServer->nextPendingConnection();
    if (mClient == NULL) {
        std::cout << "Client connected\n";
        mClient = tmp;
        connect(mClient, SIGNAL(disconnected()), this, SLOT(clientDisconnected()));
        connect(mClient, SIGNAL(readyRead()), this, SLOT(readData()));
        connect(mClient, SIGNAL(bytesWritten(qint64)), this, SLOT(bytesWritten(qint64)));
        connect(&mTimer, SIGNAL(timeout()), this, SLOT(writeData()));
        mTimer.start(100);
        mFile->open(QIODevice::ReadOnly);
    } else {
        std::cout << "Client dropped\n";
        delete tmp;
    }
}

void FileStreamer::clientDisconnected()
{
    std::cout << "Client disconnected\n";
    closeConnection();
}

void FileStreamer::closeConnection()
{
    mFile->close();
    if (mClient)
        mClient->close();
    mTimer.stop();
}

void FileStreamer::writeData()
{
    if (mTotalReadBytes - mTotalWrittenBytes > BUFFER_THRESHOLD)
        mReadMore = false;
    else
        mReadMore = true;

    if (mReadMore) {
        if (!mFile->atEnd()) {
            QByteArray block = mFile->read(PAYLOAD_SIZE);
            mTotalReadBytes += block.size();
            mClient->write(block);
        } else {
            std::cout << "File end reached closing socket\n";
            closeConnection();
        }
    }
}

void FileStreamer::readData()
{
}

void FileStreamer::bytesWritten(qint64 bytes)
{
    mTotalWrittenBytes += bytes;
    std::cout << mTotalReadBytes / 1024 << " KB read " << mTotalWrittenBytes / 1024 << " KB written\n";
}
