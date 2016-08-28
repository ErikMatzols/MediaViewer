#ifndef TUNERCHANNELS_HPP
#define TUNERCHANNELS_HPP

#include <QWidget>
//#include "WinTuner.hpp"
#include "ChannelInfo.hpp"

class TunerChannels: public QWidget
{
public:
    TunerChannels();
    ~TunerChannels();

    void setChannelList(QList<ChannelInfo*> channels);
    QList<ChannelInfo*> getChannelList();

    ChannelInfo* previousChannel();
    ChannelInfo* currentChannel();
    ChannelInfo* nextChannel();

    ChannelInfo* channelIndex(int index);
    int getCurrentIndex();
    bool loadSettingsBinary(const QString& fileName);
    bool saveSettingsBinary(const QString& fileName);

protected:

private:
    unsigned int currentIndex;
    QList<ChannelInfo*> channelList;
};

#endif
