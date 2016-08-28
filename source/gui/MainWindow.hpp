#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include "CustomWindow.hpp"

#include "AudioRenderer.hpp"
#include "Defines.hpp"
#include "Demuxer.hpp"
#include "TunerChannels.hpp"
#include "VideoRenderer.hpp"
#include <QMenuBar>
#include <QSizeGrip>
#include <QSystemTrayIcon>
#include <QTime>
#include <QVector4D>

class QAction;
class QActionGroup;
class QMenu;
class QVBoxLayout;
class AlbumController;
class Albumlist;
class ColorControlDialog;
class ControllerWidget;
class CustomButton;
class FileStreamer;
class ListenServerDialog;
class Log;
class MenuWidget;
class RemoteControl;
class TcpCommand;
class Playlist;
class OptionsDialog;
class WebView;
class WinTuner;

class MainWindow : public CustomWindow {
    Q_OBJECT

public:
    MainWindow();
    ~MainWindow();

    void setFileFilter(const QString& fileFilter);
    QString fileFilter();

    static QString loadStyleSheet(const QString& fileName);
    void minimizeToTray();

    bool loadSettingsBinary(const QString& fileName);
    bool saveSettingsBinary(const QString& fileName);

    QString dataLocation();
    QString tempLocation();

    void resetCursorHidden();

    void changeVolume(int volume);
    void changeChannel(int index);

    bool mouseEmulation();

    WinTuner* getTuner()
    {
        return mTuner;
    }

public slots:
    void openFile(const QString& source, const QString& sourceName, const QString& remoteSub);
    void openFile(const QString& fileName);
    void openWebView(const QString& url);
    void skipBack();
    void seekBack();
    void playPause();
    void stop();
    void seekForward();
    void skipForward();

    void seek(int time);

    void checkCursorHidden();
    void demuxerFinished();
    void resizeWindow(int w, int h, bool overrideCheck = false);

    void startFileStreamer(const QString& fileName, quint16 port);

    void registerVideoStreams(QList<StreamDescriptor> videoStreams);
    void registerAudioStreams(QList<StreamDescriptor> audioStreams);
    void registerSubtitleStreams(QList<StreamDescriptor> subtitleStreams);

    void addVideoFilterPreset(QStringList filterList, const QString& name, bool setChecked);
    void editVideoFilterPreset(int index, QStringList filterList, const QString& name, bool setChecked);
    void deleteVideoFilterPreset(int index);
    QStringList getVideoFilterPreset(int index);

    void addAudioFilterPreset(QStringList filterList, const QString& name, bool setChecked);
    void editAudioFilterPreset(int index, QStringList filterList, const QString& name, bool setChecked);
    void deleteAudioFilterPreset(int index);
    QStringList getAudioFilterPreset(int index);

    void addStretchPreset(QVector4D preset, const QString& name, bool setChecked);
    void editStretchPreset(int index, QVector4D preset, const QString& name);
    void deleteStretchPreset(int index);
    QVector4D getStretchPreset(int index);
    void displayErrorMessage(const QString& msg);

protected:
    void setupUI();
    void showListenServerDialog();
    void resizeEvent(QResizeEvent* event);
    void closeEvent(QCloseEvent* event);
    bool winEvent(MSG* message, long* result);
    bool eventFilter(QObject* object, QEvent* event);
    void dragEnterEvent(QDragEnterEvent* event);
    void dropEvent(QDropEvent* event);

private slots:
    void enableStreamControls();
    void disableStreamControls();
    void openFileDialog();
    void openUrlDialog();
    void openAlbumview();
    void openTuner();
    void closeAlbumview();
    void closeWebview();
    void exitProgram();

    void controlVideoMenuPressed(QAction*);
    void controlAudioMenuPressed(QAction*);
    void controlSubtitleMenuPressed(QAction*);

    void startListenServer();
    void connectToServer();
    void disconnectFromServer();

    void clientReceivedMessage(RemoteMsg& msg);
    void serverReceivedMessage(RemoteMsg& msg);

    void showPlaylist();
    void showAlbumlist();

    void showStretchPresetDialog();
    void changeStretchPreset();
    void resizeOnPlayToggle();
    void toggleKeepAspect();
    void toggleFullScreen();

    void showVideoPresetDialog();
    void changeVideoFilterPreset();
    void changeAudioFilterPreset();
    void showColorControl();

    void showLog();
    void showOptions();

    void help();
    void version();
    void about();

    void popupContextMenu(const QPoint& pos);
    void trayActivated(QSystemTrayIcon::ActivationReason reason);
    void channelChangeTimeout();

    void toggleMouseEmulation();
    void toggleDebug();

    void toggleVideoStream();
    void toggleAudioStream();
    void toggleSubtitleStream();

    void toggleVideoFilter();
    void toggleAudioFilter();
    void toggleZoomPreset();
    void tunerStarted();

    void emulateNavigateIn();
    void emulateNavigateOut();
    void emulateNavigateLeft();
    void emulateNavigateRight();
    void emulateNavigateUp();
    void emulateNavigateDown();

    void graphFilterMenuPressed(QAction*);

    void tunerChannelChanged();

private:
    QSystemTrayIcon* mTrayIcon;
    ControllerWidget* mControllerWidget;
    QVBoxLayout* vLayout;
    QFrame* mVideoFrame;
    QString mFrameStyleNormal;
    QString mFrameStyleFullScreen;
    //WebView *mWebView;
    VideoRenderer* mVideoRenderer;
    AudioRenderer* mAudioRenderer;
    Demuxer* mDemuxer;

    TcpCommand* mClient;
    TcpCommand* mServer;
    FileStreamer* mFileStreamer;
    ListenServerDialog* mListenServerDialog;
    AlbumController* mAlbumController;
    Albumlist* mAlbumlist;
    Playlist* mPlaylist;
    RemoteControl* mRemoteControl;

    WinTuner* mTuner;
    TunerChannels tunerChannels;

    ColorControlDialog* mColorControl;

    OptionsDialog* mOptionsDialog;
    Log* mLog;

    QString mDataLocation;
    QString mTempLocation;
    QString mFileFilter;
    QString mPrevFileDialogLoc;
    ConnectionState mConnectionState;
    bool mContextMenuActive;
    bool mCursorHidden;
    QTime mCurrentTime;
    int mCursorLastShown;
    QList<int> mChannelChange;
    QTimer* mChannelChangeTimer;

    // Media Menu Actions
    QAction* mActionOpenFile;
    QAction* mActionOpenLocation;
    QAction* mActionOpenAlbumView;
    QAction* mActionOpenTuner;
    QAction* mActionCloseAlbumview;
    QAction* mActionExit;

    // Control Menu Actions
    QMenu* mControlVideoMenu;
    QMenu* mControlAudioMenu;
    QMenu* mControlSubtitleMenu;
    QActionGroup* mActionGroupVideo;
    QActionGroup* mActionGroupAudio;
    QActionGroup* mActionGroupSubtitle;
    QAction* mActionSkipBack;
    QAction* mActionSeekBack;
    QAction* mActionPlayPause;
    QAction* mActionStop;
    QAction* mActionSeekForward;
    QAction* mActionSkipForward;

    // Server Menu Actions
    QAction* mActionListen;
    QAction* mActionConnect;
    QAction* mActionDisconnect;

    // View Menu Actions
    QAction* mActionShowPlaylist;
    QAction* mActionShowAlbumlist;
    QMenu* mStrechMenu;
    QAction* mActionStretchPresetEdit;
    QActionGroup* mActionGroupStretch;
    QAction* mActionColorControl;
    QAction* mActionResizeOnPlay;
    QAction* mActionKeepAspect;
    QAction* mActionToggleFullscreen;
    QAction* mActionToggleDebugText;

    // Filer Menu Action
    QMenu* mVideoFilterMenu;
    QMenu* mAudioFilterMenu;
    QAction* mActionVideoPresetEdit;
    QAction* mActionAudioPresetEdit;
    QActionGroup* mActionGroupVideoFilter;
    QActionGroup* mActionGroupAudioFilter;
    QMenu* mGraphFilterMenu;

    // Tools Menu Actions
    QAction* mActionGenerateThumbnail;
    QAction* mActionGenerateThumbnailView;
    QAction* mActionGenerateThumbnailViewSub;
    QAction* mActionShowLog;
    QAction* mActionOptions;

    // Help Menu Acions
    QAction* mActionMediaviewerHelp;
    QAction* mActionCheckForUpdates;
    QAction* mActionAboutMediaViewer;

    // Misc
    QMenu* mPopupMenu;
};

#endif
