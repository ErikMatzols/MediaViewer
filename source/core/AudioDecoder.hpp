#ifndef AUDIODECODER_HPP
#define AUDIODECODER_HPP

#include "StreamState.hpp"
#include <QThread>

class AudioRenderer;

class AudioDecoder : public QThread {
public:
    AudioDecoder(AudioRenderer* audioRenderer, Mux* state);
    ~AudioDecoder();

protected:
    void run();
    void decodeAudio();
    int decodePacket(AVPacket* pkt);

    int allocateBuffer(uint8_t** buffer);
    int interleaveBuffer(uint8_t** out);
    void render(uint8_t* output, int buffer_size);
    void updateTimestamps(int64_t pts, int64_t dts);

private:
    AudioRenderer* mRenderer;
    Mux* mState;
    SwrContext* mSwr;
    AVStream* mStream;
    AVCodecContext* mCodec;
    AVFrame* mFrame;
    int mChannels;
    double mBufferLatency;
};

#endif
