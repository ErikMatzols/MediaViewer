#include "SubParser.hpp"
#include <QFile>
#include <QTextStream>
#include <iostream>

SubParser::SubParser()
{
}

SubParser::~SubParser()
{
}

const SubContainer* SubParser::searchSubtitle(double timeStamp)
{
    for (int i = 0; i < mSubList.size(); ++i) {
        if (mSubList[i].startTime >= timeStamp) {
            mCurrentIndex = i;
            return &mSubList[i];
        }
    }
    return NULL;
}

const SubContainer* SubParser::getNextSubtitle()
{
    if (mCurrentIndex < mSubList.size())
        return &mSubList[mCurrentIndex++];
    return NULL;
}

bool SubParser::openMicroDVDFile(const QString& fileName, float framePeriod)
{
    mSubList.clear();
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        std::cout << "Failed to find " << fileName.toStdString().c_str() << "\n";
        return false;
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        SubContainer sub;
        char c;
        int frameStart, frameStop;
        QStringList strLst;
        in >> c;
        in >> frameStart;
        in >> c;
        in >> c;
        in >> frameStop;
        in >> c;
        strLst = in.readLine().split("|");
        sub.textStr = strLst[0];
        for (int i = 1; i < strLst.size(); i++)
            sub.textStr.append("\r\n" + strLst[i]);
        sub.startTime = frameStart * framePeriod;
        sub.endTime = frameStop * framePeriod;
        mSubList << sub;
    }

    file.close();

    mCurrentIndex = 0;
    return true;
}

bool SubParser::openSubRipFile(QByteArray& byteArray)
{
    QTextStream in(&byteArray);
    while (!in.atEnd()) {
        SubContainer sub;
        sub.subNumber = in.readLine().toInt();
        QString strStartEnd = in.readLine();
        QString line;
        sub.textStr = in.readLine();
        while ((line = in.readLine()) != "")
            sub.textStr.append("\r\n" + line);
        QStringList timeLst = strStartEnd.split(" ");

        QStringList startLst = timeLst[0].split(":");
        int hours = startLst[0][0].digitValue() * 10 + startLst[0][1].digitValue();
        int minutes = startLst[1][0].digitValue() * 10 + startLst[1][1].digitValue();
        QLocale french(QLocale::French);
        double secsAndMs = french.toDouble(startLst[2]);
        sub.startTime = hours * 3600 + minutes * 60 + secsAndMs;

        QStringList endLst = timeLst[2].split(":");
        hours = endLst[0][0].digitValue() * 10 + endLst[0][1].digitValue();
        minutes = endLst[1][0].digitValue() * 10 + endLst[1][1].digitValue();
        secsAndMs = french.toDouble(endLst[2]);
        sub.endTime = hours * 3600 + minutes * 60 + secsAndMs;

        mSubList << sub;
        in.skipWhiteSpace();
    }

    mCurrentIndex = 0;
    return true;
}

bool SubParser::openSubRipFile(const QString& fileName)
{
    mSubList.clear();
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream in(&file);

    while (!in.atEnd()) {
        SubContainer sub;
        sub.subNumber = in.readLine().toInt();
        QString strStartEnd = in.readLine();
        QString line;
        sub.textStr = in.readLine();
        while ((line = in.readLine()) != "")
            sub.textStr.append("\r\n" + line);
        QStringList timeLst = strStartEnd.split(" ");

        QStringList startLst = timeLst[0].split(":");
        int hours = startLst[0][0].digitValue() * 10 + startLst[0][1].digitValue();
        int minutes = startLst[1][0].digitValue() * 10 + startLst[1][1].digitValue();
        QLocale french(QLocale::French);
        double secsAndMs = french.toDouble(startLst[2]);
        sub.startTime = hours * 3600 + minutes * 60 + secsAndMs;

        QStringList endLst = timeLst[2].split(":");
        hours = endLst[0][0].digitValue() * 10 + endLst[0][1].digitValue();
        minutes = endLst[1][0].digitValue() * 10 + endLst[1][1].digitValue();
        secsAndMs = french.toDouble(endLst[2]);
        sub.endTime = hours * 3600 + minutes * 60 + secsAndMs;

        mSubList << sub;
        in.skipWhiteSpace();
    }

    file.close();

    mCurrentIndex = 0;
    return true;
}
