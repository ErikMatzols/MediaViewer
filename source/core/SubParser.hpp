#ifndef SUBPARSER_HPP
#define SUBPARSER_HPP

#include <QString>
#include <QStringList>

struct SubContainer {
    int subNumber;
    double startTime;
    double endTime;
    QString textStr;
};

class SubParser {
public:
    SubParser();
    ~SubParser();

    bool openMicroDVDFile(const QString& fileName, float framePeriod);
    bool openSubRipFile(const QString& fileName);
    bool openSubRipFile(QByteArray& byteArray);
    const SubContainer* searchSubtitle(double timeStamp);
    const SubContainer* getNextSubtitle();

protected:
private:
    QList<SubContainer> mSubList;
    int mCurrentIndex;
};

#endif
