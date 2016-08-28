#ifndef ALBUMCONTROLLER_HPP
#define ALBUMCONTROLLER_HPP

#include "Defines.hpp"
#include "ThumbnailExtractor.hpp"
#include <QApplication>
#include <QEvent>
#include <QKeyEvent>
#include <QObject>
#include <QTimer>

class AlbumModel;
class Albumlist;
class AlbumEntity;
class MainWindow;
class TcpCommand;
class VideoRenderer;

class AlbumController : public QObject {
    Q_OBJECT

public:
    AlbumController(AlbumModel* model, VideoRenderer* videoRenderer, MainWindow* parent, TcpCommand* client, TcpCommand* server);
    ~AlbumController();

    bool autoStartEnabled();
    void setAutoStartEnabled(bool enabled);

    bool playlistEnabled();
    void setPlaylistEnabled(bool enabled);

    float scrollSpeed();
    void setScrollSpeed(float scrollspeed);

    int thumbnailExtractTime();
    void setThumbnailExtractTime(int time);

    void activate(ConnectionState state);
    void deactivate();

    bool addAlbum(AlbumEntity* album);
    void removeAlbum(int index);
    void removeAllAlbums();

    bool isActivated();

    AlbumModel* getLocalModel();
    void setModel(AlbumModel* newModel);

    VideoRenderer* getRenderer();

    void readMsgServer(RemoteMsg& msg);
    void readMsgClient(RemoteMsg& msg);

    bool loadSettingsBinary(const QString& fileName, Albumlist* albumList);
    bool saveSettingsBinary(const QString& fileName);

signals:
    void openFile(const QString& source, const QString& sourceName, const QString& remoteSub);
    void openWebview(const QString& url);
    void startFileStreamer(const QString& fileName, quint16 port);
    void resizeWindow(int w, int h);

protected:
    void sendMoveInRequest();
    void sendMoveOutRequest();
    void sendAlbumModelRequest();
    void sendAlbumImageRequest();

    void sendAlbumModelReply();
    void sendAlbumFileReply(bool noStream);
    void sendAlbumImageReply();

    bool eventFilter(QObject* obj, QEvent* event);
    void handleKeyEvent(QKeyEvent* event);
    void handleMouseEvent(QMouseEvent* mouseEvent);

public slots:
    void generateThumbnail();
    void generateThumbnailView();
    void generateThumbnailViewSub();

private slots:
    void thumbnailsExtracted();

private:
    AlbumModel* mCurrentModel;
    AlbumModel* mLocalModel;
    AlbumModel* mRemoteModel;
    VideoRenderer* mRenderer;
    MainWindow* mParent;
    QTimer mRefresh;
    bool mActivated;
    int mCurrentEntityImage;
    int mThumbnailExtractTime;
    bool mUsePlaylist;
    bool mAutostart;
    bool mSetArrowCursor;

    ThumbnailExtractor mThumbnailExtractor;

    ConnectionState mState;
    TcpCommand* mTcpClient;
    TcpCommand* mTcpServer;

    int mPrevWidth, mPrevHeight;
};

#endif
