#include "MainWindow.hpp"
#include "AlbumController.hpp"
#include "AlbumModel.hpp"
#include "Albumlist.hpp"
#include "ColorControlDialog.hpp"
#include "ConnectServerDialog.hpp"
#include "ControllerWidget.hpp"
#include "CustomButton.hpp"
#include "Defines.hpp"
#include "FileStreamer.hpp"
#include "FilterPresetDialog.hpp"
#include "ListenServerDialog.hpp"
#include "Log.hpp"
#include "MenuWidget.hpp"
#include "OptionsDialog.hpp"
#include "Playlist.hpp"
#include "RemoteControl.hpp"
#include "StretchPresetDialog.hpp"
#include "TcpCommand.hpp"
#include "UrlDialog.hpp"
#include "WebView.hpp"
#include "WinTvTuner.hpp"
#include <QDesktopServices>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QMouseEvent>
#include <QStandardPaths>
#include <QTextStream>
#include <QUrl>
#include <QVBoxLayout>
#include <QWebEngineSettings>

HHOOK hookHandle;
bool mMouseEmulation = false;

MainWindow::MainWindow()
    : CustomWindow(0)
{
    qRegisterMetaType<QList<StreamDescriptor> >("QList<StreamDescriptor>");
    qRegisterMetaType<QList<ChannelInfo*> >("QList<ChannelInfo*>");
    mFileFilter = "*.avi *.mkv *.mov *.wmv *.mp3 *.mp4 *.web";

    mConnectionState = CONNECTION_LOCAL;
    mMouseEmulation = false;

    mDataLocation = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/MediaViewer";
    QFileInfo infoLocation(mDataLocation);
    if (!infoLocation.isDir())
        if (!QDir(mDataLocation).mkpath(mDataLocation))
            std::cout << "Error creating application directory\n";

    mTempLocation = QStandardPaths::writableLocation(QStandardPaths::TempLocation);

// mLog = &Log::Instance();

// mWebView = new WebView(this);
// mWebView->installEventFilter(this);
#ifndef NO_WEBVIEW_PLUGINS
// mWebView->settings()->setAttribute(QWebEngineSettings::PluginsEnabled, true);
#endif
    // mWebView->setVisible(false);

    mVideoRenderer = new VideoRenderer(this);
    mVideoRenderer->setMouseTracking(true);
    mVideoRenderer->installEventFilter(this);

    mAudioRenderer = new AudioRenderer(this);
    mDemuxer = new Demuxer(this, mVideoRenderer, mAudioRenderer);
    mVideoRenderer->setDemuxer(mDemuxer);
    mTuner = new WinTuner(this, mVideoRenderer);

    connect(mTuner, SIGNAL(graphRunning()), this, SLOT(tunerStarted()));
    connect(mTuner, SIGNAL(tuneRequestFinished()), this, SLOT(tunerChannelChanged()));

    mControllerWidget = new ControllerWidget(this);
    mControllerWidget->setMode(DEMUXER_CONTROL);

    mClient = new TcpCommand(this);
    mServer = new TcpCommand(this);
    mFileStreamer = NULL;

    mListenServerDialog = new ListenServerDialog(this);
#ifndef NO_IMAGE_ALLOC
    mTrayIcon = new QSystemTrayIcon(QIcon("../resources/Images/title24.png"), this);
#else
    mTrayIcon = new QSystemTrayIcon(this);
#endif
    mTrayIcon->setToolTip("MediaViewer Listen Server");

    AlbumModel* model = new AlbumModel;

    mAlbumController = new AlbumController(model, mVideoRenderer, this, mClient, mServer);

    mAlbumlist = new Albumlist(0, mAlbumController, mFileFilter);

    mPlaylist = new Playlist(0, mFileFilter);
    mRemoteControl = new RemoteControl(this);

    mColorControl = new ColorControlDialog(mVideoRenderer);

    mOptionsDialog = new OptionsDialog(this, mRemoteControl->getMap().size());

    mChannelChangeTimer = new QTimer(this);
    connect(mChannelChangeTimer, SIGNAL(timeout()), this,
        SLOT(channelChangeTimeout()));

    setupUI();

    mVideoRenderer->setFocus(Qt::ActiveWindowFocusReason);
    loadSettingsBinary(mDataLocation + "/General.dat");
    mAlbumController->loadSettingsBinary(mDataLocation + "/Albumview.dat",
        mAlbumlist);
    mVideoRenderer->loadSettingsBinary(mDataLocation + "/Video.dat");
    mAudioRenderer->loadSettingsBinary(mDataLocation + "/Audio.dat");
    tunerChannels.loadSettingsBinary(mDataLocation + "/Tuner.dat");
    mRemoteControl->loadSettingsBinary(mDataLocation + "/Remote.dat", this);

    connect(mVideoRenderer, SIGNAL(resizeWindow(int, int)), this,
        SLOT(resizeWindow(int, int)));

    connect(mDemuxer, SIGNAL(registerStreamDuration(int)), mControllerWidget,
        SLOT(registerDuration(int)));
    connect(mDemuxer, SIGNAL(registerVideoStreams(QList<StreamDescriptor>)), this,
        SLOT(registerVideoStreams(QList<StreamDescriptor>)));
    connect(mDemuxer, SIGNAL(registerAudioStreams(QList<StreamDescriptor>)), this,
        SLOT(registerAudioStreams(QList<StreamDescriptor>)));
    connect(mDemuxer, SIGNAL(registerSubtitleStreams(QList<StreamDescriptor>)),
        this, SLOT(registerSubtitleStreams(QList<StreamDescriptor>)));
    connect(mDemuxer, SIGNAL(progressUpdate(double)), mControllerWidget,
        SLOT(progressUpdate(double)));
    connect(mDemuxer, SIGNAL(demuxerFinished()), this, SLOT(demuxerFinished()));

    connect(mClient, SIGNAL(messageReceived(RemoteMsg&)), this,
        SLOT(clientReceivedMessage(RemoteMsg&)));
    connect(mClient, SIGNAL(tcpError(const QString&)), this,
        SLOT(displayErrorMessage(const QString&)));
    connect(mServer, SIGNAL(messageReceived(RemoteMsg&)), this,
        SLOT(serverReceivedMessage(RemoteMsg&)));

    connect(mAlbumController,
        SIGNAL(openFile(const QString&, const QString&, const QString&)),
        this, SLOT(openFile(const QString&, const QString&, const QString&)));
    connect(mAlbumController, SIGNAL(openWebview(const QString&)), this,
        SLOT(openWebView(const QString&)));
    connect(mAlbumController, SIGNAL(startFileStreamer(const QString&, quint16)),
        this, SLOT(startFileStreamer(const QString&, quint16)));
    connect(mActionGenerateThumbnail, SIGNAL(triggered()), mAlbumController,
        SLOT(generateThumbnail()));
    connect(mActionGenerateThumbnailView, SIGNAL(triggered()), mAlbumController,
        SLOT(generateThumbnailView()));
    connect(mActionGenerateThumbnailViewSub, SIGNAL(triggered()),
        mAlbumController, SLOT(generateThumbnailViewSub()));

    connect(mRemoteControl, SIGNAL(navigateOut()), this,
        SLOT(emulateNavigateOut()));
    connect(mRemoteControl, SIGNAL(navigateUp()), this,
        SLOT(emulateNavigateUp()));
    connect(mRemoteControl, SIGNAL(navigateDown()), this,
        SLOT(emulateNavigateDown()));
    connect(mRemoteControl, SIGNAL(navigateLeft()), this,
        SLOT(emulateNavigateLeft()));
    connect(mRemoteControl, SIGNAL(navigateRight()), this,
        SLOT(emulateNavigateRight()));
    connect(mRemoteControl, SIGNAL(navigateIn()), this,
        SLOT(emulateNavigateIn()));
    connect(mRemoteControl, SIGNAL(toggleFullScreen()), this,
        SLOT(toggleFullScreen()));
    connect(mRemoteControl, SIGNAL(openTuner()), this, SLOT(openTuner()));
    connect(mRemoteControl, SIGNAL(openAlbumview()), this, SLOT(openAlbumview()));
    connect(mRemoteControl, SIGNAL(toggleMouseEmulation()), this,
        SLOT(toggleMouseEmulation()));
    connect(mRemoteControl, SIGNAL(toggleZoomPreset()), this,
        SLOT(toggleZoomPreset()));
    connect(mRemoteControl, SIGNAL(toggleKeepAspect()), this,
        SLOT(toggleKeepAspect()));
    connect(mRemoteControl, SIGNAL(toggleVideoStream()), this,
        SLOT(toggleVideoStream()));
    connect(mRemoteControl, SIGNAL(toggleAudioStream()), this,
        SLOT(toggleAudioStream()));
    connect(mRemoteControl, SIGNAL(toggleSubtitleStream()), this,
        SLOT(toggleSubtitleStream()));
    connect(mRemoteControl, SIGNAL(toggleVideoFilter()), this,
        SLOT(toggleVideoFilter()));
    connect(mRemoteControl, SIGNAL(toggleAudioFilter()), this,
        SLOT(toggleAudioFilter()));
    connect(mRemoteControl, SIGNAL(playPause()), this, SLOT(playPause()));
    connect(mRemoteControl, SIGNAL(stop()), this, SLOT(stop()));
    connect(mRemoteControl, SIGNAL(seekBack()), this, SLOT(seekBack()));
    connect(mRemoteControl, SIGNAL(seekForward()), this, SLOT(seekForward()));
    connect(mRemoteControl, SIGNAL(skipBack()), this, SLOT(skipBack()));
    connect(mRemoteControl, SIGNAL(skipForward()), this, SLOT(skipForward()));
    connect(mRemoteControl, SIGNAL(volumeMute()), mControllerWidget,
        SLOT(volumePressed()));
    connect(mRemoteControl, SIGNAL(volumeUp()), mControllerWidget,
        SLOT(volumeUp()));
    connect(mRemoteControl, SIGNAL(volumeDown()), mControllerWidget,
        SLOT(volumeDown()));

    connect(mTrayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this,
        SLOT(trayActivated(QSystemTrayIcon::ActivationReason)));
    connect(mPlaylist, SIGNAL(openFile(const QString&)), this,
        SLOT(openFile(const QString&)));

    setWindowTitle("MediaViewer");
#ifndef NO_IMAGE_ALLOC
    setIcon("../resources/Images/title16.png");
    setWindowIcon(QIcon("../resources/Images/title16.png"));
#endif
    setAcceptDrops(true);

    show();
    resizeWindow(0, 160, true);
}

MainWindow::~MainWindow()
{
    delete mTrayIcon;
    delete mAlbumlist;
    delete mPlaylist;
    delete mColorControl;
}

QString
MainWindow::dataLocation()
{
    return mDataLocation;
}

QString
MainWindow::tempLocation()
{
    return mTempLocation;
}

void MainWindow::setFileFilter(const QString& fileFilter)
{
    mFileFilter = fileFilter;
}

QString
MainWindow::fileFilter()
{
    return mFileFilter;
}

QString
MainWindow::loadStyleSheet(const QString& fileName)
{
    QFile file(fileName);
    QString str;

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return str;
    QTextStream in(&file);
    str = in.readAll();
    file.close();

    return str;
}

void MainWindow::minimizeToTray()
{
    mTrayIcon->show();
    mListenServerDialog->hide();
    hide();
}

bool MainWindow::loadSettingsBinary(const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        std::cout << "Failed to load " << fileName.toStdString().c_str() << "\n";
        return false;
    }
    QDataStream in(&file);

    bool temp;
    in.setVersion(QDataStream::Qt_4_6);
    in >> mFileFilter;
    in >> temp;
    mActionResizeOnPlay->setChecked(temp);
    mVideoRenderer->resizeOnPlay(temp);
    in >> temp;
    mActionKeepAspect->setChecked(temp);
    mVideoRenderer->keepAspectRatio(temp);

    int size;
    in >> size;
    for (int i = 1; i < size; i++) {
        QString text;
        QVector4D preset;
        in >> text;
        in >> preset;
        addStretchPreset(preset, text, false);
    }
    in >> size;
    for (int i = 1; i < size; i++) {
        QString text;
        QStringList filterList;
        in >> text;
        in >> filterList;
        addVideoFilterPreset(filterList, text, false);
    }
    in >> size;
    for (int i = 1; i < size; i++) {
        QString text;
        QStringList filterList;
        in >> text;
        in >> filterList;
        addAudioFilterPreset(filterList, text, false);
    }
    file.close();
    return true;
}

bool MainWindow::saveSettingsBinary(const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly))
        return false;
    QDataStream out(&file);

    out.setVersion(QDataStream::Qt_4_6);
    out << mFileFilter;
    out << mActionResizeOnPlay->isChecked();
    out << mActionKeepAspect->isChecked();

    QList<QAction*> actionList = mActionGroupStretch->actions();
    out << actionList.size();
    for (int i = 1; i < actionList.size(); i++) {
        out << actionList[i]->text();
        out << actionList[i]->data().value<QVector4D>();
    }

    actionList = mActionGroupVideoFilter->actions();
    out << actionList.size();
    for (int i = 1; i < actionList.size(); i++) {
        out << actionList[i]->text();
        out << actionList[i]->data().toStringList();
    }

    actionList = mActionGroupAudioFilter->actions();
    out << actionList.size();
    for (int i = 1; i < actionList.size(); i++) {
        out << actionList[i]->text();
        out << actionList[i]->data().toStringList();
    }

    file.close();
    return true;
}

void MainWindow::resetCursorHidden()
{
    mCurrentTime.restart();
    mCursorHidden = false;
    mCursorLastShown = 0;
}

void MainWindow::checkCursorHidden()
{
    int elapsed = mCurrentTime.elapsed();
    if (elapsed - mCursorLastShown >= 3000) {
        if (isFullScreen() && !mCursorHidden && !mContextMenuActive) {
            mVideoRenderer->setCursor(Qt::BlankCursor);
            mCursorHidden = true;
            mCursorLastShown = elapsed;
        }
    }
}

void MainWindow::changeVolume(int volume)
{
    if (mDemuxer->isRunning()) {
        float gain = volume / 100.0f;
        mAudioRenderer->adjustGain(gain);
    }
    /*
  else if (mTuner->isRunning()) {
      mTuner->setVolume(volume);
  }
  */
    mVideoRenderer->updateOverlayText("Volume " + QString::number(volume) + "%");
}

void MainWindow::changeChannel(int index)
{
    ChannelInfo* channel = tunerChannels.channelIndex(index);
    if (channel) {
        channel->applySettingsToTuner(mTuner);
        mVideoRenderer->updateOverlayText(channel->getChannelName());
    }
}

void MainWindow::openFile(const QString& source, const QString& sourceName,
    const QString& remoteSub)
{
    stop();
    closeWebview();
    if (mActionCloseAlbumview->isEnabled()) {
        mAlbumController->deactivate();
        mActionGenerateThumbnail->setEnabled(false);
        mActionGenerateThumbnailView->setEnabled(false);
        mActionGenerateThumbnailViewSub->setEnabled(false);
        mVideoRenderer->installEventFilter(this);
        mVideoRenderer->setFocus(Qt::ActiveWindowFocusReason);
    }
    mDemuxer->startDemuxer(source, sourceName, remoteSub);
    enableStreamControls();
}

void MainWindow::openFile(const QString& fileName)
{
    stop();

    closeWebview();

    if (mActionCloseAlbumview->isEnabled()) {
        mAlbumController->deactivate();
        mActionGenerateThumbnail->setEnabled(false);
        mActionGenerateThumbnailView->setEnabled(false);
        mActionGenerateThumbnailViewSub->setEnabled(false);
        mVideoRenderer->installEventFilter(this);
        mVideoRenderer->setFocus(Qt::ActiveWindowFocusReason);
    }

    QFileInfo info(fileName);

    mDemuxer->startDemuxer(fileName, info.fileName(), "");

    enableStreamControls();
}

void MainWindow::skipBack()
{
    if (mDemuxer->isRunning()) {
        const QString* file = mPlaylist->prevFile();
        if (file) {
            openFile(*file);
        }
    } else if (mTuner->isRunning()) {
        ChannelInfo* channel = tunerChannels.previousChannel();
        channel->applySettingsToTuner(mTuner);
        mVideoRenderer->updateOverlayText(channel->getChannelName());
        mControllerWidget->updateChannelIndex(tunerChannels.getCurrentIndex());
    }
}

void MainWindow::seekBack()
{
    if (mDemuxer->isRunning())
        mDemuxer->setSeekingState(mDemuxer->currentMasterPts() - 15.0);
}

void MainWindow::playPause()
{
    if (mDemuxer->isRunning()) {
        if (mControllerWidget->isPaused()) {
            mControllerWidget->setPaused(false);
            mDemuxer->setDemuxingState();
        } else {
            mControllerWidget->setPaused(true);
            mDemuxer->setPausedState();
        }
    } else if (mTuner->isRunning()) {
        if (mControllerWidget->isPaused()) {
            mControllerWidget->setPaused(false);
            mTuner->pauseDVB(false);
        } else {
            mControllerWidget->setPaused(true);
            mTuner->pauseDVB(true);
        }
    }
}

void MainWindow::stop()
{
    if (mDemuxer->isRunning() || mTuner->isRunning()) {
        mDemuxer->stopDemuxer();
        mTuner->stopDVB();
        mGraphFilterMenu->clear();
        if (mActionCloseAlbumview->isEnabled())
            openAlbumview();
        else if (!isFullScreen())
            resizeWindow(0, 160);

        disableStreamControls();
        mGraphFilterMenu->setEnabled(false);
        mControllerWidget->setMode(DEMUXER_CONTROL);
    } else if (mActionCloseAlbumview->isEnabled()) {
        closeWebview();
        openAlbumview();
    }
}

void MainWindow::seekForward()
{
    if (mDemuxer->isRunning())
        mDemuxer->setSeekingState(mDemuxer->currentMasterPts() + 15.0);
}

void MainWindow::skipForward()
{
    if (mDemuxer->isRunning()) {
        const QString* file = mPlaylist->nextFile();
        if (file) {
            openFile(*file);
        }
    } else if (mTuner->isRunning()) {
        ChannelInfo* channel = tunerChannels.nextChannel();
        channel->applySettingsToTuner(mTuner);
        mVideoRenderer->updateOverlayText(channel->getChannelName());
        mControllerWidget->updateChannelIndex(tunerChannels.getCurrentIndex());
    }
}

void MainWindow::seek(int time)
{
    mDemuxer->setSeekingState(time);
}

void MainWindow::demuxerFinished()
{
    const QString* file = mPlaylist->nextFile();
    if (file)
        openFile(*file);
    else {
        if (mActionCloseAlbumview->isEnabled())
            openAlbumview();
        else if (!isFullScreen())
            resizeWindow(0, 160);
        disableStreamControls();
    }
}

void MainWindow::resizeWindow(int w, int h, bool overrideCheck)
{
    if (!isFullScreen() && (mActionResizeOnPlay->isChecked() || overrideCheck)) {
        int left, top, right, bottom;
        getContentsMargins(left, top, right, bottom);
        resize(w + left + right, h + getMenuBar()->height() + mControllerWidget->height() + top + bottom + 6);
    }
}

void MainWindow::startFileStreamer(const QString& fileName, quint16 port)
{
    std::cout << "FileStreamer started " << fileName.toStdString().c_str()
              << "\n";
    delete mFileStreamer; // TODO: fixme
    mFileStreamer = new FileStreamer(fileName, this);
    mFileStreamer->listen(QHostAddress::Any, port);
}

void MainWindow::registerVideoStreams(QList<StreamDescriptor> videoStreams)
{
    foreach (QAction* act, mActionGroupVideo->actions())
        mActionGroupVideo->removeAction(act);
    foreach (QAction* act, mControlVideoMenu->actions())
        mControlVideoMenu->removeAction(act);
    mControlVideoMenu->clear();
    QAction* action = new QAction("Off", mControlVideoMenu);
    action->setData(QVariant(-1));
    mControlVideoMenu->addAction(action);
    mActionGroupVideo->addAction(action);

    for (int i = 0; i < videoStreams.size(); i++) {
        QAction* action = new QAction("Stream " + QString::number(videoStreams[i].index) + ": " + videoStreams[i].name,
            mControlVideoMenu);
        action->setData(QVariant(videoStreams[i].index));
        mControlVideoMenu->addAction(action);
        mActionGroupVideo->addAction(action);
        action->setCheckable(true);
        if (i == 0) {
            action->setChecked(true);
            // controlVideoMenuPressed(action);
        }
    }

    mControlVideoMenu->setEnabled(true);
}

void MainWindow::registerAudioStreams(QList<StreamDescriptor> audioStreams)
{
    foreach (QAction* act, mActionGroupAudio->actions())
        mActionGroupAudio->removeAction(act);
    foreach (QAction* act, mControlAudioMenu->actions())
        mControlAudioMenu->removeAction(act);

    mControlAudioMenu->clear();

    QAction* action = new QAction("Off", mControlAudioMenu);
    action->setData(QVariant(-1));

    mControlAudioMenu->addAction(action);
    mActionGroupAudio->addAction(action);

    for (int i = 0; i < audioStreams.size(); i++) {
        QAction* action = new QAction("Stream " + QString::number(audioStreams[i].index) + ": " + audioStreams[i].name,
            mControlAudioMenu);
        action->setData(QVariant(audioStreams[i].index));
        mControlAudioMenu->addAction(action);
        mActionGroupAudio->addAction(action);
        action->setCheckable(true);
        if (i == 0) {
            action->setChecked(true);
            // controlAudioMenuPressed(action);
        }
    }
    mControlAudioMenu->setEnabled(true);
}

void MainWindow::registerSubtitleStreams(QList<StreamDescriptor> subtitleStreams)
{
    foreach (QAction* act, mActionGroupSubtitle->actions())
        mActionGroupSubtitle->removeAction(act);
    foreach (QAction* act, mControlSubtitleMenu->actions())
        mControlSubtitleMenu->removeAction(act);
    mControlSubtitleMenu->clear();
    QAction* action = new QAction("Off", mControlSubtitleMenu);
    action->setData(QVariant(-1));
    mControlSubtitleMenu->addAction(action);
    mActionGroupSubtitle->addAction(action);

    for (int i = 0; i < subtitleStreams.size(); i++) {
        QAction* action = new QAction("Stream " + QString::number(subtitleStreams[i].index) + ": " + subtitleStreams[i].name,
            mControlSubtitleMenu);
        action->setData(QVariant(subtitleStreams[i].index));
        mControlSubtitleMenu->addAction(action);
        mActionGroupSubtitle->addAction(action);
        action->setCheckable(true);
        if (i == 0) {
            action->setChecked(true);
            // controlSubtitleMenuPressed(action);
        }
    }
    mControlSubtitleMenu->setEnabled(true);
}

void MainWindow::addVideoFilterPreset(QStringList filterList, const QString& name,
    bool setChecked)
{
    QAction* action = mVideoFilterMenu->addAction(name, this, SLOT(changeVideoFilterPreset()));
    mActionGroupVideoFilter->addAction(action);
    action->setData(QVariant(filterList));
    action->setCheckable(true);
    action->setChecked(setChecked);
}

void MainWindow::editVideoFilterPreset(int index, QStringList filterList,
    const QString& name, bool /*setChecked*/)
{
    QList<QAction*> actionList = mActionGroupVideoFilter->actions();
    if (actionList.count() > index) {
        actionList[index]->setText(name);
        actionList[index]->setData(QVariant(filterList));
    }
}

void MainWindow::deleteVideoFilterPreset(int index)
{
    QList<QAction*> actionList = mActionGroupVideoFilter->actions();
    if (actionList.count() > index) {
        mVideoFilterMenu->removeAction(actionList[index]);
        mActionGroupVideoFilter->removeAction(actionList[index]);
        delete actionList[index];
    }
}

QStringList
MainWindow::getVideoFilterPreset(int index)
{
    QList<QAction*> actionList = mActionGroupVideoFilter->actions();
    if (actionList.count() > index)
        return actionList[index]->data().toStringList();
    return QStringList();
}

void MainWindow::addAudioFilterPreset(QStringList filterList, const QString& name,
    bool setChecked)
{
    QAction* action = mAudioFilterMenu->addAction(name, this, SLOT(changeAudioFilterPreset()));
    mActionGroupAudioFilter->addAction(action);
    action->setData(QVariant(filterList));
    action->setCheckable(true);
    action->setChecked(setChecked);
}

void MainWindow::editAudioFilterPreset(int index, QStringList filterList,
    const QString& name, bool /*setChecked*/)
{
    QList<QAction*> actionList = mActionGroupAudioFilter->actions();
    if (actionList.count() > index) {
        actionList[index]->setText(name);
        actionList[index]->setData(QVariant(filterList));
    }
}

void MainWindow::deleteAudioFilterPreset(int index)
{
    QList<QAction*> actionList = mActionGroupAudioFilter->actions();
    if (actionList.count() > index) {
        mAudioFilterMenu->removeAction(actionList[index]);
        mActionGroupAudioFilter->removeAction(actionList[index]);
        delete actionList[index];
    }
}

QStringList
MainWindow::getAudioFilterPreset(int index)
{
    QList<QAction*> actionList = mActionGroupAudioFilter->actions();
    if (actionList.count() > index)
        return actionList[index]->data().toStringList();
    return QStringList();
}

void MainWindow::addStretchPreset(QVector4D preset, const QString& name,
    bool setChecked)
{
    QAction* action = mStrechMenu->addAction(name, this, SLOT(changeStretchPreset()));
    mActionGroupStretch->addAction(action);
    action->setData(QVariant(preset));
    action->setCheckable(true);
    action->setChecked(setChecked);
    if (setChecked)
        mVideoRenderer->changeStretchPreset(preset);
}

void MainWindow::editStretchPreset(int index, QVector4D preset, const QString& name)
{
    // TODO: change all edit to changeStretchPreset and push update if the edited
    // was the currently selected
    QList<QAction*> actionList = mActionGroupStretch->actions();
    if (actionList.count() > index) {
        actionList[index]->setText(name);
        actionList[index]->setData(QVariant(preset));
    }
}

void MainWindow::deleteStretchPreset(int index)
{
    QList<QAction*> actionList = mActionGroupStretch->actions();
    if (actionList.count() > index) {
        mStrechMenu->removeAction(actionList[index]);
        mActionGroupStretch->removeAction(actionList[index]);
        delete actionList[index];
    }
}

QVector4D
MainWindow::getStretchPreset(int index)
{
    QList<QAction*> actionList = mActionGroupStretch->actions();
    QAction* action = actionList.at(index);
    if (action)
        if (action->data().canConvert(QVariant::Vector4D))
            return action->data().value<QVector4D>();
    return QVector4D();
}

void MainWindow::displayErrorMessage(const QString& msg)
{
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Error");
    msgBox.setText(msg);
    msgBox.exec();

    if (isFullScreen())
        toggleFullScreen();

    disconnectFromServer();
    disableStreamControls();
    mControllerWidget->setMode(DEMUXER_CONTROL);
    stop();
}

void MainWindow::setupUI()
{
    QMenu* mediaMenu = getMenuBar()->addMenu("Media");
    mActionOpenFile = new QAction("File...", this);
    connect(mActionOpenFile, SIGNAL(triggered()), this, SLOT(openFileDialog()));
    mActionOpenLocation = new QAction("Url...", this);
    connect(mActionOpenLocation, SIGNAL(triggered()), this,
        SLOT(openUrlDialog()));
    mActionOpenAlbumView = new QAction("Albumview", this);

    connect(mActionOpenAlbumView, SIGNAL(triggered()), this,
        SLOT(openAlbumview()));
    mActionOpenTuner = new QAction("Tuner", this);
    connect(mActionOpenTuner, SIGNAL(triggered()), this, SLOT(openTuner()));
    mActionCloseAlbumview = new QAction("Close Albumview", this);
    mActionCloseAlbumview->setEnabled(false);
    connect(mActionCloseAlbumview, SIGNAL(triggered()), this,
        SLOT(closeAlbumview()));

    mActionExit = new QAction("Exit", this);
    connect(mActionExit, SIGNAL(triggered()), this, SLOT(exitProgram()));

    QMenu* openMenu = new QMenu("Open", this);
    openMenu->addAction(mActionOpenFile);
    addAction(mActionOpenFile);
    mActionOpenFile->setShortcut(Qt::CTRL + Qt::Key_F);
    openMenu->addAction(mActionOpenLocation);
    addAction(mActionOpenLocation);
    mActionOpenLocation->setShortcut(Qt::CTRL + Qt::Key_U);
    openMenu->addAction(mActionOpenAlbumView);
    addAction(mActionOpenAlbumView);
    mActionOpenAlbumView->setShortcut(Qt::CTRL + Qt::Key_A);
    openMenu->addAction(mActionOpenTuner);
    addAction(mActionOpenTuner);
    mActionOpenTuner->setShortcut(Qt::CTRL + Qt::Key_T);

    mediaMenu->addMenu(openMenu);
    mediaMenu->addSeparator();
    mediaMenu->addAction(mActionCloseAlbumview);
    mediaMenu->addSeparator();
    mediaMenu->addAction(mActionExit);
    addAction(mActionExit);
    mActionExit->setShortcut(Qt::CTRL + Qt::Key_E);

    QMenu* controlMenu = getMenuBar()->addMenu("Control");
    mControlVideoMenu = controlMenu->addMenu("Video Stream");
    connect(mControlVideoMenu, SIGNAL(triggered(QAction*)), this,
        SLOT(controlVideoMenuPressed(QAction*)));
    mControlVideoMenu->setEnabled(false);
    mControlAudioMenu = controlMenu->addMenu("Audio Stream");
    connect(mControlAudioMenu, SIGNAL(triggered(QAction*)), this,
        SLOT(controlAudioMenuPressed(QAction*)));
    mControlAudioMenu->setEnabled(false);
    mControlSubtitleMenu = controlMenu->addMenu("Subtitle Stream");
    connect(mControlSubtitleMenu, SIGNAL(triggered(QAction*)), this,
        SLOT(controlSubtitleMenuPressed(QAction*)));
    mControlSubtitleMenu->setEnabled(false);

    mActionGroupVideo = new QActionGroup(this);
    mActionGroupAudio = new QActionGroup(this);
    mActionGroupSubtitle = new QActionGroup(this);

    mActionSkipBack = new QAction("Skip Back/Prev Channel", this);
    mActionSkipBack->setEnabled(false);
    connect(mActionSkipBack, SIGNAL(triggered()), this, SLOT(skipBack()));

    mActionSeekBack = new QAction("Seek Back", this);
    mActionSeekBack->setEnabled(false);
    connect(mActionSeekBack, SIGNAL(triggered()), this, SLOT(seekBack()));

    mActionPlayPause = new QAction("Play/Pause", this);
    mActionPlayPause->setEnabled(false);
    connect(mActionPlayPause, SIGNAL(triggered()), this, SLOT(playPause()));

    mActionStop = new QAction("Stop", this);
    mActionStop->setEnabled(false);
    connect(mActionStop, SIGNAL(triggered()), this, SLOT(stop()));

    mActionSeekForward = new QAction("Seek Forward", this);
    mActionSeekForward->setEnabled(false);
    connect(mActionSeekForward, SIGNAL(triggered()), this, SLOT(seekForward()));

    mActionSkipForward = new QAction("Skip Forward/Next Channel", this);
    mActionSkipForward->setEnabled(false);
    connect(mActionSkipForward, SIGNAL(triggered()), this, SLOT(skipForward()));

    controlMenu->addSeparator();
    controlMenu->addAction(mActionSkipBack);
    addAction(mActionSkipBack);
    mActionSkipBack->setShortcut(Qt::Key_Down);

    controlMenu->addAction(mActionSeekBack);
    addAction(mActionSeekBack);
    mActionSeekBack->setShortcut(Qt::Key_Left);

    controlMenu->addAction(mActionPlayPause);
    addAction(mActionPlayPause);
    mActionPlayPause->setShortcut(Qt::Key_P);

    controlMenu->addAction(mActionStop);
    addAction(mActionStop);
    mActionStop->setShortcut(Qt::Key_S);

    controlMenu->addAction(mActionSeekForward);
    addAction(mActionSeekForward);
    mActionSeekForward->setShortcut(Qt::Key_Right);

    controlMenu->addAction(mActionSkipForward);
    addAction(mActionSkipForward);
    mActionSkipForward->setShortcut(Qt::Key_Up);

    QMenu* serverMenu = getMenuBar()->addMenu("Server");
    mActionListen = new QAction("Start Listen Server", this);
    connect(mActionListen, SIGNAL(triggered()), this, SLOT(startListenServer()));
    mActionConnect = new QAction("Connect To Server", this);
    connect(mActionConnect, SIGNAL(triggered()), this, SLOT(connectToServer()));
    mActionDisconnect = new QAction("Disconnect From Server", this);
    mActionDisconnect->setEnabled(false);
    connect(mActionDisconnect, SIGNAL(triggered()), this,
        SLOT(disconnectFromServer()));

    serverMenu->addAction(mActionListen);
    addAction(mActionListen);
    mActionListen->setShortcut(Qt::CTRL + Qt::Key_S);
    serverMenu->addAction(mActionConnect);
    addAction(mActionConnect);
    mActionConnect->setShortcut(Qt::CTRL + Qt::Key_C);
    serverMenu->addSeparator();
    serverMenu->addAction(mActionDisconnect);
    addAction(mActionDisconnect);
    mActionDisconnect->setShortcut(Qt::CTRL + Qt::Key_D);

    QMenu* viewMenu = getMenuBar()->addMenu("View");
    mActionShowPlaylist = new QAction("Playlist", this);
    connect(mActionShowPlaylist, SIGNAL(triggered()), this, SLOT(showPlaylist()));
    mActionShowAlbumlist = new QAction("Albumlist", this);
    connect(mActionShowAlbumlist, SIGNAL(triggered()), this,
        SLOT(showAlbumlist()));
    mStrechMenu = new QMenu("Stretch", this);
    mActionStretchPresetEdit = new QAction("Edit Presets", this);
    connect(mActionStretchPresetEdit, SIGNAL(triggered()), this,
        SLOT(showStretchPresetDialog()));
    mStrechMenu->addAction(mActionStretchPresetEdit);
    mStrechMenu->addSeparator();
    mActionResizeOnPlay = new QAction("Resize On Play", this);
    mActionResizeOnPlay->setCheckable(true);
    mActionResizeOnPlay->setChecked(true);
    connect(mActionResizeOnPlay, SIGNAL(triggered()), this,
        SLOT(resizeOnPlayToggle()));
    mActionKeepAspect = new QAction("Keep Aspect Ratio", this);
    mActionKeepAspect->setCheckable(true);
    mActionKeepAspect->setChecked(false);
    connect(mActionKeepAspect, SIGNAL(triggered()), this,
        SLOT(toggleKeepAspect()));

    mActionToggleDebugText = new QAction("Toggle Debug Text", this);
    mActionToggleDebugText->setShortcut(Qt::Key_F9);
    connect(mActionToggleDebugText, SIGNAL(triggered()), this,
        SLOT(toggleDebug()));

    mActionToggleFullscreen = new QAction("Toggle Fullscreen", this);
    mActionToggleFullscreen->setShortcut(Qt::Key_F10);
    connect(mActionToggleFullscreen, SIGNAL(triggered()), this,
        SLOT(toggleFullScreen()));

    viewMenu->addAction(mActionShowPlaylist);
    addAction(mActionShowPlaylist);
    mActionShowPlaylist->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_P);

    viewMenu->addAction(mActionShowAlbumlist);
    addAction(mActionShowAlbumlist);
    mActionShowAlbumlist->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_A);

    viewMenu->addSeparator();
    viewMenu->addMenu(mStrechMenu);
    viewMenu->addAction(mActionResizeOnPlay);
    viewMenu->addAction(mActionKeepAspect);
    viewMenu->addAction(mActionToggleDebugText);
    viewMenu->addAction(mActionToggleFullscreen);
    addAction(mActionToggleDebugText);
    addAction(mActionToggleFullscreen);

    QMenu* filterMenu = getMenuBar()->addMenu("Filter");
    mVideoFilterMenu = new QMenu("Video Filter", this);
    mAudioFilterMenu = new QMenu("Audio Filter", this);

    mActionVideoPresetEdit = new QAction("Edit Presets", this);
    connect(mActionVideoPresetEdit, SIGNAL(triggered()), this,
        SLOT(showVideoPresetDialog()));

    mActionGroupStretch = new QActionGroup(this);
    QVector4D preset(0, 0, 0, 0);
    addStretchPreset(preset, "Off", true);

    // mActionAudioPresetEdit = new QAction("Edit Presets", this);
    // connect(mActionAudioPresetEdit, SIGNAL(triggered()), this,
    // SLOT(showAudioPresetDialog()));

    mActionGroupVideoFilter = new QActionGroup(this);
    mActionGroupAudioFilter = new QActionGroup(this);

    mGraphFilterMenu = new QMenu("Graph Filter", this);
    connect(mGraphFilterMenu, SIGNAL(triggered(QAction*)), this,
        SLOT(graphFilterMenuPressed(QAction*)));
    mGraphFilterMenu->setEnabled(false);

    filterMenu->addMenu(mVideoFilterMenu);
    filterMenu->addMenu(mAudioFilterMenu);
    filterMenu->addMenu(mGraphFilterMenu);
    mVideoFilterMenu->addAction(mActionVideoPresetEdit);
    mVideoFilterMenu->addSeparator();

    mActionColorControl = new QAction("Color Control", this);
    connect(mActionColorControl, SIGNAL(triggered()), this,
        SLOT(showColorControl()));
    filterMenu->addSeparator();
    filterMenu->addAction(mActionColorControl);
    // addAction(mActionColorControl);
    // mAudioFilterMenu->addAction(mActionAudioPresetEdit);
    // mAudioFilterMenu->addSeparator();
    addVideoFilterPreset(QStringList(), "Off", true);
    addAudioFilterPreset(QStringList(), "Off", true);

    QMenu* toolsMenu = getMenuBar()->addMenu("Tools");
    mActionGenerateThumbnail = new QAction("Generate Thumbnail", this);
    mActionGenerateThumbnail->setEnabled(false);

    mActionGenerateThumbnailView = new QAction("Generate Thumbnail View", this);
    mActionGenerateThumbnailView->setEnabled(false);

    mActionGenerateThumbnailViewSub = new QAction("Generate Thumbnail View Sub", this);
    mActionGenerateThumbnailViewSub->setEnabled(false);

    mActionShowLog = new QAction("Log", this);
    connect(mActionShowLog, SIGNAL(triggered()), this, SLOT(showLog()));

    mActionOptions = new QAction("Options", this);
    connect(mActionOptions, SIGNAL(triggered()), this, SLOT(showOptions()));

    toolsMenu->addAction(mActionGenerateThumbnail);
    addAction(mActionGenerateThumbnail);
    mActionGenerateThumbnail->setShortcut(Qt::Key_T);

    toolsMenu->addAction(mActionGenerateThumbnailView);
    addAction(mActionGenerateThumbnailView);
    mActionGenerateThumbnailView->setShortcut(Qt::Key_Y);

    toolsMenu->addAction(mActionGenerateThumbnailViewSub);
    addAction(mActionGenerateThumbnailViewSub);
    mActionGenerateThumbnailViewSub->setShortcut(Qt::Key_U);
    toolsMenu->addSeparator();

    toolsMenu->addAction(mActionShowLog);
    addAction(mActionShowLog);
    mActionShowLog->setShortcut(Qt::CTRL + Qt::Key_L);
    toolsMenu->addSeparator();

    toolsMenu->addAction(mActionOptions);
    addAction(mActionOptions);
    mActionOptions->setShortcut(Qt::CTRL + Qt::Key_O);

    QMenu* helpMenu = getMenuBar()->addMenu("Help");
    mActionMediaviewerHelp = new QAction("Mediaviewer Help", this);
    connect(mActionMediaviewerHelp, SIGNAL(triggered()), this, SLOT(help()));
    mActionCheckForUpdates = new QAction("Check For Updates", this);
    connect(mActionCheckForUpdates, SIGNAL(triggered()), this, SLOT(version()));
    mActionAboutMediaViewer = new QAction("About Mediaviewer", this);
    connect(mActionAboutMediaViewer, SIGNAL(triggered()), this, SLOT(about()));

    helpMenu->addAction(mActionMediaviewerHelp);
    mActionMediaviewerHelp->setShortcut(Qt::CTRL + Qt::Key_F1);
    helpMenu->addAction(mActionCheckForUpdates);
    mActionCheckForUpdates->setShortcut(Qt::CTRL + Qt::Key_F2);
    helpMenu->addSeparator();
    helpMenu->addAction(mActionAboutMediaViewer);
    mActionAboutMediaViewer->setShortcut(Qt::CTRL + Qt::Key_F3);

    mPopupMenu = new QMenu("PopupMenu", this);
    mPopupMenu->addMenu(mediaMenu);
    mPopupMenu->addMenu(controlMenu);
    mPopupMenu->addMenu(serverMenu);
    mPopupMenu->addMenu(viewMenu);
    mPopupMenu->addMenu(filterMenu);
    mPopupMenu->addMenu(toolsMenu);
    mPopupMenu->addMenu(helpMenu);

    QWidget* mainWidget = new QWidget;
    mVideoFrame = new QFrame;
    QVBoxLayout* videoFrameLayout = new QVBoxLayout(mVideoFrame);
    // videoFrameLayout->addWidget(mWebView);
    videoFrameLayout->addWidget(mVideoRenderer);
    videoFrameLayout->setMargin(0);
    mFrameStyleNormal = loadStyleSheet("../style/VideoStyle.css");
    mFrameStyleFullScreen = loadStyleSheet("../style/VideoStyleFullScreen.css");
    mVideoFrame->setStyleSheet(mFrameStyleNormal);

    vLayout = new QVBoxLayout(mainWidget);
    // vLayout->addWidget(mWebView);
    vLayout->addWidget(mVideoFrame);
    vLayout->addWidget(mControllerWidget);
    // vLayout->addSpacing(8);
    vLayout->setMargin(0);
    setMainWidget(mainWidget);

    QString style = loadStyleSheet("../style/MenuStyle.css");
    setStyleSheet(style);
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
    CustomWindow::resizeEvent(event);
}

void MainWindow::closeEvent(QCloseEvent* /*event*/)
{
    stop();
    mPlaylist->close();
    mAlbumlist->close();
    mColorControl->close();
    // mLog->close();

    if (mActionCloseAlbumview->isEnabled())
        closeAlbumview();

    disconnectFromServer();

    saveSettingsBinary(mDataLocation + "/General.dat");
    mAlbumController->saveSettingsBinary(mDataLocation + "/Albumview.dat");
    mVideoRenderer->saveSettingsBinary(mDataLocation + "/Video.dat");
    mAudioRenderer->saveSettingsBinary(mDataLocation + "/Audio.dat");
    tunerChannels.saveSettingsBinary(mDataLocation + "/Tuner.dat");
    mServer->saveSettingsBinary(mDataLocation + "/Network.dat");
    mRemoteControl->saveSettingsBinary(mDataLocation + "/Remote.dat");

    if (mMouseEmulation)
        toggleMouseEmulation();
}

bool MainWindow::winEvent(MSG* message, long* /*result*/)
{
    switch (message->message) {
    case WM_INPUT: {
        UINT dwSize;
        GetRawInputData((HRAWINPUT)message->lParam, RID_INPUT, NULL, &dwSize,
            sizeof(RAWINPUTHEADER));
        LPBYTE lpb = new BYTE[dwSize];
        if (lpb == NULL)
            return 0;

        if (GetRawInputData((HRAWINPUT)message->lParam, RID_INPUT, lpb, &dwSize,
                sizeof(RAWINPUTHEADER))
            != dwSize)
            std::cout << "GetRawInputData doesn't return correct size !\n";

        RAWINPUT* raw = (RAWINPUT*)lpb;
        if (raw->header.dwType == RIM_TYPEHID) {
            if (mOptionsDialog->isVisible())
                mOptionsDialog->remoteUpdateValue(
                    mRemoteControl, raw->data.hid.bRawData,
                    raw->data.hid.dwSizeHid * raw->data.hid.dwCount);
            else
                mRemoteControl->handleRemoteId(raw->data.hid.bRawData,
                    raw->data.hid.dwSizeHid * raw->data.hid.dwCount);
        }
        delete[] lpb;
        break;
    }
    default:
        break;
    }

    return false;
}

bool MainWindow::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() >= Qt::Key_0 && keyEvent->key() <= Qt::Key_9) {
            if (mTuner->isRunning()) {
                if (mChannelChange.count() == 0) {
                    mChannelChange.push_back(keyEvent->key() - Qt::Key_0);
                    mVideoRenderer->updateOverlayText(QString::number(mChannelChange[0]));
                    mChannelChangeTimer->start(2000);
                } else {
                    if (mChannelChange[0] == 0 && keyEvent->key() == Qt::Key_0)
                        return false;
                    mChannelChange.push_back(keyEvent->key() - Qt::Key_0);
                    mChannelChangeTimer->stop();
                    mVideoRenderer->updateOverlayText(QString::number(mChannelChange[0]) + QString::number(mChannelChange[1]));
                    int ch = 10 * mChannelChange[0] + mChannelChange[1];
                    ChannelInfo* channel = tunerChannels.channelIndex(ch - 1);
                    if (channel) {
                        channel->applySettingsToTuner(mTuner);
                        mVideoRenderer->updateOverlayText(channel->getChannelName());
                        mControllerWidget->updateChannelIndex(ch - 1);
                    }
                    mChannelChange.clear();
                }
            }
        }
        return false;
    }
    if (event->type() == QEvent::MouseMove) {
        mContextMenuActive = false;
        if (mCursorHidden) {
            mVideoRenderer->setCursor(Qt::ArrowCursor);
            mCursorHidden = false;
        }
        mCursorLastShown = mCurrentTime.elapsed();
        return false;
    } else if (event->type() == QEvent::MouseButtonDblClick) {
        toggleFullScreen();
        return false;
    } else if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::RightButton) {
            mContextMenuActive = true;
            popupContextMenu(mouseEvent->globalPos());
        } else if (mouseEvent->button() == Qt::LeftButton) {
            mContextMenuActive = false;
        }
        return false;
    }
    return CustomWindow::eventFilter(obj, event);
}

void MainWindow::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasFormat("text/uri-list"))
        event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent* event)
{
    QList<QUrl> urlList;
    QString fName;
    QFileInfo info;

    if (event->mimeData()->hasUrls()) {
        urlList = event->mimeData()->urls();
        if (urlList.size() > 0) {
            fName = urlList[0].toLocalFile();
            openFile(fName);
        }
    }
    event->acceptProposedAction();
}

void MainWindow::enableStreamControls()
{
    mControllerWidget->setPaused(false);
    mActionSkipBack->setEnabled(true);
    mActionSeekBack->setEnabled(true);
    mActionPlayPause->setEnabled(true);
    mActionStop->setEnabled(true);
    mActionSeekForward->setEnabled(true);
    mActionSkipForward->setEnabled(true);
    // mActionToggleVideoStream->setEnabled(true);
    // mActionToggleAudioStream->setEnabled(true);
    // mActionToggleSubtitleStream->setEnabled(true);
}

void MainWindow::disableStreamControls()
{
    mControllerWidget->disableProgress();
    mControlVideoMenu->clear();
    mControlVideoMenu->setEnabled(false);
    mControlAudioMenu->clear();
    mControlAudioMenu->setEnabled(false);
    mControlSubtitleMenu->clear();
    mControlSubtitleMenu->setEnabled(false);
    mActionSkipBack->setEnabled(false);
    mActionSeekBack->setEnabled(false);
    mActionPlayPause->setEnabled(false);
    mActionStop->setEnabled(false);
    mActionSeekForward->setEnabled(false);
    mActionSkipForward->setEnabled(false);
    // mActionToggleVideoStream->setEnabled(false);
    // mActionToggleAudioStream->setEnabled(false);
    // mActionToggleSubtitleStream->setEnabled(false);
}

void MainWindow::openFileDialog()
{
    QString fileName = QFileDialog::getOpenFileName(
        this, "Open File", mPrevFileDialogLoc, mFileFilter);
    if (fileName.compare("") != 0) {
        QFileInfo info(fileName);
        mPrevFileDialogLoc = info.path();
        openFile(fileName);
    }
}

void MainWindow::openUrlDialog()
{
    UrlDialog urlDialog(this);
    urlDialog.exec();
    if (urlDialog.result() == QDialog::Accepted)
        openFile(urlDialog.retreiveUrl());
}

void MainWindow::openAlbumview()
{
    if (!mAlbumController->isActivated()) {
        mVideoRenderer->removeEventFilter(this);
        mAlbumController->activate(mConnectionState);
        mActionCloseAlbumview->setEnabled(true);
        mActionGenerateThumbnail->setEnabled(true);
        mActionGenerateThumbnailView->setEnabled(true);
        mActionGenerateThumbnailViewSub->setEnabled(true);
    }
}

void MainWindow::openTuner()
{
    if (mDemuxer->isRunning())
        mDemuxer->stopDemuxer();

    closeWebview();

    if (mActionCloseAlbumview->isEnabled()) {
        mAlbumController->deactivate();
        mActionGenerateThumbnail->setEnabled(false);
        mActionGenerateThumbnailView->setEnabled(false);
        mActionGenerateThumbnailViewSub->setEnabled(false);
        mVideoRenderer->installEventFilter(this);
        mVideoRenderer->setFocus(Qt::ActiveWindowFocusReason);
    }

    if (tunerChannels.getChannelList().size() == 0) {
        std::cout << "Error Channel list empty check channel list in options\n";
        displayErrorMessage("channel list empty check channel list in Options\n");
        return;
    }

    mTuner->startDVB(tunerChannels.currentChannel()->getDeviceName());

    enableStreamControls();

    mControllerWidget->setMode(TUNER_CONTROL);
}

void MainWindow::openWebView(const QString& /*url*/)
{
    stop();
    if (mActionCloseAlbumview->isEnabled()) {
        mAlbumController->deactivate();
        mActionGenerateThumbnail->setEnabled(false);
        mActionGenerateThumbnailView->setEnabled(false);
        mActionGenerateThumbnailViewSub->setEnabled(false);
        // mVideoRenderer->installEventFilter(this);
        // mVideoRenderer->setFocus(Qt::ActiveWindowFocusReason);
    }
    // closeAlbumview();

    // mActionCloseWebView->setEnabled(true);
    mVideoRenderer->setVisible(false);
    // mWebView->setVisible(true);
    resizeWindow(800, 480);
    // mWebView->setFocus(Qt::ActiveWindowFocusReason);
    // mWebView->load(QUrl(url));
}

void MainWindow::closeAlbumview()
{
    if (mActionCloseAlbumview->isEnabled()) {
        mActionGenerateThumbnail->setEnabled(false);
        mActionGenerateThumbnailView->setEnabled(false);
        mActionGenerateThumbnailViewSub->setEnabled(false);
        mAlbumController->deactivate();
        mActionCloseAlbumview->setEnabled(false);
        mVideoRenderer->installEventFilter(this);
        mVideoRenderer->setFocus(Qt::ActiveWindowFocusReason);

        if (!isFullScreen() && !mDemuxer->isRunning() && !mTuner->isRunning())
            resizeWindow(0, 160);
    }
}

void MainWindow::closeWebview()
{
    //#ifndef NO_WEBVIEW_LOAD
    //    mWebView->load(QUrl("../blank.html"));
    //#endif
    mVideoRenderer->setVisible(true);
    //    mWebView->setVisible(false);
}

void MainWindow::exitProgram()
{
    close();
}
/*
void MainWindow::changeVideoStream()
{
    if (mDemuxer->isRunning()) {
        QAction *action = mActionGroupVideo->checkedAction();
        mDemuxer->changeVideoStream(action->data().toInt());
    }
    else if (mTuner->isRunning()) {
        //mTuner->mapPidToVideoPin();
    }
}
*/
/*
void MainWindow::changeAudioStream()
{
    if (mDemuxer->isRunning()) {
        QAction *action = mActionGroupAudio->checkedAction();
        mDemuxer->changeAudioStream(action->data().toInt());
    }
    else if (mTuner->isRunning()) {
        //mTuner->mapPidToAudioPin();
    }
}
*/
/*
void MainWindow::changeSubtitleStream()
{
    QAction *action = mActionGroupSubtitle->checkedAction();
    mDemuxer->changeSubtitleStream(action->data().toInt());
}
*/
void MainWindow::startListenServer()
{
    disconnectFromServer();
    mListenServerDialog->initializeServer(mServer);
    showListenServerDialog();
}

void MainWindow::showListenServerDialog()
{
    mListenServerDialog->exec();
    if (mListenServerDialog->result() == QDialog::Rejected) {
        if (mFileStreamer) {
            mFileStreamer->close();
            delete mFileStreamer;
            mFileStreamer = NULL;
        }
    }
}

void MainWindow::connectToServer()
{
    ConnectServerDialog dialog(mClient, this);
    dialog.exec();
    if (dialog.result() == QDialog::Accepted) {
        QByteArray block;
        QDataStream out(&block, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_4_6);
        out << (int)REQ_HANDSHAKE;
        out << (int)0;
        QString tmp = dialog.getPassword();
        out << tmp;
        out.device()->seek(sizeof(int));
        out << block.size() - 2 * sizeof(int);
        mClient->sendMessage(block);

        std::cout << "Connected to server\n";
        mActionDisconnect->setEnabled(true);
        mConnectionState = CONNECTION_REMOTE;
        closeAlbumview();
    }
}

void MainWindow::disconnectFromServer()
{
    mClient->disconnect();
    closeAlbumview();
    mActionDisconnect->setEnabled(false);
    mConnectionState = CONNECTION_LOCAL;
}

void MainWindow::clientReceivedMessage(RemoteMsg& msg)
{
    std::cout << "Message received from server\n";
    mAlbumController->readMsgClient(msg);
    mClient->clearMessage();
}

void MainWindow::serverReceivedMessage(RemoteMsg& msg)
{
    std::cout << "Message received from client\n";
    mAlbumController->readMsgServer(msg);
    mServer->clearMessage();
}

void MainWindow::showPlaylist()
{
    mPlaylist->show();
}

void MainWindow::showAlbumlist()
{
    mAlbumlist->show();
}

void MainWindow::showColorControl()
{
    mColorControl->show();
}

void MainWindow::showStretchPresetDialog()
{
    QStringList lst;
    QList<QAction*> actions = mActionGroupStretch->actions();
    for (int i = 0; i < actions.count(); i++)
        lst << actions[i]->text();

    StretchPresetDialog dialog(this, lst);
    dialog.exec();
}

void MainWindow::changeStretchPreset()
{
    QAction* action = mActionGroupStretch->checkedAction();
    QVector4D preset = action->data().value<QVector4D>();
    mVideoRenderer->changeStretchPreset(action->data().value<QVector4D>());
    mVideoRenderer->updateOverlayText("Stretch Preset " + action->text());
}

void MainWindow::resizeOnPlayToggle()
{
    if (mActionResizeOnPlay->isChecked())
        mVideoRenderer->resizeOnPlay(true);
    else
        mVideoRenderer->resizeOnPlay(false);
}

void MainWindow::toggleKeepAspect()
{
    if (mVideoRenderer->getKeepAspectRatio()) {
        mActionKeepAspect->setChecked(false);
        mVideoRenderer->keepAspectRatio(false);
        mVideoRenderer->cornerDefault();
    } else {
        mActionKeepAspect->setChecked(true);
        mVideoRenderer->keepAspectRatio(true);
    }
}

void MainWindow::toggleFullScreen()
{
    static int margin = vLayout->margin();
    if (isFullScreen()) {
        mVideoFrame->setStyleSheet(mFrameStyleNormal);
        mControllerWidget->setVisible(true);
        showNormal();
    } else {
        mVideoFrame->setStyleSheet(mFrameStyleFullScreen);
        mControllerWidget->setVisible(false);
        showFullScreen();
    }
}

void MainWindow::showVideoPresetDialog()
{
    QList<QAction*> presetActions = mActionGroupVideoFilter->actions();
    QStringList presetList;
    for (int i = 0; i < presetActions.size(); i++)
        presetList << presetActions[i]->text();

    FilterPresetDialog dialog(this, VIDEOMODE, presetList,
        mVideoRenderer->retreiveFilters());
    dialog.exec();
    if (dialog.result() == QDialog::Accepted) {
    }
}

void MainWindow::changeVideoFilterPreset()
{
    QAction* action = mActionGroupVideoFilter->checkedAction();
    mVideoRenderer->changeVideoFilterPreset(action->data().toStringList());
    mVideoRenderer->updateOverlayText("Filter Preset " + action->text());
}

void MainWindow::changeAudioFilterPreset()
{
}

void MainWindow::showLog()
{
    // mLog->show();
}

void MainWindow::showOptions()
{
    mOptionsDialog->loadGeneralSettings(this);
    mOptionsDialog->loadAlbumviewSettings(mAlbumController);
    mOptionsDialog->loadVideoSettings(mVideoRenderer);
    mOptionsDialog->loadAudioSettings(mAudioRenderer);
    mOptionsDialog->loadTunerSettings(&tunerChannels);
    mOptionsDialog->loadNetworkSettings(mServer);
    mOptionsDialog->loadRemoteSettings(mRemoteControl);
    mOptionsDialog->exec();
    if (mOptionsDialog->result() == QDialog::Accepted) {
        mOptionsDialog->saveGeneralSettings(this);
        mOptionsDialog->saveAlbumviewSettings(mAlbumController);
        mOptionsDialog->saveVideoSettings(mVideoRenderer);
        mOptionsDialog->saveAudioSettings(mAudioRenderer);
        mOptionsDialog->saveTunerSettings(&tunerChannels);
        mOptionsDialog->saveNetworkSettings(mServer);
        mOptionsDialog->saveRemoteSettings(mRemoteControl);
    }
}

void MainWindow::help()
{
//    QString path = QFileInfo("../help/readme.html").absoluteFilePath();
//    QDesktopServices::openUrl(QUrl("file:///" + path));
}

void MainWindow::version()
{
    QDialog dialog(this);
    dialog.setFixedSize(300, 200);
    dialog.setWindowTitle("Check For Updates");
    dialog.exec();
}

void MainWindow::about()
{
#ifndef NO_IMAGE_ALLOC
    QPixmap pix;
    pix.load("../resources/Images/title48.png");
#endif
    QDialog* aboutdialog = new QDialog(this);
    //aboutdialog->setModal(true);
    aboutdialog->setFixedSize(300, 300);
    aboutdialog->setWindowTitle("About MediaViewer");
    QFont font;
    font.setBold(true);
    QLabel* label = new QLabel("", aboutdialog);
    QLabel* label2 = new QLabel(
        "This software uses libraries from\n"
        "FFmpeg\n"
        "Qt\n"
        "ftgl\n"
        "OpenAL-soft\n"
        "freetype2\n"
        "glew\n\n"
        "Written by Erik Matzols 2009");
#ifndef NO_IMAGE_ALLOC
    label->setPixmap(pix);
#endif
    label2->setFont(font);
    QVBoxLayout* lay = new QVBoxLayout(aboutdialog);
    lay->addWidget(label);
    lay->addWidget(label2);

    label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    label2->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    aboutdialog->setLayout(lay);
    aboutdialog->exec();
}

void MainWindow::popupContextMenu(const QPoint& pos)
{
    mPopupMenu->popup(pos);
}

void MainWindow::trayActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::DoubleClick) {
        show();
        mTrayIcon->hide();
        showListenServerDialog();
    }
}

void MainWindow::channelChangeTimeout()
{
    if (mChannelChange.count() == 1) {
        if (mChannelChange[0] > 0) {
            ChannelInfo* channel = tunerChannels.channelIndex(mChannelChange[0] - 1);
            if (channel) {
                channel->applySettingsToTuner(mTuner);
                mVideoRenderer->updateOverlayText(channel->getChannelName());
                mControllerWidget->updateChannelIndex(mChannelChange[0] - 1);
            }
        }
        mChannelChange.clear();
    }
}

LRESULT CALLBACK
KeyboardHook(int ncode, WPARAM wParam, LPARAM lParam)
{
    static int acc = 0;
    KBDLLHOOKSTRUCT* key = (KBDLLHOOKSTRUCT*)lParam;

    if (!mMouseEmulation)
        return CallNextHookEx(hookHandle, ncode, wParam, lParam);

    INPUT input;
    input.type = INPUT_MOUSE;
    input.mi.dx = 0;
    input.mi.dy = 0;
    input.mi.mouseData = 0;
    input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    input.mi.time = 0;
    input.mi.dwExtraInfo = 0;

    switch (wParam) {
    case WM_KEYDOWN: {
        if (key->vkCode == VK_LEFT) {
            input.mi.dx = -9 - (acc++);
            input.mi.dy = 0;
            input.mi.dwFlags = MOUSEEVENTF_MOVE;
            SendInput(1, &input, sizeof(input));
        } else if (key->vkCode == VK_RIGHT) {
            input.mi.dx = 9 + (acc++);
            input.mi.dy = 0;
            input.mi.dwFlags = MOUSEEVENTF_MOVE;
            SendInput(1, &input, sizeof(input));
        } else if (key->vkCode == VK_UP) {
            input.mi.dx = 0;
            input.mi.dy = -9 - (acc++);
            input.mi.dwFlags = MOUSEEVENTF_MOVE;
            SendInput(1, &input, sizeof(input));
        } else if (key->vkCode == VK_DOWN) {
            input.mi.dx = 0;
            input.mi.dy = 9 + (acc++);
            input.mi.dwFlags = MOUSEEVENTF_MOVE;
            SendInput(1, &input, sizeof(input));
        } else if (key->vkCode == VK_RETURN) {
            SendInput(1, &input, sizeof(input));
            input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
            SendInput(1, &input, sizeof(input));
        }
        break;
    }
    case WM_KEYUP: {
        acc = 0;
        break;
    }
    default:
        break;
    }
    return 1;
}

void MainWindow::toggleMouseEmulation()
{
    mMouseEmulation = !mMouseEmulation;
    if (mMouseEmulation)
        hookHandle = SetWindowsHookEx(WH_KEYBOARD_LL, &KeyboardHook, GetModuleHandle(NULL), 0);
    else
        UnhookWindowsHookEx(hookHandle);
}

void MainWindow::toggleDebug()
{
    static bool debug = false;
    debug = !debug;
    mDemuxer->setDebug(debug);
}

void MainWindow::toggleVideoStream()
{
    QAction* checkedAction = mActionGroupVideo->checkedAction();
    QList<QAction*> actionLst = mActionGroupVideo->actions();
    for (int i = 0; i < actionLst.size(); i++) {
        if (checkedAction == actionLst[i]) {
            if (actionLst.size() > (i + 1)) {
                controlVideoMenuPressed(actionLst[i + 1]);
            } else {
                controlVideoMenuPressed(actionLst[0]);
            }
        }
    }
}

void MainWindow::toggleAudioStream()
{
    QAction* checkedAction = mActionGroupAudio->checkedAction();
    QList<QAction*> actionLst = mActionGroupAudio->actions();
    for (int i = 0; i < actionLst.size(); i++) {
        if (checkedAction == actionLst[i]) {
            if (actionLst.size() > (i + 1)) {
                controlAudioMenuPressed(actionLst[i + 1]);
            } else {
                controlAudioMenuPressed(actionLst[0]);
            }
        }
    }
}

void MainWindow::toggleSubtitleStream()
{
    QAction* checkedAction = mActionGroupSubtitle->checkedAction();
    QList<QAction*> actionLst = mActionGroupSubtitle->actions();
    for (int i = 0; i < actionLst.size(); i++) {
        if (checkedAction == actionLst[i]) {
            if (actionLst.size() > (i + 1)) {
                controlSubtitleMenuPressed(actionLst[i + 1]);
            } else {
                controlSubtitleMenuPressed(actionLst[0]);
            }
        }
    }
}

void MainWindow::toggleVideoFilter()
{
    QAction* checkedAction = mActionGroupVideoFilter->checkedAction();
    QList<QAction*> actionLst = mActionGroupVideoFilter->actions();
    for (int i = 0; i < actionLst.size(); i++) {
        if (checkedAction == actionLst[i]) {
            if (actionLst.size() > (i + 1))
                actionLst[i + 1]->setChecked(true);
            else
                actionLst[0]->setChecked(true);
            changeVideoFilterPreset();
        }
    }
}

void MainWindow::toggleAudioFilter()
{
}

void MainWindow::toggleZoomPreset()
{
    QAction* checkedAction = this->mActionGroupStretch->checkedAction();
    QList<QAction*> actionList = mActionGroupStretch->actions();
    for (int i = 0; i < actionList.size(); i++) {
        if (checkedAction == actionList[i]) {
            if (actionList.size() > (i + 1))
                actionList[i + 1]->setChecked(true);
            else
                actionList[0]->setChecked(true);
            changeStretchPreset();
        }
    }
}

bool MainWindow::mouseEmulation()
{
    return mMouseEmulation;
}

void MainWindow::tunerStarted()
{
    mGraphFilterMenu->clear();
    QStringList filters = mTuner->queryFilters();
    foreach (QString filter, filters) {
        QAction* action = new QAction(filter, mGraphFilterMenu);
        mGraphFilterMenu->addAction(action);
    }
    mGraphFilterMenu->setEnabled(true);
    mControllerWidget->registerChannels(tunerChannels.getChannelList(),
        tunerChannels.getCurrentIndex());
    ChannelInfo* channel = tunerChannels.currentChannel();
    channel->applySettingsToTuner(mTuner);
    mVideoRenderer->updateOverlayText(channel->getChannelName());
}

void MainWindow::tunerChannelChanged()
{
    ChannelInfo* channel = tunerChannels.currentChannel();
    QList<MediaPid> videoStreams = channel->queryVideoStreams();
    QList<MediaPid> audioStreams = channel->queryAudioStreams();

    foreach (QAction* act, mActionGroupVideo->actions())
        mActionGroupVideo->removeAction(act);
    foreach (QAction* act, mControlVideoMenu->actions())
        mControlVideoMenu->removeAction(act);
    mControlVideoMenu->clear();

    for (int i = 0; i < videoStreams.size(); i++) {
        QAction* action = new QAction(mediatypeToStr[videoStreams[i].type], mControlVideoMenu);
        action->setData(QVariant(i));
        mControlVideoMenu->addAction(action);
        mActionGroupVideo->addAction(action);
        action->setCheckable(true);
        if (i == 0) {
            controlVideoMenuPressed(action);
        }
    }

    foreach (QAction* act, mActionGroupAudio->actions())
        mActionGroupAudio->removeAction(act);
    foreach (QAction* act, mControlAudioMenu->actions())
        mControlAudioMenu->removeAction(act);
    mControlAudioMenu->clear();

    for (int i = 0; i < audioStreams.size(); i++) {
        QAction* action = new QAction(mediatypeToStr[audioStreams[i].type], mControlAudioMenu);
        action->setData(QVariant(i));
        mControlAudioMenu->addAction(action);
        mActionGroupAudio->addAction(action);
        action->setCheckable(true);
        if (i == 0) {
            controlAudioMenuPressed(action);
        }
    }

    mControlVideoMenu->setEnabled(true);
    mControlAudioMenu->setEnabled(true);
}

void MainWindow::emulateNavigateIn()
{
    INPUT input;
    input.type = INPUT_KEYBOARD;
    SendInput(1, &input, sizeof(input));
}

void MainWindow::emulateNavigateOut()
{
    INPUT input;
    input.type = INPUT_KEYBOARD;
    SendInput(1, &input, sizeof(input));
}

void MainWindow::emulateNavigateLeft()
{
    INPUT input;
    input.type = INPUT_KEYBOARD;
    SendInput(1, &input, sizeof(input));
}

void MainWindow::emulateNavigateRight()
{
    INPUT input;
    input.type = INPUT_KEYBOARD;
    SendInput(1, &input, sizeof(input));
}

void MainWindow::emulateNavigateUp()
{
    INPUT input;
    input.type = INPUT_KEYBOARD;
    SendInput(1, &input, sizeof(input));
}

void MainWindow::emulateNavigateDown()
{
    INPUT input;
    input.type = INPUT_KEYBOARD;
    SendInput(1, &input, sizeof(input));
}

void MainWindow::graphFilterMenuPressed(QAction* action)
{
    mTuner->showPropertyPage(action->text());
}

void MainWindow::controlVideoMenuPressed(QAction* action)
{
    if (mDemuxer->isRunning()) {
        mDemuxer->changeVideoStream(action->data().toInt());
    } else if (mTuner->isRunning()) {
        ChannelInfo* channel = tunerChannels.currentChannel();
        QList<MediaPid> videoStreams = channel->queryVideoStreams();
        mTuner->mapPidToVideoPin(videoStreams[action->data().toInt()]);
    }

    action->setCheckable(true);
    action->setChecked(true);

    mVideoRenderer->updateOverlayText("Video Stream " + action->text());
}

void MainWindow::controlAudioMenuPressed(QAction* action)
{
    if (mDemuxer->isRunning()) {
        mDemuxer->changeAudioStream(action->data().toInt());
    } else if (mTuner->isRunning()) {
        ChannelInfo* channel = tunerChannels.currentChannel();
        QList<MediaPid> audioStreams = channel->queryAudioStreams();
        mTuner->mapPidToAudioPin(audioStreams[action->data().toInt()]);
    }

    action->setCheckable(true);
    action->setChecked(true);

    mVideoRenderer->updateOverlayText("Audio Stream " + action->text());
}

void MainWindow::controlSubtitleMenuPressed(QAction* action)
{
    if (mDemuxer->isRunning()) {
        mDemuxer->changeSubtitleStream(action->data().toInt());
    } else if (mTuner->isRunning()) {
    }

    action->setCheckable(true);
    action->setChecked(true);

    mVideoRenderer->updateOverlayText("Subtitle Stream " + action->text());
}
