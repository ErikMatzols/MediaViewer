/*
#ifndef TUNERINTERFACE_HPP
#define TUNERINTERFACE_HPP

#include <QThread>
#include <iostream>

enum ThreadMode {RUNGRAPH, AUTOTUNE};

struct ChannelInfo
{
    QString device;
    QString mode;
    QString input;
    QString inputType;
    QString country;
    QString tuningSpace;
    QString channel;
    QString channelName;
};

class TunerInterface : public QThread
{
    Q_OBJECT
public:
    
	TunerInterface(QObject *parent)
        : QThread(parent)
	{
        mItemIndex = 0;
        mMode = RUNGRAPH;
	}
	virtual ~TunerInterface()
	{
        std::cout << "TunerInterface::Destructor() called\n";
	}
	
	virtual void startTuner(enum ThreadMode mode) = 0;
    virtual void pauseTuner(bool) = 0;
	virtual void stopTuner() = 0;
	virtual void setChannel(int channel) = 0;
	virtual int getChannel() = 0;

    virtual void nextChannel() = 0;
    virtual void prevChannel() = 0;
    virtual void gotoChannel(const QString& name) = 0;
    virtual void gotoChannelIndex(int index) = 0;

    virtual int openChannelConfiguration(const QString& graphFile) = 0;
    virtual void closeChannelConfiguration() = 0;

    virtual void getAvailableModes(QStringList& modes) = 0;
    virtual void setMode(const QString& mode) = 0;

    virtual void getAvailableTVFormats(QStringList& TVformats) = 0;
    virtual void setTVFormat(const QString& TVFormat) = 0;

    virtual void getAvailableInputs(QStringList& inputs) = 0;
    virtual void setCurrentInput(const QString& input) = 0;

    virtual void getInputType(QStringList& inputTypes) = 0;
    virtual void setInputType(const QString& inputType) = 0;

    virtual void getCountryCode(QString& code) = 0;
    virtual void setCountryCode(const QString& code) = 0;
    virtual void getTuningSpace(QString& space) = 0;
    virtual void setTuningSpace(const QString& space) = 0;

    virtual void setVolume(int volume) = 0;

    virtual void getChannelMinMax(int& min,int& max) = 0;

    bool loadSettingsBinary(const QString& fileName);
    bool saveSettingsBinary(const QString& fileName);

    QList<ChannelInfo*> getChannelList();
    QList<ChannelInfo*>* getChannelListPointer();

    void clearChannelList();
    void addChannelToList(ChannelInfo *channel);

    int currentChannelIndex();

    void setThreadMode(ThreadMode mode);
    ThreadMode getThreadMode();

signals:
    void channelUpdate(int nr,bool found);
    void autoTuneCompleted();
    void tunerError(const QString&);

protected:
	virtual void run() = 0;
    virtual void autoTune() = 0;
    virtual void runGraph() = 0;

    const ChannelInfo& currentItem();
    const ChannelInfo& nextItem();
    const ChannelInfo& prevItem();
    const ChannelInfo& findItem(const QString& name);
    const ChannelInfo& findItem(int index);
    int itemCount();

private:
    ThreadMode mMode;
    int mItemIndex;
    QList<ChannelInfo*> mChannelList;
};

#endif
*/