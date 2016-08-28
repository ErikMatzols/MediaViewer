#ifndef CHANNELINFO_HPP
#define CHANNELINFO_HPP

//#include <atlbase.h>
#include <tuner.h>
#include <DShow.h>
#include <iostream>
#include <Windows.h>
#include "Settings.hpp"
#include <QStringList>
#include "DumpFilter.hpp"
#include <Mpeg2data.h>
#include <Bdatif.h>
#include <QList>
#include "Mediatypes.hpp"

class WinTuner;
class ChannelInfo;

struct MediaPid
{
    MediaPid(ULONG p, MediaType t)
    {
        pid = p;
        type = t;
    }
    
    MediaPid()
    {
        pid = 0x00;
        type = UNKNOWN;
    }

    ULONG pid;
    MediaType type;
};

struct TuneRequestInfo
{
    virtual QList<ChannelInfo*> tune(WinTuner *winTuner) = 0;
};

struct AnalogTuneRequest : public TuneRequestInfo
{
    QList<ChannelInfo*> tune(WinTuner *winTuner);

    AMTunerModeType mode;
    TunerInputType inputType;
    long input;
    long countryCode;
    long tuningSpace;
    long channel;
};

struct DVBTTuneRequest : public TuneRequestInfo
{
    QList<ChannelInfo*> tune(WinTuner *winTuner);

    long frequency;
    long bandwidth;
    long ONID;
    long SID;
    long TSID;
    QList<MediaPid> stream;
};

struct DVBCTuneRequest : public TuneRequestInfo
{
    QList<ChannelInfo*> tune(WinTuner *winTuner);
};

struct DVBSTuneRequest : public TuneRequestInfo
{
    QList<ChannelInfo*> tune(WinTuner *winTuner);
};

class ChannelInfo
{
public:
    ChannelInfo();
    ChannelInfo(const QString& channelName, const QString& deviceName);
    virtual ~ChannelInfo();

    virtual ChannelInfo* copy();
    virtual int ID();

    void setChannelName(const QString& channelName);
    QString getChannelName();

    void setDeviceName(const QString &deviceName);
    QString getDeviceName();

    virtual QList<MediaPid> queryVideoStreams();
    virtual QList<MediaPid> queryAudioStreams();

    virtual void applySettingsToTuner(WinTuner *dvbTuner);

    friend QDataStream& operator << (QDataStream &out, const ChannelInfo &info);
    friend QDataStream& operator >> (QDataStream &in, ChannelInfo &info);

protected:
    virtual QDataStream& save(QDataStream &out) const;
    virtual QDataStream& load(QDataStream &in);

private:
    static const int id = 0;
    QString channelName;
    QString deviceName;
};

class AnalogChannelInfo : public ChannelInfo
{
public:
    AnalogChannelInfo(){}
    AnalogChannelInfo(AnalogTuneRequest req, const QString &channelName, const QString &deviceName);
    ~AnalogChannelInfo();

    AnalogChannelInfo* copy();
    int ID();
    void applySettingsToTuner(WinTuner *winTuner);

protected:
     QDataStream& save(QDataStream &out) const;
     QDataStream& load(QDataStream &in);

private:
    static const int id = 1;
    AnalogTuneRequest tuneReq;
};


class DVBTChannelInfo : public ChannelInfo
{
public:
    DVBTChannelInfo(){}
    DVBTChannelInfo(DVBTTuneRequest tune, const QString &channelName,
                    const QString &serviceProviderName, const QString &deviceName);
    ~DVBTChannelInfo();

    DVBTChannelInfo* copy();
    int ID();

    QList<MediaPid> queryVideoStreams();
    QList<MediaPid> queryAudioStreams();

    void applySettingsToTuner(WinTuner *dvbTuner);

protected:
     QDataStream& save(QDataStream &out) const;
     QDataStream& load(QDataStream &in);

private:
    static const int id = 2;
    DVBTTuneRequest tuneReq;
    QString serviceProviderName;
};

class DVBCChannelInfo : public ChannelInfo
{
public:
    DVBCChannelInfo(){}
    DVBCChannelInfo(const QString &channelName, const QString &deviceName);
    ~DVBCChannelInfo();

    DVBCChannelInfo* copy();
    int ID();
    void applySettingsToTuner(WinTuner *dvbTuner);

protected:
     QDataStream& save(QDataStream &out) const;
     QDataStream& load(QDataStream &in);

private:
    static const int id = 3;
    DVBCTuneRequest tuneReq;
};

class DVBSChannelInfo : public ChannelInfo
{
public:
    DVBSChannelInfo(){}
    DVBSChannelInfo(const QString &channelName, const QString &deviceName);
    ~DVBSChannelInfo();

    DVBSChannelInfo* copy();
    int ID();
    void applySettingsToTuner(WinTuner *dvbTuner);

protected:
     QDataStream& save(QDataStream &out) const;
     QDataStream& load(QDataStream &in);

private:
    static const int id = 4;
    DVBSTuneRequest tuneReq;  
};

#endif
