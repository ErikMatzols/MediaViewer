#ifndef WINTUNER_HPP
#define WINTUNER_HPP

#include <QThread>
#include "ChannelInfo.hpp"
#include <comip.h>
class VideoRenderer;
//class RendererFilter;
class DemuxFilter;

class WinTuner : public QThread
{
    Q_OBJECT
public:
    WinTuner(QObject *parent, VideoRenderer *videoRenderer);

    ~WinTuner();

    void startDVB(const QString &deviceName);

    void stopDVB();

    void pauseDVB(bool isPaused);

    void showPropertyPage(const QString &filterName);

    QStringList queryTuningSpaces();

    QStringList queryFilters();

    long AnalogTune(AnalogTuneRequest);

    long DVBTTune(DVBTTuneRequest);

    long DVBCTune(DVBCTuneRequest);

    long DVBSTune(DVBSTuneRequest);
    
    QList<ChannelInfo*> AnalogScanRequest(AnalogTuneRequest);

    QList<ChannelInfo*> DVBTScanRequest(DVBTTuneRequest);

    QList<ChannelInfo*> DVBCScanRequest(DVBCTuneRequest);

    QList<ChannelInfo*> DVBSScanRequest(DVBSTuneRequest);

    QString getCurrentDeviceName();

    QStringList getAvailableModes();

    void setMode(const QString& mode);

    AMTunerModeType modeToEnum(const QString& mode);

    QStringList getInputTypes();

    void setInputType(const QString& inputType);

    TunerInputType inputTypeToEnum(const QString& inputType);

    QStringList getAvailableInputs();

    void setCurrentInput(const QString &input);

    QString getCountryCode();

    void setCountryCode(const QString &code);

    QString getTuningSpace();

    void setTuningSpace(const QString &space);

    void getChannelMinMax(int& min,int& max);

    void mapPidToVideoPin(MediaPid stream);

    void mapPidToAudioPin(MediaPid stream);

signals:
    void graphRunning();

    void signalReceived(long strength);

    void analogChannelChanged(int channel);

    void tuneRequestFinished();

protected:
    bool initialize();

    void uninitialize();

    void run();

    bool buildAndConnectGraphFromFile(const QString &fileName);

    QList<ChannelInfo*> queryTables(long bandwidth, long frequency);

    IBaseFilter* createFilterSmart(const QString &dName);

    IPin* getPinSmart(IBaseFilter* pFilter, const QString &pinName, PIN_DIRECTION dir);

    ITuningSpace* retreiveTuningSpace(const QString &tuningSpaceName);

private:
    bool stopped;
    bool paused;
    bool togglePaused;
    IFilterGraph *pGraph;
    IAMTVTuner *pTunerAnalog;
    IScanningTuner *pTunerBDA;
    IMpeg2Data *pMpeg2Data;
    IMpeg2Demultiplexer *pMpeg2Demux;
    IPin *pPSIPin;
    IPin *pVideoPin;
    IPin *pAudioPin;

    DemuxFilter *pDumpFilter;
    QString deviceName;
    VideoRenderer *videoRenderer;
    IQualProp *pQualProp;
    IPin *pPin;
    IGuideData *pGuideData;
};

#endif
