#include "RemoteControl.hpp"

#include "MainWindow.hpp"
#include <QDataStream>
#include <QFile>
#include <iostream>

RemoteControl::RemoteControl(QObject* parent)
    : QObject(parent)
{
    map["NavigateOut"] = RemoteLookup(&RemoteControl::navigateOut, -1);
    map["NavigateDown"] = RemoteLookup(&RemoteControl::navigateDown, -1);
    map["NavigateLeft"] = RemoteLookup(&RemoteControl::navigateLeft, -1);
    map["NavigateRight"] = RemoteLookup(&RemoteControl::navigateRight, -1);
    map["NavigateUp"] = RemoteLookup(&RemoteControl::navigateUp, -1);
    map["NavigateIn"] = RemoteLookup(&RemoteControl::navigateIn, -1);
    map["Toggle FullScreen"] = RemoteLookup(&RemoteControl::toggleFullScreen, -1);
    map["Toggle Mouse Emulation"] = RemoteLookup(&RemoteControl::toggleMouseEmulation, -1);
    map["Toggle Zoom Preset"] = RemoteLookup(&RemoteControl::toggleZoomPreset, -1);
    map["Toggle Keep Aspect"] = RemoteLookup(&RemoteControl::toggleKeepAspect, -1);
    map["Toggle Video Stream"] = RemoteLookup(&RemoteControl::toggleVideoStream, -1);
    map["Toggle Audio Stream"] = RemoteLookup(&RemoteControl::toggleAudioStream, -1);
    map["Toggle Subtitle Stream"] = RemoteLookup(&RemoteControl::toggleSubtitleStream, -1);
    map["Toggle Video Filter Preset"] = RemoteLookup(&RemoteControl::toggleVideoFilter, -1);
    map["Toggle Audio Filter Preset"] = RemoteLookup(&RemoteControl::toggleAudioFilter, -1);
    map["Open Albumview"] = RemoteLookup(&RemoteControl::openAlbumview, -1);
    map["Open Tuner"] = RemoteLookup(&RemoteControl::openTuner, -1);
    map["PlayPause"] = RemoteLookup(&RemoteControl::playPause, -1);
    map["Stop Playback"] = RemoteLookup(&RemoteControl::stop, -1);
    map["SeekBack"] = RemoteLookup(&RemoteControl::seekBack, -1);
    map["SeekForward"] = RemoteLookup(&RemoteControl::seekForward, -1);
    map["SkipBack"] = RemoteLookup(&RemoteControl::skipBack, -1);
    map["SkipForward"] = RemoteLookup(&RemoteControl::skipForward, -1);
    map["VolumeUp"] = RemoteLookup(&RemoteControl::volumeUp, -1);
    map["VolumeDown"] = RemoteLookup(&RemoteControl::volumeDown, -1);
    map["VolumeMute"] = RemoteLookup(&RemoteControl::volumeMute, -1);
    usage = 0;
    usagePage = 0;
    mRegistered = false;
    mUseDevice = false;
}

RemoteControl::~RemoteControl()
{
}

void RemoteControl::handleRemoteId(BYTE* data, int sz)
{
    int id = -1;
    QStringList mask = mInputMask.split(" ");
    if (mask.count() == sz) {
        for (int i = 0; i < sz; i++)
            if (mask[i].compare("FF") == 0)
                id += data[i];
    }
    QMap<QString, RemoteLookup>::const_iterator i = map.constBegin();
    while (i != map.constEnd()) {
        if (id != -1 && i->id == id)
            emit(this->*i->func)();
        ++i;
    }
}

bool RemoteControl::loadSettingsBinary(const QString& fileName, MainWindow* window)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        std::cout << "Failed to load " << fileName.toStdString().c_str() << "\n";
        return false;
    }
    QDataStream in(&file);

    in.setVersion(QDataStream::Qt_4_6);
    in >> usage;
    in >> usagePage;
    in >> mUseDevice;
    in >> mInputMask;
    QMap<QString, RemoteLookup>::iterator i = map.begin();
    while (i != map.end()) {
        in >> i->id;
        ++i;
    }
    file.close();

    if (mUseDevice)
        registerDevice(window, usage, usagePage);

    return true;
}

bool RemoteControl::saveSettingsBinary(const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly))
        return false;
    QDataStream out(&file);

    out.setVersion(QDataStream::Qt_4_6);
    out << usage;
    out << usagePage;
    out << mUseDevice;
    out << mInputMask;
    QMap<QString, RemoteLookup>::const_iterator i = map.constBegin();
    while (i != map.constEnd()) {
        out << i->id;
        ++i;
    }
    file.close();

    return true;
}

QMap<QString, RemoteLookup>& RemoteControl::getMap()
{
    return map;
}

void RemoteControl::registerDevice(MainWindow* window, USHORT usage, USHORT usagePage)
{
    if (mRegistered)
        unregisterDevice(window);

    this->usage = usage;
    this->usagePage = usagePage;
    if (mUseDevice) {
        RAWINPUTDEVICE Rid;
        Rid.dwFlags = RIDEV_INPUTSINK; //0x00;
        Rid.hwndTarget = (HWND)window->winId();
        Rid.usUsage = usage;
        Rid.usUsagePage = usagePage;
        mRegistered = true;
        if (!RegisterRawInputDevices(&Rid, 1, sizeof(RAWINPUTDEVICE)))
            mRegistered = false;
    }
}

void RemoteControl::unregisterDevice(MainWindow* window)
{
    RAWINPUTDEVICE Rid;
    Rid.dwFlags = RIDEV_REMOVE;
    Rid.hwndTarget = (HWND)window->winId();
    Rid.usUsage = usage;
    Rid.usUsagePage = usagePage;
    mRegistered = false;
    RegisterRawInputDevices(&Rid, 1, sizeof(RAWINPUTDEVICE));
}

USHORT RemoteControl::getUsage()
{
    return usage;
}

USHORT RemoteControl::getUsagePage()
{
    return usagePage;
}

bool RemoteControl::deviceUsed()
{
    return mUseDevice;
}

void RemoteControl::setUsed(bool used)
{
    mUseDevice = used;
}
