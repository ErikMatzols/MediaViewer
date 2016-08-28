#ifndef DEMUXER_HPP
#define DEMUXER_HPP

#include "StreamState.hpp"
#include <QMutex>
#include <QQueue>
#include <QThread>

class AudioRenderer;
class VideoRenderer;
class VideoDecoder;
class AudioDecoder;
class SubParser;

class Demuxer : public QThread {
    Q_OBJECT

public:
    Demuxer(QWidget* parent, VideoRenderer* videoRenderer, AudioRenderer* audioRenderer);
    ~Demuxer();

    void startDemuxer(const QString& source, const QString& sourceName, const QString& remoteSub);
    void stopDemuxer();

    static int queryMediaDuration(const QString& fileName);

    void changeVideoStream(int index);
    void changeAudioStream(int index);
    void changeSubtitleStream(int index);

    void setDemuxingState();
    void setPausedState();
    void setSeekingState(double seekTime);

    void setDebug(bool debug);

    double currentMasterPts();

    Mux* getStreamState() { return mMux; }

signals:
    void registerStreamDuration(int duration);
    void registerVideoStreams(QList<StreamDescriptor> videoStreams);
    void registerAudioStreams(QList<StreamDescriptor> audioStreams);
    void registerSubtitleStreams(QList<StreamDescriptor> subtitleStreams);
    void demuxerFinished();
    void progressUpdate(double time);

protected:
    void run();

    bool openMediaFile();
    void closeMediaFile();

    void demuxing();
    void paused();
    void seeking();

    bool openStream(int index);
    void closeStream(int index);

    AVPacket* dupPacket(AVPacket* pkt);
    void clearQueue(QQueue<AVPacket*>& queue);

    void decodeSyncAudioVideo();

    static void logCallback(void* ptr, int level, const char* fmt, va_list vl);

    void updateMasterPts();

private:
    VideoDecoder* mVideoDecoder;
    AudioDecoder* mAudioDecoder;
    Mux* mMux;
    AudioRenderer* mAudioRenderer;
    VideoRenderer* mVideoRenderer;
    int64_t mInitPts;
    bool mFirstPacket;
    Timer mProgressTimer;
};

#endif
