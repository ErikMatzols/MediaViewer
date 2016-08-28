#ifndef THUMBNAILEXTRACTOR_HPP
#define THUMBNAILEXTRACTOR_HPP

#include "StreamState.hpp"
#include <QFileInfo>
#include <QStringList>
#include <QThread>

class VideoRenderer;

struct RGBData {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

class ThumbnailExtractor : public QThread {
    Q_OBJECT

public:
    ThumbnailExtractor(QObject* parent, VideoRenderer* renderer);
    ~ThumbnailExtractor();

    void extractThumbnail(QString file, int timestamp);
    void extractThumbnails(QStringList list, int timestamp);

signals:
    void thumbnailsExtracted();

protected:
    void run();

private:
    QStringList mFileList;
    QString mImageName;
    VideoRenderer* mRenderer;
    int mTimestamp;
};

#endif
