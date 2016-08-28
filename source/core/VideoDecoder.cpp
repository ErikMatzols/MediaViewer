#include "VideoDecoder.hpp"
#include "VideoRenderer.hpp"

#include <iostream>

namespace {
void yield(int ms)
{
    Sleep(ms);
}

bool isEndOfStream(Mux* mux)
{
    return mux->mStopped || (mux->mEndReached && mux->mVideoQueue.isEmpty() && mux->mSubtitleQueue.isEmpty());
}
}

VideoDecoder::VideoDecoder(VideoRenderer* videoRenderer, Mux* mux)
{
    setObjectName("VideoDecoder");
    mRenderer = videoRenderer;
    mMux = mux;
}

VideoDecoder::~VideoDecoder()
{
}

void VideoDecoder::run()
{
    std::cout << "VideoDecoder started\n";

    mStream = mMux->mFormatCtx->streams[mMux->mVideoIndex];
    mCodec = mStream->codec;
    mFrame = av_frame_alloc();
    mGotFrame = 0;
    mPts = 0;
    mDts = 0;

    openRenderer();

    while (!isEndOfStream(mMux)) {
        if (!mGotFrame) {
            decodeVideo();
        }
        if (mGotFrame) {
            render();
        }

        yield(4);
    }

    closeRenderer();

    av_frame_free(&mFrame);

    std::cout << "VideoDecoder finished\n";
}

void VideoDecoder::openRenderer()
{
    QMetaObject::invokeMethod(mRenderer, "moveContextToThread", Qt::BlockingQueuedConnection, Q_ARG(void*, currentThread()));
    mRenderer->openVideoRenderer(mCodec->width, mCodec->height, mCodec->pix_fmt, 1.0f);
}

void VideoDecoder::closeRenderer()
{
    mRenderer->closeVideoRenderer();
    mRenderer->moveContextToThread(0);
}

void VideoDecoder::decodeVideo()
{
    mMux->mVideoQueueMutex.lock();

    AVPacket* packet = !mMux->mVideoQueue.isEmpty() ? mMux->mVideoQueue.dequeue() : 0;

    if (!packet) {
        mMux->mVideoQueueMutex.unlock();
        return;
    }

    decodePacket(packet);

    av_free_packet(packet);
    av_free(packet);

    mMux->mVideoQueueMutex.unlock();
}

void VideoDecoder::decodePacket(AVPacket* pkt)
{
    int len = avcodec_decode_video2(mCodec, mFrame, &mGotFrame, pkt);

    if (len < 0) {
        std::cout << "VideoDecoder: skipping video frame\n";
        return;
    }

    mPts = pkt->pts;
    mDts = pkt->dts;

    if ((mDts < 0) && mFrame->opaque && (*(uint64_t*)mFrame->opaque >= 0)) {
        mPts = *(uint64_t*)mFrame->opaque;
    } else if (mDts >= 0) {
        mPts = av_frame_get_best_effort_timestamp(mFrame);
    }

    mMux->videoPts = mPts * av_q2d(mStream->time_base);
}

void VideoDecoder::render()
{
    if (mMux->masterPts >= mMux->videoPts) {
        mRenderer->renderVideo(mFrame->data, mFrame->linesize, mMux);
        mGotFrame = 0;
    } else if (mMux->mPaused) {
        mRenderer->renderVideo(mFrame->data, mFrame->linesize, mMux);
    }
}
