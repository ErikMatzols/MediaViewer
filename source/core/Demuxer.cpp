#include "Demuxer.hpp"
#include "AudioDecoder.hpp"
#include "AudioRenderer.hpp"
#include "VideoDecoder.hpp"
#include "VideoRenderer.hpp"

#include "SubParser.hpp"

#include <QApplication>
#include <QFileInfo>

#include "Log.hpp"

namespace {
QString getStreamDescription(AVStream* stream)
{
    char buf[1024];
    avcodec_string(buf, sizeof(buf), stream->codec, 0);
    QString name = buf;
    name.remove(0, name.indexOf(": ") + 2);
    return name;
}
}

Demuxer::Demuxer(QWidget* parent, VideoRenderer* videoRenderer, AudioRenderer* audioRenderer)
    : QThread(parent)
{
    setObjectName("Demuxer");

    mVideoRenderer = videoRenderer;
    mAudioRenderer = audioRenderer;
    mMux = new Mux;
    mMux->mMaxQueueSize = 200;

    av_register_all();
    //av_log_set_callback(logCallback);
}

Demuxer::~Demuxer()
{
}

void Demuxer::startDemuxer(const QString& source, const QString& sourceName, const QString& remoteSub)
{
    mMux->mSource = source;
    mMux->mSourceName = sourceName;
    mMux->mSubtitle = remoteSub;
    start(QThread::HighPriority);
}

void Demuxer::stopDemuxer()
{
    mMux->mStopped = true;
    wait();
}

void Demuxer::changeVideoStream(int index)
{
    mMux->mDemuxerMutex.lock();
    mMux->mVideoDecoderMutex.lock();
    closeStream(mMux->mVideoIndex);
    if (index > -1)
        openStream(index);
    mMux->mDemuxerMutex.unlock();
    mMux->mVideoDecoderMutex.unlock();
}

void Demuxer::changeAudioStream(int index)
{
    mMux->mDemuxerMutex.lock();
    mMux->mAudioDecoderMutex.lock();
    closeStream(mMux->mAudioIndex);
    if (index > -1)
        openStream(index);
    mMux->mDemuxerMutex.unlock();
    mMux->mAudioDecoderMutex.unlock();
}

void Demuxer::changeSubtitleStream(int index)
{
    mMux->mDemuxerMutex.lock();
    mMux->mVideoDecoderMutex.lock();
    if (mMux->mExternalSubtitle) {
        mMux->mExternalSubtitleOpen = !mMux->mExternalSubtitleOpen;
        mMux->mSubtitleQueue.clear();
        mMux->mSubtitleFrameDecoded = false;
    } else {
        closeStream(mMux->mSubtitleIndex);
        if (index > -1)
            openStream(index);
    }
    mMux->mDemuxerMutex.unlock();
    mMux->mVideoDecoderMutex.unlock();
}

void Demuxer::setDemuxingState()
{
    mMux->masterClock.resume();
    mMux->mPaused = false;
    mMux->state = DemuxerState::DEMUXING;
}

void Demuxer::setPausedState()
{
    mMux->mPaused = true;
    mMux->masterClock.pause();
    mMux->state = DemuxerState::PAUSED;
}

void Demuxer::setSeekingState(double seekTime)
{
    if (seekTime < 0)
        seekTime = 0;

    mMux->seekTime = seekTime;
    mMux->state = DemuxerState::SEEKING;

    //    QString str = QString("%1:%2:%3")
    //                  .arg((int)seekTime / 3600, 2, 10, QChar('0'))
    //                  .arg(((int)seekTime % 3600) / 60, 2, 10, QChar('0'))
    //                  .arg(((int)seekTime % 3600) % 60, 2, 10, QChar('0'));
}

double Demuxer::currentMasterPts()
{
    mMux->mDemuxerMutex.lock();
    double mst = mMux->masterPts;
    mMux->mDemuxerMutex.unlock();
    return mst;
}

void Demuxer::run()
{
    std::cout << "Demuxer started\n";

    mVideoDecoder = new VideoDecoder(mVideoRenderer, mMux);
    mAudioDecoder = new AudioDecoder(mAudioRenderer, mMux);

    mMux->mStopped = !openMediaFile();

    if (!mMux->mStopped) {
        if (mMux->mVideoIndex != -1)
            mVideoDecoder->start(QThread::HighPriority);

        if (mMux->mAudioIndex != -1)
            mAudioDecoder->start(QThread::HighPriority);
    }

    bool demuxerDone = false;
    double prevTime = 0;
    mProgressTimer.reset();

    while (!demuxerDone && !mMux->mStopped) {
        updateMasterPts();

        switch (mMux->state) {
        case DemuxerState::DEMUXING:
            demuxing();
            if (mMux->mEndReached && !mMux->mVideoQueue.size() && !mMux->mAudioQueue.size() && !mMux->mSubtitleQueue.size())
                demuxerDone = true;
            break;
        case DemuxerState::PAUSED:
            paused();
            break;
        case DemuxerState::SEEKING:
            seeking();
            break;
        default:
            std::cout << "Error: Illegal demuxer state\n";
            break;
        }

        double currentTime = mProgressTimer.getTime();
        if (currentTime - prevTime > 0.2) {
            emit progressUpdate(mMux->masterPts);
            prevTime = currentTime;
        }
    }

    std::cout << "Demuxer finished\n";

    closeMediaFile();

    if (!mMux->mStopped)
        emit demuxerFinished();
}

void Demuxer::demuxing()
{
    AVPacket packet;

    bool isQueueFull = mMux->mVideoQueue.size() == mMux->mMaxQueueSize || mMux->mAudioQueue.size() == mMux->mMaxQueueSize || mMux->mSubtitleQueue.size() == mMux->mMaxQueueSize;

    if (!isQueueFull) {
        if (av_read_frame(mMux->mFormatCtx, &packet) < 0) {
            mMux->mEndReached = true;
        } else {
            if (mFirstPacket) {
                if (packet.pts >= 0)
                    mInitPts = packet.pts;
                else if (packet.dts >= 0)
                    mInitPts = packet.dts;
                mFirstPacket = false;
            }
            if (packet.pts >= 0 && packet.dts >= 0) {
                packet.pts -= mInitPts;
                packet.dts -= mInitPts;
            } else if (packet.pts >= 0)
                packet.pts -= mInitPts;
            else
                packet.dts -= mInitPts;

            mMux->mDemuxerMutex.lock();
            if (packet.stream_index == mMux->mVideoIndex) {
                mMux->mVideoQueueMutex.lock();
                mMux->mVideoQueue.append(dupPacket(&packet));
                mMux->mVideoQueueMutex.unlock();
            } else if (packet.stream_index == mMux->mAudioIndex) {
                mMux->mAudioQueueMutex.lock();
                mMux->mAudioQueue.append(dupPacket(&packet));
                mMux->mAudioQueueMutex.unlock();
            } else if (packet.stream_index == mMux->mSubtitleIndex) {
                //mMux->mSubtitleQueueMutex.lock();
                //mMux->mSubtitleQueue.append(dupPacket(&packet));
                //mMux->mSubtitleQueueMutex.unlock();
            } else
                av_free_packet(&packet);

            mMux->mDemuxerMutex.unlock();
        }
    }

    if (isQueueFull) {
        msleep(1);
    }
}

void Demuxer::paused()
{
    msleep(1);
}

void Demuxer::seeking()
{
    mMux->mDemuxerMutex.lock();
    mMux->mVideoQueueMutex.lock();
    mMux->mAudioQueueMutex.lock();

    if (mMux->seekTime < 0) {
        mMux->seekTime = 0;
    }

    int flags = AVSEEK_FLAG_BACKWARD;

    av_seek_frame(mMux->mFormatCtx, -1, (int64_t)(mMux->seekTime * AV_TIME_BASE), flags);

    if (mMux->mVideoIndex >= 0)
        avcodec_flush_buffers(mMux->mFormatCtx->streams[mMux->mVideoIndex]->codec);
    if (mMux->mAudioIndex >= 0)
        avcodec_flush_buffers(mMux->mFormatCtx->streams[mMux->mAudioIndex]->codec);
    if (mMux->mSubtitleIndex >= 0 && !mMux->mRawTextSubtitle)
        avcodec_flush_buffers(mMux->mFormatCtx->streams[mMux->mSubtitleIndex]->codec);

    clearQueue(mMux->mVideoQueue);
    clearQueue(mMux->mAudioQueue);
    clearQueue(mMux->mSubtitleQueue);

    mMux->mSubtitleFrameDecoded = 0;

    decodeSyncAudioVideo();

    if (mMux->mPaused)
        mMux->state = DemuxerState::PAUSED;
    else
        mMux->state = DemuxerState::DEMUXING;

    mMux->mAudioQueueMutex.unlock();
    mMux->mVideoQueueMutex.unlock();
    mMux->mDemuxerMutex.unlock();
}

void Demuxer::setDebug(bool debug)
{
    mMux->mDebug = debug;
}

bool Demuxer::openMediaFile()
{
    mMux->mFormatCtx = NULL;
    mMux->state = DemuxerState::DEMUXING;
    mMux->mStopped = false;
    mMux->mEndReached = false;
    mMux->mPaused = false;
    mMux->mDebug = false;
    mMux->mSubtitleFrameDecoded = 0;
    mMux->seekTime = 0;
    mMux->videoPts = 0;
    mMux->audioPts = 0;
    mMux->mSubtitlePts = 0;

    mMux->mRawTextSubtitle = false;
    mMux->mExternalSubtitle = false;
    mMux->mExternalSubtitleOpen = false;
    mMux->masterPts = 0;
    mMux->mVideoIndex = -1;
    mMux->mAudioIndex = -1;
    mMux->mSubtitleIndex = -1;
    mMux->mVideoQueue.clear();
    mMux->mAudioQueue.clear();
    mMux->mSubtitleQueue.clear();
    mMux->mVideo.clear();
    mMux->mAudioStreams.clear();
    mMux->mSubtitleStreams.clear();

    mInitPts = 0;
    mFirstPacket = true;

    if (avformat_open_input(&mMux->mFormatCtx, mMux->mSource.toStdString().c_str(), 0, 0) != 0) {
        std::cout << "Error: Cannot open file " << mMux->mSource.toStdString().c_str() << "\n";
        return false;
    }

    if (avformat_find_stream_info(mMux->mFormatCtx, 0) < 0) {
        std::cout << "Error: Cannot find stream info\n";
        return false;
    }

    av_dump_format(mMux->mFormatCtx, 0, mMux->mSource.toStdString().c_str(), 0);
    if (mMux->mFormatCtx->duration != AV_NOPTS_VALUE) {
        int secs = mMux->mFormatCtx->duration / AV_TIME_BASE;
        mMux->mDuration = secs;
        emit registerStreamDuration(secs);
    } else {
        std::cout << "Warning: Stream duration not available\n";
    }

    for (unsigned int i = 0; i < mMux->mFormatCtx->nb_streams; i++) {
        if (mMux->mFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
            mMux->mVideo.append(StreamDescriptor(i, getStreamDescription(mMux->mFormatCtx->streams[i])));
        if (mMux->mFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
            mMux->mAudioStreams.append(StreamDescriptor(i, getStreamDescription(mMux->mFormatCtx->streams[i])));
        if (mMux->mFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_SUBTITLE)
            mMux->mSubtitleStreams.append(StreamDescriptor(i, getStreamDescription(mMux->mFormatCtx->streams[i])));
    }

    if (mMux->mVideo.size()) {
        registerVideoStreams(mMux->mVideo);
        openStream(mMux->mVideo[0].index);

        if (mMux->mSubtitleStreams.size()) {
            registerSubtitleStreams(mMux->mSubtitleStreams);
            openStream(mMux->mSubtitleStreams[0].index);
        }

//       QString file;
//       if (!mMux->mSubtitle.size())
//       {
//           QFileInfo info(mMux->mSource);
//           file = info.absolutePath() + "/" + info.completeBaseName();
//       }
//       else
//       {
//           QFileInfo info(mMux->mSubtitle);
//           file = info.absolutePath() + "/" + info.completeBaseName();
//       }

//       if (mSubParser->openSubRipFile(file + ".srt"))
//       {
//           std::cout << "Using external .str file\n";
//           mMux->mExternalSubtitle = true;
//           mMux->mRawTextSubtitle = true;
//       }
//       else if (mSubParser->openMicroDVDFile(file + ".sub",mMux->mFormatCtx->streams[mMux->mVideoIndex]->time_base.num / (float)mMux->mFormatCtx->streams[mMux->mVideoIndex]->time_base.den))
//       {
//           std::cout << "Using external .sub file\n";
//           mMux->mExternalSubtitle = true;
//           mMux->mRawTextSubtitle = true;
//       }

//       if (mMux->mExternalSubtitle)
//       {
//           QList<int> lst;
//           lst.append(0);
//           registerSubtitleStreams(lst);
//           mMux->mExternalSubtitleOpen = true;
//       }
//       else
//       {
//           if (mMux->mSubtitleStreams.size())
//           {
//               registerSubtitleStreams(mMux->mSubtitleStreams);
//               openStream(mMux->mSubtitleStreams[0]);
//           }
//       }
    }
    if (mMux->mAudioStreams.size()) {
        registerAudioStreams(mMux->mAudioStreams);
        openStream(mMux->mAudioStreams[0].index);
    }

    mMux->masterClock.reset();

    return true;
}

void Demuxer::closeMediaFile()
{
    if (mMux->mFormatCtx) {
        mAudioDecoder->wait();
        delete mAudioDecoder;
        closeStream(mMux->mAudioIndex);

        mVideoDecoder->wait();
        delete mVideoDecoder;
        closeStream(mMux->mVideoIndex);
        closeStream(mMux->mSubtitleIndex);
        avformat_close_input(&mMux->mFormatCtx);
    }
}

bool Demuxer::openStream(int index)
{
    if (index < 0)
        return false;
    switch (mMux->mFormatCtx->streams[index]->codec->codec_type) {
    case AVMEDIA_TYPE_VIDEO: {
        closeStream(mMux->mVideoIndex);
        AVCodec* codec = avcodec_find_decoder(mMux->mFormatCtx->streams[index]->codec->codec_id);
        if (!codec) {
            std::cout << "Error: Cannot find video codec\n";
            return false;
        }
        if (avcodec_open2(mMux->mFormatCtx->streams[index]->codec, codec, 0) < 0) {
            std::cout << "Error: Cannot open video codec\n";
            return false;
        }

        mMux->mVideoIndex = index;
    } break;
    case AVMEDIA_TYPE_AUDIO: {
        closeStream(mMux->mAudioIndex);
        AVCodec* codec = avcodec_find_decoder(mMux->mFormatCtx->streams[index]->codec->codec_id);
        if (!codec) {
            std::cout << "Error: Cannot find audio codec\n";
            return false;
        }
        if (avcodec_open2(mMux->mFormatCtx->streams[index]->codec, codec, 0) < 0) {
            std::cout << "Error: Cannot open audio codec\n";
            return false;
        }

        mMux->mAudioIndex = index;
    } break;
    case AVMEDIA_TYPE_SUBTITLE: {
        closeStream(mMux->mSubtitleIndex);
        switch (mMux->mFormatCtx->streams[index]->codec->codec_id) {
        case AV_CODEC_ID_TEXT: // RAW TEXT
            mMux->mRawTextSubtitle = true;
            break;
        default:
            AVCodec* codec = avcodec_find_decoder(mMux->mFormatCtx->streams[index]->codec->codec_id);
            if (!codec) {
                std::cout << "Error: Cannot find subtitle codec\n";
                return false;
            }
            if (avcodec_open2(mMux->mFormatCtx->streams[index]->codec, codec, 0) < 0) {
                std::cout << "Error: Cannot open subtitle codec\n";
                return false;
            }
            break;
        }
        mMux->mSubtitleIndex = index;
    } break;
    default:
        std::cout << "Error: Unknown codec type\n";
        return false;
    }
    return true;
}

void Demuxer::closeStream(int index)
{
    if (index < 0)
        return;

    switch (mMux->mFormatCtx->streams[index]->codec->codec_type) {
    case AVMEDIA_TYPE_VIDEO:
        clearQueue(mMux->mVideoQueue);
        mMux->mVideoIndex = -1;
        break;
    case AVMEDIA_TYPE_AUDIO:
        clearQueue(mMux->mAudioQueue);
        mMux->mAudioIndex = -1;
        break;
    case AVMEDIA_TYPE_SUBTITLE:
        clearQueue(mMux->mSubtitleQueue);
        mMux->mSubtitleIndex = -1;
        break;
    default:
        break;
    }

    // shall not be used for subtitlestream with raw text because no codec is used
    if (!(mMux->mFormatCtx->streams[index]->codec->codec_type == AVMEDIA_TYPE_SUBTITLE && mMux->mRawTextSubtitle)) {
        avcodec_flush_buffers(mMux->mFormatCtx->streams[index]->codec);
        avcodec_close(mMux->mFormatCtx->streams[index]->codec);
    }
}

AVPacket* Demuxer::dupPacket(AVPacket* pkt)
{
    AVPacket* dup = (AVPacket*)av_malloc(sizeof(AVPacket));
    *dup = *pkt;

    if (av_dup_packet(dup) < 0) {
        std::cout << "Error: Duping packet\n";
        return NULL;
    }

    return dup;
}

void Demuxer::clearQueue(QQueue<AVPacket*>& queue)
{
    while (queue.size() > 0) {
        AVPacket* packet = queue.dequeue();
        av_free_packet(packet);
        av_free(packet);
    }
}

void Demuxer::logCallback(void* ptr, int level, const char* fmt, va_list vl)
{
    static char message[8192];
    const char* module = NULL;
    const char* className = NULL;
    if (level > av_log_get_level())
        return;

    if (ptr) {
        AVClass* avc = *(AVClass**)ptr;
        module = avc->item_name(ptr);
        className = avc->class_name;
        std::cout << "Module " << module << " ";
    }

    vsnprintf(message, sizeof message, fmt, vl);
    std::cout << message;
}

void Demuxer::updateMasterPts()
{
    if (mMux->mVideoIndex != -1 && mMux->mAudioIndex != -1) {
        // audio && video sync masterPts to master clock
        mMux->masterPts = mMux->masterClock.getTime();
        //printf("delta %f\n", mMux->audioPts - mMux->videoPts);
    } else if (mMux->mVideoIndex != -1 && mMux->mAudioIndex == -1) {
        // video only sync masterPts to master clock
        mMux->masterPts = mMux->masterClock.getTime();
    } else {
        // audio only sync masterPts to audio pts
        mMux->masterPts = mMux->audioPts;
    }
}

int Demuxer::queryMediaDuration(const QString& fileName)
{
    AVFormatContext* context;
    int duration = 0;
    if (avformat_open_input(&context, fileName.toStdString().c_str(), 0, 0) != 0) {
        std::cout << "Error: Cannot open file " << fileName.toStdString().c_str() << "\n";
        return duration;
    }

    if (avformat_find_stream_info(context, 0) < 0)
        std::cout << "Error: Cannot find stream info\n";

    if (context->duration != AV_NOPTS_VALUE)
        duration = context->duration / AV_TIME_BASE;
    else
        std::cout << "Warning: Stream duration not available\n";
    avformat_close_input(&context);
    return duration;
}

void Demuxer::decodeSyncAudioVideo()
{
    AVPacket* packet = av_packet_alloc();

    AVFrame* videoFrame = av_frame_alloc();
    AVFrame* audioFrame = av_frame_alloc();

    int videoFrameDecoded = 0;
    int audioFrameDecoded = 0;

    while (1) {
        if (av_read_frame(mMux->mFormatCtx, packet) < 0) {
            mMux->mEndReached = true;
            break;
        }

        if (packet->pts >= 0 && packet->dts >= 0) {
            packet->pts -= mInitPts;
            packet->dts -= mInitPts;
        } else if (packet->pts >= 0)
            packet->pts -= mInitPts;
        else
            packet->dts -= mInitPts;

        if (packet->stream_index == mMux->mVideoIndex) {
            int len = avcodec_decode_video2(mMux->mFormatCtx->streams[mMux->mVideoIndex]->codec, videoFrame,
                &videoFrameDecoded, packet);

            if (len < 0)
                std::cout << "Demuxer: Error Decoding seek video packet\n";

            double pts = 0;
            if ((packet->dts < 0) && videoFrame->opaque && (*(uint64_t*)videoFrame->opaque >= 0)) {
                pts = *(uint64_t*)videoFrame->opaque;
            } else if (packet->dts >= 0) {
                pts = packet->dts;
            }

            mMux->videoPts = pts * av_q2d(mMux->mFormatCtx->streams[mMux->mVideoIndex]->time_base);

            if (videoFrameDecoded /*&& abs(mState->mSeekTime - mState->mVideoPts) < 0.3*/)
                break;
        } else if (packet->stream_index == mMux->mAudioIndex && mMux->mVideoIndex == -1) {
            int used = avcodec_decode_audio4(mMux->mFormatCtx->streams[mMux->mAudioIndex]->codec, audioFrame, &audioFrameDecoded, packet);
            mMux->audioPts = packet->dts * av_q2d(mMux->mFormatCtx->streams[mMux->mAudioIndex]->time_base);
            if (used > 0 && abs(mMux->seekTime - mMux->audioPts) < 0.3) {
                break;
            }
        }
        av_free_packet(packet);
    }

    av_frame_free(&videoFrame);
    av_frame_free(&audioFrame);

    if (videoFrameDecoded) {
        mMux->masterPts = mMux->videoPts;
        mMux->masterClock.setTime(mMux->videoPts);
    } else if (audioFrameDecoded) {
        mMux->masterPts = mMux->audioPts;
        mMux->masterClock.setTime(mMux->audioPts);
    }
}
