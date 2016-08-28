#ifndef WINTUNERSCANREQUEST_HPP
#define WINTUNERSCANREQUEST_HPP

#include <QThread>
#include "WinTuner.hpp"

class WinTunerScanRequest : public QThread
{
    Q_OBJECT
public:
    WinTunerScanRequest(QObject *parent)
        : QThread(parent)
    {
    }

    WinTunerScanRequest()
    {
    }

    void startScanRequest(WinTuner *winTuner, QList<TuneRequestInfo*> tuneRequests)
    {
        this->winTuner = winTuner;
        this->tuneRequests = tuneRequests;
        start();
    }

    void stopScanRequest()
    {
        stopped = true;
        wait();
    }

signals:
    void channelsFound(QList<ChannelInfo*> channels);
    void scanRequestFinished();

protected:
    void run()
    {
        stopped = false;
        foreach(TuneRequestInfo *tuneRequest, tuneRequests) {
            QList<ChannelInfo*> channels = tuneRequest->tune(winTuner);
            if (!channels.isEmpty())
                emit channelsFound(channels);
            if (stopped)
                break;
        }
        foreach(TuneRequestInfo *tuneRequest, tuneRequests)
            delete tuneRequest;
        tuneRequests.clear();
        emit scanRequestFinished();
    }

private:
    QList<TuneRequestInfo*> tuneRequests;
    WinTuner *winTuner;
    bool stopped;
};

#endif
