#ifndef SETTINGS_HPP

#include <QMap>
#include <QString>

typedef QMap<QString, QString> SettingsMap;
typedef QMap<QString, SettingsMap> SettingsGroupMap;

class Settings {
public:
    Settings();
    ~Settings();

    bool loadFile(const QString& fileName);
    bool saveFile();
    SettingsGroupMap retreiveMap();
    QList<SettingsMap> retreiveArray();
    const SettingsMap* retreiveGroup(const QString& group);
    bool retreiveStringValue(const QString& group, const QString& key, QString& value);
    bool retreiveIntValue(const QString& group, const QString& key, int& value);
    bool saveValue(const QString& group, const QString& key, const QString& value);

protected:
private:
    QString fileName;
    QList<SettingsMap> settingsArray;
    SettingsGroupMap settingsGroupMap;
};

#endif
