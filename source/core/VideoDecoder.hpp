#ifndef VIDEODECODER_HPP
#define VIDEODECODER_HPP

#include "StreamState.hpp"
#include <QThread>

class VideoRenderer;

class VideoDecoder : public QThread {
public:
    VideoDecoder(VideoRenderer* videoRenderer, Mux* mux);
    ~VideoDecoder();

protected:
    void run();

    void openRenderer();
    void closeRenderer();

    void decodeVideo();
    void decodePacket(AVPacket* pkt);

    void render();

private:
    VideoRenderer* mRenderer;
    Mux* mMux;

    AVStream* mStream;
    AVCodecContext* mCodec;
    AVFrame* mFrame;
    int mPts, mDts, mGotFrame;
};

#endif
