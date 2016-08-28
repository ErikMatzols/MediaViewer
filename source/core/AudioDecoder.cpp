#include "AudioDecoder.hpp"
#include "AudioRenderer.hpp"

#include <iostream>

namespace {
void yield()
{
    Sleep(2);
}

bool isBufferFullYield(AudioRenderer::BufferResult result)
{
    if (result == AudioRenderer::BufferResult::BufferFull) {
        yield();
        return true;
    }
    return false;
}

bool isEndOfStream(Mux* mux)
{
    return mux->mStopped || (mux->mEndReached && mux->mAudioQueue.isEmpty());
}
}

AudioDecoder::AudioDecoder(AudioRenderer* audioRenderer, Mux* state)
{
    setObjectName("AudioDecoder");
    mRenderer = audioRenderer;
    mState = state;
}

AudioDecoder::~AudioDecoder()
{
}

void AudioDecoder::run()
{
    std::cout << "AudioDecoder started\n";

    mStream = mState->mFormatCtx->streams[mState->mAudioIndex];
    mCodec = mStream->codec;
    mFrame = av_frame_alloc();

    mCodec->channel_layout = av_get_default_channel_layout(mCodec->channels);
    mChannels = mCodec->channels == 1 ? mCodec->channels : 2;

    mRenderer->openAudioRenderer(16, mCodec->sample_rate, mChannels);

    int64_t out_channel_layout = av_get_default_channel_layout(mChannels);
    mSwr = swr_alloc_set_opts(NULL,
        out_channel_layout, AV_SAMPLE_FMT_S16, mCodec->sample_rate,
        mCodec->channel_layout, mCodec->sample_fmt, mCodec->sample_rate,
        0, NULL);

    if (swr_init(mSwr) < 0) {
        std::cout << "swr_init failed\n";
    }

    while (!isEndOfStream(mState)) {
        if (mState->mPaused) {
            mRenderer->pause();
            yield();
        } else {
            decodeAudio();
        }
    }

    mRenderer->closeAudioRenderer();

    av_frame_free(&mFrame);
    swr_free(&mSwr);

    std::cout << "AudioDecoder finished\n";
}

void AudioDecoder::decodeAudio()
{
    mState->mAudioQueueMutex.lock();
    AVPacket* packet = !mState->mAudioQueue.isEmpty() ? mState->mAudioQueue.dequeue() : 0;
    mState->mAudioQueueMutex.unlock();

    if (!packet) {
        yield();
        return;
    }

    decodePacket(packet);
    uint8_t* buffer;
    if (allocateBuffer(&buffer) < 0) {
        av_free_packet(packet);
        av_free(packet);
        return;
    }

    int buffer_size = interleaveBuffer(&buffer);
    if (buffer_size == -1) {
       av_free_packet(packet);
       av_free(packet);
       return;
    }

    updateTimestamps(packet->pts, packet->dts);

    render(buffer, buffer_size);

    av_free(&buffer[0]);
    av_free_packet(packet);
    av_free(packet);
}

int AudioDecoder::decodePacket(AVPacket* pkt)
{
    int packet_size = pkt->size;
    int buffer_size = 0;

    while (packet_size > 0) {
        int got_frame = 0;
        int len = avcodec_decode_audio4(mCodec, mFrame, &got_frame, pkt);

        if (len < 0) {
            std::cout << "AudioDecoder: skipping audio frame\n";
            return 0;
        } else if (got_frame) {
            buffer_size = av_samples_get_buffer_size(
                NULL,
                mCodec->channels,
                mFrame->nb_samples,
                mCodec->sample_fmt,
                0);
        }

        packet_size -= len;

        if (buffer_size > 0) {
            break;
        }
    }

    return buffer_size;
}

int AudioDecoder::allocateBuffer(uint8_t** buffer)
{
    int linesize = 0;
    int ret = av_samples_alloc(buffer, &linesize, mChannels, mFrame->nb_samples, AV_SAMPLE_FMT_S16, 0);
    if (ret < 0) {
        std::cout << "av_samples_alloc failed\n";
    }

    return ret;
}

int AudioDecoder::interleaveBuffer(uint8_t** out)
{
    int samples = mFrame->nb_samples;
    int size = 0;
    if ((size = swr_convert(mSwr, out, samples, (const uint8_t**)mFrame->data, samples)) < 0) {
        std::cout << "swr_convert failed\n";
        return -1;
    }

    mBufferLatency = (size / mCodec->sample_rate) * 4;

    return size * 2 * mChannels;
}

void AudioDecoder::render(uint8_t* output, int buffer_size)
{
    if (buffer_size > 0) {
        AudioRenderer::BufferResult result;
        do {
            result = mRenderer->queueBuffer(output, buffer_size);
        } while (isBufferFullYield(result));
    }
}

void AudioDecoder::updateTimestamps(int64_t pts, int64_t dts)
{
    if (pts >= 0)
        mState->audioPts = pts * av_q2d(mStream->time_base);
    else if (dts >= 0)
        mState->audioPts = dts * av_q2d(mStream->time_base);
    else
        std::cout << "AudioDecoder: AudioPts AV_NOPTS_VALUE\n";
}
