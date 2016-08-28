#ifndef AUDIORENDERER_HPP
#define AUDIORENDERER_HPP

#include <AL/al.h>
#include <AL/alc.h>
#include <QObject>

class AudioRenderer : public QObject {
    Q_OBJECT
public:
    AudioRenderer(QObject* parent);
    ~AudioRenderer();

    enum class BufferResult {
        BufferQueued,
        BufferReplaced,
        BufferFull
    };

    void openAudioRenderer(ALsizei bits, ALsizei rate, ALsizei channels);
    void closeAudioRenderer();

    BufferResult queueBuffer(const ALvoid* data, ALsizei size);
    void adjustGain(float gain);

    void errorCheck();

    void setCompensateLatency(bool compensate);
    bool compensateBufferLatency();

    const char* getDefaultDeviceName();

    int getCurrentTotalBufferSize();

    void setBufferCount(int buffSize);
    int getBufferCount();

    int getFreq();
    int getBytesPerSample();
    int getChannelCount();
    float getPlaybackPosition();

    bool loadSettingsBinary(const QString& fileName);
    bool saveSettingsBinary(const QString& fileName);

    void pause();
protected:
    void normalize(ALvoid* data, ALsizei size);

private:
    ALCcontext* mContext;
    ALCdevice* mDevice;

    const ALchar* mDevicesName;
    const ALchar* mDefaultDeviceName;

    ALenum mFormat;
    ALsizei mBps;
    ALsizei mFreq;
    ALsizei mChannels;

    ALuint* mBuff;

    ALsizei mCurrentTotalBufferSize;
    ALsizei mBufferSize;
    ALuint mPendingBufferSize;
    ALuint mSource;
    ALfloat mGain;

    bool mCompensateLatency;
};

#endif
