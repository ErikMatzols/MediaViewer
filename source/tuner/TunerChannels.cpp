#include "TunerChannels.hpp"
#include <QFile>

TunerChannels::TunerChannels()
{
    currentIndex = 0;
}

TunerChannels::~TunerChannels()
{
    foreach (ChannelInfo* channel, channelList)
        delete channel;
}

void TunerChannels::setChannelList(QList<ChannelInfo*> channels)
{
    foreach (ChannelInfo* channel, channelList)
        delete channel;
    channelList.clear();
    channelList = channels;
}

QList<ChannelInfo*> TunerChannels::getChannelList()
{
    return channelList;
}

ChannelInfo* TunerChannels::previousChannel()
{
    currentIndex = currentIndex == 0 ? channelList.size() - 1 : currentIndex - 1;
    return channelList[currentIndex];
}

ChannelInfo* TunerChannels::currentChannel()
{
    return channelList[currentIndex];
}

ChannelInfo* TunerChannels::nextChannel()
{
    currentIndex = (currentIndex + 1) % channelList.size();
    return channelList[currentIndex];
}

ChannelInfo* TunerChannels::channelIndex(int index)
{
    if (index < channelList.size()) {
        currentIndex = index;
        return channelList[index];
    }
    return NULL;
}

int TunerChannels::getCurrentIndex()
{
    return currentIndex;
}

bool TunerChannels::loadSettingsBinary(const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        std::cout << "Failed to load " << fileName.toStdString().c_str() << "\n";
        return false;
    }

    QDataStream in(&file);
    in.setVersion(QDataStream::Qt_4_6);

    int size;
    in >> size;

    for (int i = 0; i < size; i++) {
        ChannelInfo* info = NULL;
        int id;
        in >> id;
        switch (id) {
        case 0:
            info = new ChannelInfo;
            break;
        case 1:
            info = new AnalogChannelInfo;
            break;
        case 2:
            info = new DVBTChannelInfo;
            break;
        case 3:
            info = new DVBCChannelInfo;
            break;
        case 4:
            info = new DVBSChannelInfo;
            break;
        default:
            return false;
        }
        in >> *info;
        channelList.push_back(info);
    }
    file.close();
    return true;
}

bool TunerChannels::saveSettingsBinary(const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly))
        return false;

    QDataStream out(&file);

    out.setVersion(QDataStream::Qt_4_6);

    out << channelList.size();
    QListIterator<ChannelInfo*> i(channelList);
    while (i.hasNext()) {
        ChannelInfo* info = i.next();
        int id = info->ID();
        out << id;
        out << *info;
    }
    file.close();
    return true;
}
