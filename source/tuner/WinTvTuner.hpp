/*
#ifndef WINDOWS_TVTUNER_HPP
#define WINDOWS_TVTUNER_HPP

#include "TunerInterface.hpp"
#include "WinFrameGrabCallback.hpp"

class WinTvTuner : public TunerInterface
{
    Q_OBJECT
public:
    WinTvTuner(QObject *parent, VideoRenderer* renderer);
    WinTvTuner(QObject *parent);
    ~WinTvTuner();

    void startTuner(enum ThreadMode mode);
    void pauseTuner(bool pause);
    void stopTuner();
    void setChannel(int channel);
    int getChannel();

    void nextChannel();
    void prevChannel();
    void gotoChannel(const QString& name);
    void gotoChannelIndex(int index);

    int openChannelConfiguration(const QString& graphFile);
    void closeChannelConfiguration();

    void getAvailableModes(QStringList& modes);
    void setMode(const QString& mode);

    void getAvailableTVFormats(QStringList& TVformats);
    void setTVFormat(const QString& TVFormat);

    void getAvailableInputs(QStringList& inputs);
    void setCurrentInput(const QString& input);

    void getInputType(QStringList& inputTypes);
    void setInputType(const QString& inputType);

    void getCountryCode(QString& code);
    void setCountryCode(const QString& code);
    void getTuningSpace(QString& space);
    void setTuningSpace(const QString& space);

    void getChannelMinMax(int& min,int& max);

    void setVolume(int volume);

protected:
    void run();

    void autoTune();
    void runGraph();

    HRESULT buildGraphFromFile(const CComPtr<IFilterGraph>& graph,const QString& graphFile);
    CComPtr<IBaseFilter> createFilter(const QString& dName);
    CComPtr<IPin> getPin(IBaseFilter *pFilter,const QString& pinName);
    HRESULT initFrameGrabber();

private:
    VideoRenderer *renderer;
    WinFrameGrabCallback grabberCallback;

    IAMTVTuner *pTuner;
    IAMCrossbar *pCrossbar;
    ISampleGrabber *pSampleGrab;
    IBasicAudio *pAudio;

    //QString graphFile;
    bool stopped;
    bool mPaused;
    bool mTogglePaused;
    //int lastChannel;
};

#endif
*/