#ifndef REMOTECONTROL_HPP
#define REMOTECONTROL_HPP

#include <QMap>
#include <QObject>
#include <QString>
#include <windows.h>

class MainWindow;
class RemoteControl;

struct RemoteLookup {
    RemoteLookup() {}
    RemoteLookup(void (RemoteControl::*func)(void), int id)
    {
        this->func = func;
        this->id = id;
    }

    void (RemoteControl::*func)(void);
    int id;
};

class RemoteControl : public QObject {
    Q_OBJECT
public:
    RemoteControl(QObject* parent);
    ~RemoteControl();

    void registerDevice(MainWindow* window, USHORT usage, USHORT usagePage);
    void unregisterDevice(MainWindow* window);
    void handleRemoteId(BYTE* data, int sz);

    USHORT getUsage();
    USHORT getUsagePage();

    bool deviceUsed();
    void setUsed(bool used);

    QMap<QString, RemoteLookup>& getMap();

    bool loadSettingsBinary(const QString& fileName, MainWindow* window);
    bool saveSettingsBinary(const QString& fileName);

    void setInputMask(QString inputMask)
    {
        mInputMask = inputMask;
    }

    QString getInputMask()
    {
        return mInputMask;
    }

signals:
    void navigateOut();
    void navigateUp();
    void navigateDown();
    void navigateLeft();
    void navigateRight();
    void navigateIn();
    void toggleFullScreen();

    void openTuner();
    void openAlbumview();

    void toggleMouseEmulation();
    void toggleZoomPreset();
    void toggleKeepAspect();
    void toggleVideoStream();
    void toggleAudioStream();
    void toggleSubtitleStream();
    void toggleVideoFilter();
    void toggleAudioFilter();

    void playPause();
    void stop();
    void seekBack();
    void seekForward();
    void skipBack();
    void skipForward();

    void volumeUp();
    void volumeDown();
    void volumeMute();

protected:
private:
    QString sDeviceName;
    QString mInputMask;
    USHORT usage;
    USHORT usagePage;
    bool mRegistered;
    bool mUseDevice;
    QMap<QString, RemoteLookup> map;
};

#endif
