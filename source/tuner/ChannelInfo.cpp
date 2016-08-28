#include "ChannelInfo.hpp"
#include "WinTuner.hpp"
#include <QDataStream>

QList<ChannelInfo*> AnalogTuneRequest::tune(WinTuner* winTuner)
{
    return winTuner->AnalogScanRequest(*this);
}

QList<ChannelInfo*> DVBTTuneRequest::tune(WinTuner* winTuner)
{
    return winTuner->DVBTScanRequest(*this);
}

QList<ChannelInfo*> DVBCTuneRequest::tune(WinTuner* winTuner)
{
    return winTuner->DVBCScanRequest(*this);
}

QList<ChannelInfo*> DVBSTuneRequest::tune(WinTuner* winTuner)
{
    return winTuner->DVBSScanRequest(*this);
}

ChannelInfo::ChannelInfo()
{
}

ChannelInfo::ChannelInfo(const QString& channelName, const QString& deviceName)
{
    this->channelName = channelName;
    this->deviceName = deviceName;
}

ChannelInfo::~ChannelInfo()
{
}

ChannelInfo* ChannelInfo::copy()
{
    return new ChannelInfo(channelName, deviceName);
}

int ChannelInfo::ID()
{
    return id;
}

void ChannelInfo::setChannelName(const QString& channelName)
{
    this->channelName = channelName;
}

QString ChannelInfo::getChannelName()
{
    return channelName;
}

void ChannelInfo::setDeviceName(const QString& deviceName)
{
    this->deviceName = deviceName;
}

QString ChannelInfo::getDeviceName()
{
    return deviceName;
}

QList<MediaPid> ChannelInfo::queryVideoStreams()
{
    return QList<MediaPid>();
}

QList<MediaPid> ChannelInfo::queryAudioStreams()
{
    return QList<MediaPid>();
}

void ChannelInfo::applySettingsToTuner(WinTuner* /*dvbTuner*/)
{
}

QDataStream& ChannelInfo::save(QDataStream& out) const
{
    return out;
}

QDataStream& ChannelInfo::load(QDataStream& in)
{
    return in;
}

AnalogChannelInfo::AnalogChannelInfo(AnalogTuneRequest req, const QString& channelName, const QString& deviceName)
    : ChannelInfo(channelName, deviceName)
{
    tuneReq = req;
}

AnalogChannelInfo::~AnalogChannelInfo()
{
}

AnalogChannelInfo* AnalogChannelInfo::copy()
{
    return new AnalogChannelInfo(tuneReq, getChannelName(), getDeviceName());
}

int AnalogChannelInfo::ID()
{
    return id;
}

void AnalogChannelInfo::applySettingsToTuner(WinTuner* winTuner)
{
    if (winTuner->getCurrentDeviceName().compare(getDeviceName()) == 0) {
        winTuner->AnalogTune(tuneReq);
    } else {
        winTuner->stopDVB();
        winTuner->startDVB(getDeviceName());
    }
}

QDataStream& AnalogChannelInfo::save(QDataStream& out) const
{
    out << (int)tuneReq.mode;
    out << (int)tuneReq.input;
    out << (int)tuneReq.inputType;
    out << (int)tuneReq.countryCode;
    out << (int)tuneReq.tuningSpace;
    out << (int)tuneReq.channel;
    return out;
}

QDataStream& AnalogChannelInfo::load(QDataStream& in)
{
    int tmp;
    in >> tmp;
    tuneReq.mode = (AMTunerModeType)tmp;
    in >> tmp;
    tuneReq.input = tmp;
    in >> tmp;
    tuneReq.inputType = (TunerInputType)tmp;
    in >> tmp;
    tuneReq.countryCode = tmp;
    in >> tmp;
    tuneReq.tuningSpace = tmp;
    in >> tmp;
    tuneReq.channel = tmp;
    return in;
}

DVBTChannelInfo::DVBTChannelInfo(DVBTTuneRequest tune, const QString& serviceName,
    const QString& serviceProviderName, const QString& deviceName)
    : ChannelInfo(serviceName, deviceName)
{
    tuneReq = tune;
    this->serviceProviderName = serviceProviderName;
}

DVBTChannelInfo::~DVBTChannelInfo()
{
}

DVBTChannelInfo* DVBTChannelInfo::copy()
{
    return new DVBTChannelInfo(tuneReq, getChannelName(), serviceProviderName, getDeviceName());
}

int DVBTChannelInfo::ID()
{
    return id;
}

QList<MediaPid> DVBTChannelInfo::queryVideoStreams()
{
    QList<MediaPid> videoStreams;
    foreach (MediaPid p, tuneReq.stream)
        if (isVideoType(p.type))
            videoStreams.push_back(p);
    return videoStreams;
}

QList<MediaPid> DVBTChannelInfo::queryAudioStreams()
{
    QList<MediaPid> audioStreams;
    foreach (MediaPid p, tuneReq.stream)
        if (isAudioType(p.type))
            audioStreams.push_back(p);
    return audioStreams;
}

void DVBTChannelInfo::applySettingsToTuner(WinTuner* dvbTuner)
{
    if (dvbTuner->getCurrentDeviceName().compare(getDeviceName()) == 0) {
        dvbTuner->DVBTTune(tuneReq);
    } else {
        dvbTuner->stopDVB();
        dvbTuner->startDVB(getDeviceName());
    }
}

QDataStream& DVBTChannelInfo::save(QDataStream& out) const
{
    out << (int)tuneReq.bandwidth;
    out << (int)tuneReq.frequency;
    out << (int)tuneReq.ONID;
    out << (int)tuneReq.SID;
    out << (int)tuneReq.TSID;
    out << tuneReq.stream.size();
    for (int i = 0; i < tuneReq.stream.size(); i++) {
        out << (int)tuneReq.stream[i].pid;
        out << (int)tuneReq.stream[i].type;
    }
    return out;
}

QDataStream& DVBTChannelInfo::load(QDataStream& in)
{
    int tmp;
    in >> tmp;
    tuneReq.bandwidth = tmp;
    in >> tmp;
    tuneReq.frequency = tmp;
    in >> tmp;
    tuneReq.ONID = tmp;
    in >> tmp;
    tuneReq.SID = tmp;
    in >> tmp;
    tuneReq.TSID = tmp;
    in >> tmp;
    for (int i = 0; i < tmp; i++) {
        MediaPid stream;
        int t;
        in >> t;
        stream.pid = t;
        in >> t;
        stream.type = (MediaType)t;
        tuneReq.stream.push_back(stream);
    }
    return in;
}

DVBCChannelInfo::DVBCChannelInfo(const QString& channelName, const QString& deviceName)
    : ChannelInfo(channelName, deviceName)
{
}

DVBCChannelInfo::~DVBCChannelInfo()
{
}

DVBCChannelInfo* DVBCChannelInfo::copy()
{
    return new DVBCChannelInfo(getChannelName(), getDeviceName());
}

int DVBCChannelInfo::ID()
{
    return id;
}

void DVBCChannelInfo::applySettingsToTuner(WinTuner* dvbTuner)
{
    if (dvbTuner->getCurrentDeviceName().compare(getDeviceName()) == 0) {
        //dvbTuner->tuneDVBT();
    } else {
        dvbTuner->stopDVB();
        dvbTuner->startDVB(getDeviceName());
    }
}

QDataStream& DVBCChannelInfo::save(QDataStream& out) const
{
    return out;
}

QDataStream& DVBCChannelInfo::load(QDataStream& in)
{
    return in;
}

DVBSChannelInfo::DVBSChannelInfo(const QString& channelName, const QString& deviceName)
    : ChannelInfo(channelName, deviceName)
{
}

DVBSChannelInfo::~DVBSChannelInfo()
{
}

DVBSChannelInfo* DVBSChannelInfo::copy()
{
    return new DVBSChannelInfo(getChannelName(), getDeviceName());
}

int DVBSChannelInfo::ID()
{
    return id;
}

void DVBSChannelInfo::applySettingsToTuner(WinTuner* dvbTuner)
{
    if (dvbTuner->getCurrentDeviceName().compare(getDeviceName()) == 0) {
        //dvbTuner->tuneDVBT();
    } else {
        dvbTuner->stopDVB();
        dvbTuner->startDVB(getDeviceName());
    }
}

QDataStream& DVBSChannelInfo::save(QDataStream& out) const
{
    return out;
}

QDataStream& DVBSChannelInfo::load(QDataStream& in)
{
    return in;
}

QDataStream& operator<<(QDataStream& out, const ChannelInfo& info)
{
    out << info.channelName;
    out << info.deviceName;
    info.save(out);
    return out;
}

QDataStream& operator>>(QDataStream& in, ChannelInfo& info)
{
    in >> info.channelName;
    in >> info.deviceName;
    info.load(in);
    return in;
}
