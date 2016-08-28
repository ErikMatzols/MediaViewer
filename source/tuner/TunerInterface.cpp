/*
#include "TunerInterface.hpp"
#include <QDataStream>
#include <QFile>

bool TunerInterface::loadSettingsBinary(const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        std::cout << "Failed to load " << fileName.toStdString().c_str() << "\n";     
        return false;
    }
    QDataStream in(&file);

    in.setVersion(QDataStream::Qt_4_6);
    int listSize;
    in >> listSize;
    for(int i = 0; i < listSize; i++) {
        ChannelInfo *info = new ChannelInfo;
        in >> info->device;
        in >> info->mode;
        in >> info->input;
        in >> info->inputType;
        in >> info->country;
        in >> info->tuningSpace;
        in >> info->channel;
        in >> info->channelName;
        mChannelList.append(info);
    }
    file.close();
    return true;
}

bool TunerInterface::saveSettingsBinary(const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly))
        return false;
    QDataStream out(&file);

    out.setVersion(QDataStream::Qt_4_6);
    out << mChannelList.size();
    for(int i = mChannelList.size(); i > 0; i--) {
        ChannelInfo *info = mChannelList.takeFirst();
        out << info->device;
        out << info->mode;
        out << info->input;
        out << info->inputType;
        out << info->country;
        out << info->tuningSpace;
        out << info->channel;
        out << info->channelName;
        delete info;
    }
    file.close();
    return true;
}

QList<ChannelInfo*> TunerInterface::getChannelList()
{
    return mChannelList;
}

QList<ChannelInfo*>* TunerInterface::getChannelListPointer()
{
    return &mChannelList;
}

const ChannelInfo& TunerInterface::nextItem()
{
    mItemIndex++;
    if (mItemIndex >= mChannelList.size())
        mItemIndex = 0;
    return *mChannelList.at(mItemIndex);
}

const ChannelInfo& TunerInterface::findItem(const QString& name)
{
    for(int i = 0; i < mChannelList.count(); i++) {
        if (mChannelList[i]->channelName.compare(name) == 0) {
            mItemIndex = i;
            break;
        }
    }
    return *mChannelList.at(mItemIndex);
}

const ChannelInfo& TunerInterface::findItem(int index)
{
    mItemIndex = index;
    return *mChannelList.at(mItemIndex);
}

const ChannelInfo& TunerInterface::prevItem()
{
    mItemIndex--;
    if (mItemIndex < 0)
        mItemIndex = mChannelList.size()-1;
    return *mChannelList.at(mItemIndex);
}

const ChannelInfo& TunerInterface::currentItem()
{
    return *mChannelList.at(mItemIndex);
}

int TunerInterface::itemCount()
{
    return mChannelList.count();
}

void TunerInterface::clearChannelList()
{
    mChannelList.clear();
}

void TunerInterface::addChannelToList(ChannelInfo *channel)
{
    mChannelList.append(channel);
}

void TunerInterface::setThreadMode(ThreadMode mode)
{
    mMode = mode;
}

ThreadMode TunerInterface::getThreadMode()
{
    return mMode;
}

int TunerInterface::currentChannelIndex()
{
    return mItemIndex;
}

*/
