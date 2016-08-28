#include "AudioRenderer.hpp"
#include "StreamState.hpp"
#include <QDataStream>
#include <QFile>
#include <iostream>

AudioRenderer::AudioRenderer(QObject* parent)
    : QObject(parent)
{
    mContext = 0;
    mDevice = 0;
    mDevicesName = alcGetString(NULL, ALC_DEVICE_SPECIFIER);
    mDefaultDeviceName = alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER);
    mGain = 100;
    mCompensateLatency = true;
    mBufferSize = mPendingBufferSize = 5;
}

AudioRenderer::~AudioRenderer()
{
}

void AudioRenderer::openAudioRenderer(ALsizei bits, ALsizei rate, ALsizei channels)
{
    mDevice = alcOpenDevice(0);
    mContext = alcCreateContext(mDevice, 0);
    alcMakeContextCurrent(mContext);

    mCurrentTotalBufferSize = 0;
    mBufferSize = mPendingBufferSize;
    mBuff = new ALuint[mBufferSize];
    alGenBuffers(mBufferSize, mBuff);
    alGenSources(1, &mSource);

    mFreq = rate;
    mBps = bits / 8;
    mChannels = channels;

    if (bits == 8) {
        if (mChannels == 1)
            mFormat = AL_FORMAT_MONO8;
        if (mChannels == 2)
            mFormat = AL_FORMAT_STEREO8;
        if (alIsExtensionPresent("AL_EXT_MCFORMATS")) {
            if (mChannels == 4)
                mFormat = alGetEnumValue("AL_FORMAT_QUAD8");
            if (mChannels == 6)
                mFormat = alGetEnumValue("AL_FORMAT_51CHN8");
            if (mChannels == 8)
                mFormat = alGetEnumValue("AL_FORMAT_71CHN8");
        }
    }
    if (bits == 16) {
        if (mChannels == 1)
            mFormat = AL_FORMAT_MONO16;
        if (mChannels == 2)
            mFormat = AL_FORMAT_STEREO16;

        if (alIsExtensionPresent("AL_EXT_MCFORMATS")) {
            if (mChannels == 4)
                mFormat = alGetEnumValue("AL_FORMAT_QUAD16");
            if (mChannels == 6)
                mFormat = alGetEnumValue("AL_FORMAT_51CHN16");
            if (mChannels == 8)
                mFormat = alGetEnumValue("AL_FORMAT_71CHN16");
        }
    }

    alSourcef(mSource, AL_GAIN, mGain);
}

void AudioRenderer::closeAudioRenderer()
{
    alcMakeContextCurrent(0);
    alcDestroyContext(mContext);
    alcCloseDevice(mDevice);

    alDeleteBuffers(mBufferSize, mBuff);
    alDeleteSources(1, &mSource);
    delete[] mBuff;
}

AudioRenderer::BufferResult AudioRenderer::queueBuffer(const ALvoid* data, ALsizei size)
{
    ALint processed, queued, buffSize;
    BufferResult result;

    alGetSourcei(mSource, AL_BUFFERS_QUEUED, &queued);
    alGetSourcei(mSource, AL_BUFFERS_PROCESSED, &processed);

    if (queued < mBufferSize) {
        mCurrentTotalBufferSize += size;
        alBufferData(mBuff[queued], mFormat, data, size, mFreq);
        alSourceQueueBuffers(mSource, 1, &mBuff[queued]);

        result = BufferResult::BufferQueued;
    } else if (processed > 0) {
        ALuint buf;
        alSourceUnqueueBuffers(mSource, 1, &buf);
        alGetBufferi(buf, AL_SIZE, &buffSize);

        mCurrentTotalBufferSize -= buffSize / mBps;
        mCurrentTotalBufferSize += size;

        alBufferData(buf, mFormat, data, size, mFreq);
        alSourceQueueBuffers(mSource, 1, &buf);

        result = BufferResult::BufferReplaced;
    } else {
        result = BufferResult::BufferFull;
    }

    ALint state;
    alGetSourcei(mSource, AL_SOURCE_STATE, &state);

    if (state != AL_PLAYING) {
        std::cout << "alSourcePlay\n";
        alSourcePlay(mSource);

    }

    errorCheck();

    return result;
}

void AudioRenderer::pause()
{
   ALint state;
   alGetSourcei(mSource, AL_SOURCE_STATE, &state);

   if (state == AL_PLAYING) {
      alSourcePause(mSource);
   }
}

void AudioRenderer::errorCheck()
{
    ALenum error = alGetError();
    switch (error) {
    case AL_NO_ERROR:
        break;
    case AL_INVALID_NAME:
        std::cout << "a bad name (ID) was passed to an OpenAL function\n";
        break;
    case AL_INVALID_ENUM:
        std::cout << "an invalid enum value was passed to an OpenAL function\n";
        break;
    case AL_INVALID_VALUE:
        std::cout << "an invalid value was passed to an OpenAL function\n";
        break;
    case AL_INVALID_OPERATION:
        std::cout << "the requested operation is not valid\n";
        break;
    case AL_OUT_OF_MEMORY:
        std::cout << "the requested operation resulted in OpenAL running out of memory\n";
        break;
    default:
        break;
    };
}

void AudioRenderer::adjustGain(float gain)
{
    if (gain > 1.0f)
        gain = 1.0f;
    else if (gain < 0.0f)
        gain = 0.0f;
    mGain = gain;
    alSourcef(mSource, AL_GAIN, mGain);
}

void AudioRenderer::setCompensateLatency(bool compensate)
{
    mCompensateLatency = compensate;
}

bool AudioRenderer::compensateBufferLatency()
{
    return mCompensateLatency;
}

const char* AudioRenderer::getDefaultDeviceName()
{
    return mDefaultDeviceName;
}

int AudioRenderer::getCurrentTotalBufferSize()
{
    return mCurrentTotalBufferSize;
}

int AudioRenderer::getBufferCount()
{
    return mBufferSize;
}

void AudioRenderer::setBufferCount(int buffCount)
{
    mPendingBufferSize = buffCount;
}

bool AudioRenderer::loadSettingsBinary(const QString& fileName)
{
    //    QFile file(fileName);
    //    if (!file.open(QIODevice::ReadOnly)) {
    //        std::cout << "Failed to load " << fileName.toStdString().c_str() << "\n";
    //        return false;
    //    }

    //    QDataStream in(&file);

    //    in.setVersion(QDataStream::Qt_4_6);
    //    in >> mPendingBufferSize;
    //    mBufferSize = mPendingBufferSize;
    //    in >> mCompensateLatency;

    //    file.close();

    return true;
}

bool AudioRenderer::saveSettingsBinary(const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly))
        return false;

    QDataStream out(&file);

    out.setVersion(QDataStream::Qt_4_6);
    out << mPendingBufferSize;
    out << mCompensateLatency;

    file.close();

    return true;
}

int AudioRenderer::getFreq()
{
    return mFreq;
}

int AudioRenderer::getBytesPerSample()
{
    return mBps;
}

int AudioRenderer::getChannelCount()
{
    return mChannels;
}

float AudioRenderer::getPlaybackPosition()
{
    float pos;
    alGetSourcef(mSource, AL_SEC_OFFSET, &pos);
    return pos;
}

void AudioRenderer::normalize(ALvoid* /*data*/, ALsizei /*size*/)
{
    //mFormat
    //mFreq
    /*
    int16_t *ptr = (int16_t*) data;
    int16_t peakLeft = 0;
    int16_t peakRight = 0;

    int values = size / 2;
    for (int i = 0; i < values; i += 2) {
        int val = abs(ptr[i]);
        int val2 = abs(ptr[i+1]);
        peakLeft = val > peakLeft ? val : peakLeft;
        peakRight = val2 > peakRight ? val2 : peakRight;
    }
    float leftAmp = (32767 / (float) peakLeft)*0.5;
    float rightAmp = (32767 / (float) peakRight)*0.5;

    for (int i = 0; i < values; i += 2) {
        ptr[i] = (int) (ptr[i] * leftAmp);
        ptr[i+1] = (int) (ptr[i+1] * rightAmp);
    }
*/
}
