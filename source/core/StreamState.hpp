#ifndef STREAMSTATE_HPP
#define STREAMSTATE_HPP

#define __STDC_CONSTANT_MACROS

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/pixdesc.h>
#include <libswresample/swresample.h>
}

#include "Timer.hpp"
#include <QList>
#include <QMutex>
#include <QQueue>
#include <QString>

enum class DemuxerState { DEMUXING,
    PAUSED,
    SEEKING };

struct StreamDescriptor {
    StreamDescriptor(int index, const QString& name)
        : index(index)
        , name(name)
    {
    }

    int index;
    QString name;
};

struct Mux {
    Mux()
    {
    }

    AVFormatContext* mFormatCtx;

    QList<StreamDescriptor> mVideo;
    QList<StreamDescriptor> mAudioStreams;
    QList<StreamDescriptor> mSubtitleStreams;

    Timer masterClock;
    double masterPts;
    double videoPts;
    double audioPts;

    bool mStopped;
    bool mEndReached;
    bool mPaused;
    bool mDebug;

    int mSubtitleFrameDecoded;

    int mVideoIndex;
    int mAudioIndex;
    int mSubtitleIndex;

    int mMaxQueueSize;
    int mDuration;
    double seekTime;
    double mSubtitlePts;

    QString mSource;
    QString mSourceName;
    QString mSubtitle;

    QString mSubtitleText;
    bool mRawTextSubtitle;
    bool mExternalSubtitle;
    bool mExternalSubtitleOpen;
    bool mRemoteSubtitle;

    QQueue<AVPacket*> mVideoQueue;
    QMutex mVideoQueueMutex;

    QQueue<AVPacket*> mAudioQueue;
    QMutex mAudioQueueMutex;

    QQueue<AVPacket*> mSubtitleQueue;
    QMutex mSubtitleQueueMutex;

    QMutex mDemuxerMutex;
    QMutex mAudioDecoderMutex;
    QMutex mVideoDecoderMutex;

    DemuxerState state;
};

#endif
