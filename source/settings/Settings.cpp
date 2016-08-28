#include "Settings.hpp"
#include <QFile>
#include <QStringList>
#include <QTextStream>

Settings::Settings()
{
}

Settings::~Settings()
{
}

bool Settings::loadFile(const QString& fileName)
{
    this->fileName = fileName;
    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    QTextStream in(&file);
    QString line = in.readLine();
    while (!in.atEnd()) {
        if (line[0] == '[') {
            SettingsMap settings;
            QString key = line;
            do {
                line = in.readLine();
                QStringList keyVal = line.split("=");
                settings.insert(keyVal.first(), keyVal.last());
            } while (!in.atEnd() && line[0] != '[');
            settingsArray.push_back(settings);
            settingsGroupMap.insert(key, settings);
        }
    }

    file.close();
    return true;
}

bool Settings::saveFile()
{

    return true;
}

bool Settings::retreiveStringValue(const QString& group, const QString& key, QString& value)
{
    SettingsGroupMap::const_iterator group_iter = settingsGroupMap.constFind(group);
    if (group_iter != settingsGroupMap.end()) {
        SettingsMap::const_iterator settings_iter = group_iter.value().constFind(key);
        if (settings_iter != group_iter.value().end()) {
            value = settings_iter.value();
            return true;
        }
    }
    return false;
}

bool Settings::retreiveIntValue(const QString& group, const QString& key, int& value)
{
    SettingsGroupMap::const_iterator group_iter = settingsGroupMap.constFind(group);
    if (group_iter != settingsGroupMap.end()) {
        SettingsMap::const_iterator settings_iter = group_iter.value().constFind(key);
        if (settings_iter != group_iter.value().end()) {
            value = settings_iter.value().toInt();
            return true;
        }
    }
    return false;
}

const SettingsMap* Settings::retreiveGroup(const QString& group)
{
    SettingsGroupMap::const_iterator group_iter = settingsGroupMap.constFind(group);
    if (group_iter != settingsGroupMap.end())
        return &group_iter.value();
    return NULL;
}

bool Settings::saveValue(const QString& group, const QString& key, const QString& value)
{
    SettingsGroupMap::iterator group_iter = settingsGroupMap.find(group);
    if (group_iter != settingsGroupMap.end()) {
        group_iter.value().insert(key, value);
        return true;
    }
    return false;
}

SettingsGroupMap Settings::retreiveMap()
{
    return settingsGroupMap;
}

QList<SettingsMap> Settings::retreiveArray()
{
    return settingsArray;
}
