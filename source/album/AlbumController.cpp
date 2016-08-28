#include "AlbumController.hpp"
#include "AlbumList.hpp"
#include "AlbumModel.hpp"
#include "MainWindow.hpp"
#include "TcpCommand.hpp"
#include "VideoRenderer.hpp"
#include <iostream>

AlbumController::AlbumController(AlbumModel* model, VideoRenderer* renderer, MainWindow* parent, TcpCommand* client, TcpCommand* server)
    : QObject(parent)
    , mLocalModel(model)
    , mRenderer(renderer)
    , mParent(parent)
    , mThumbnailExtractor(this, renderer)
{
    mRemoteModel = new AlbumModel();
    mCurrentModel = mLocalModel;
    mActivated = false;
    mAutostart = false;
    mUsePlaylist = false;
    mThumbnailExtractTime = 15;
    mPrevWidth = 640;
    mPrevHeight = 480;
    mSetArrowCursor = false;
    mState = CONNECTION_LOCAL;
    mTcpClient = client;
    mTcpServer = server;
    connect(&mRefresh, SIGNAL(timeout()), mRenderer, SLOT(renderAlbums()));
    connect(&mThumbnailExtractor, SIGNAL(thumbnailsExtracted()), this, SLOT(thumbnailsExtracted()));
    mCurrentEntityImage = 0;
}

AlbumController::~AlbumController()
{
    delete mLocalModel;
    delete mRemoteModel;
}

bool AlbumController::autoStartEnabled()
{
    return mAutostart;
}

void AlbumController::setAutoStartEnabled(bool enabled)
{
    mAutostart = enabled;
}

bool AlbumController::playlistEnabled()
{
    return mUsePlaylist;
}

void AlbumController::setPlaylistEnabled(bool enabled)
{
    mUsePlaylist = enabled;
}

float AlbumController::scrollSpeed()
{
    return mRenderer->scrollSpeed();
}

void AlbumController::setScrollSpeed(float scrollspeed)
{
    mRenderer->setScrollSpeed(scrollspeed);
}

int AlbumController::thumbnailExtractTime()
{
    return mThumbnailExtractTime;
}

void AlbumController::setThumbnailExtractTime(int time)
{
    mThumbnailExtractTime = time;
}

void AlbumController::activate(ConnectionState state)
{
    mSetArrowCursor = false;
    mActivated = true;
    mState = state;
    if (!mRenderer->isVisible())
        mRenderer->setVisible(true);
    if (mState == CONNECTION_LOCAL) {
        mCurrentModel = mLocalModel;
        mRenderer->setModel(mCurrentModel);
        mRenderer->loadAlbumEntityImages(mCurrentModel->currentAlbums());
    } else {
        mCurrentModel = mRemoteModel;
        mRenderer->setModel(mCurrentModel);
        sendAlbumModelRequest();
    }

    mRenderer->installEventFilter(this);
    mRenderer->setFocus(Qt::ActiveWindowFocusReason);
    if (mPrevWidth == mRenderer->screenWidth() && mPrevHeight == mRenderer->screenHeight() && !mParent->isFullScreen())
        mRenderer->openAlbumRenderer(640, 480);
    else
        mRenderer->openAlbumRenderer(mPrevWidth, mPrevHeight);
}

void AlbumController::deactivate()
{
    mSetArrowCursor = true;
    mRefresh.stop();

    mRenderer->unloadAlbumEntityImages(mCurrentModel->currentAlbums());
    mRenderer->removeEventFilter(this);
    mRenderer->clearFocus();
    mRenderer->closeAlbumRenderer();

    mActivated = false;
    mPrevWidth = mRenderer->width();
    mPrevHeight = mRenderer->height();
}

bool AlbumController::addAlbum(AlbumEntity* album)
{
    // TODO: warning! model reverts root level but renderer does
    // not unload images if it is in another level,
    // UGLY fix always unload images when add new albums
    // annyoing when you are adding multiple albums
    if (mLocalModel == mCurrentModel)
        mRenderer->unloadAlbumEntityImages(mLocalModel->currentAlbums());

    mLocalModel->addAlbum(album);

    if (mLocalModel == mCurrentModel)
        mRenderer->loadAlbumEntityImages(mLocalModel->currentAlbums());

    return true;
}

void AlbumController::removeAlbum(int index)
{
    mLocalModel->removeAlbum(index);
}

void AlbumController::removeAllAlbums()
{
    mLocalModel->clearAlbums();
}

bool AlbumController::isActivated()
{
    return mActivated;
}

AlbumModel* AlbumController::getLocalModel()
{
    return mLocalModel;
}

void AlbumController::setModel(AlbumModel* newModel)
{
    mRenderer->setModel(newModel);
    delete mLocalModel;
    mLocalModel = newModel;
}

VideoRenderer* AlbumController::getRenderer()
{
    return mRenderer;
}

void AlbumController::readMsgServer(RemoteMsg& msg)
{
    switch (msg.msgType) {
    case REQ_ALBUMMODEL:
        sendAlbumModelReply();
        break;
    case REQ_MOVEIN: {
        QDataStream in(&msg.dataBlock, QIODevice::ReadOnly);
        in.setVersion(QDataStream::Qt_4_6);
        int selectedEntity;
        in >> selectedEntity;
        mCurrentModel->setSelectedEntity(selectedEntity);
        QFileInfo inf(mCurrentModel->currentSelectedFile());
        if (inf.isFile()) {
            // fix web album in remote mode
            if (inf.suffix().compare("web") == 0)
                sendAlbumFileReply(true);
            else {
                emit startFileStreamer(mCurrentModel->currentSelectedFile(), mTcpServer->serverStreamPort());
                sendAlbumFileReply(false);
            }
        } else {
            int albumState = mCurrentModel->moveIn();
            if (albumState == MOVEIN_SUCCESS)
                sendAlbumModelReply();
        }
    } break;
    case REQ_MOVEOUT:
        mCurrentModel->moveOut();
        sendAlbumModelReply();
        break;
    case REQ_ALBUMIMAGE: {
        QDataStream in(&msg.dataBlock, QIODevice::ReadOnly);
        in.setVersion(QDataStream::Qt_4_6);
        in >> mCurrentEntityImage;
        sendAlbumImageReply();
    } break;
    default:
        return;
        break;
    }
}

void AlbumController::readMsgClient(RemoteMsg& msg)
{
    QDataStream in(msg.dataBlock);
    in.setVersion(QDataStream::Qt_4_6);
    switch (msg.msgType) {
    case REP_ALBUMMODEL: {
        int entities;
        QString dirName;
        in >> entities;
        in >> dirName;
        mRenderer->unloadAlbumEntityImages(mCurrentModel->currentAlbums());
        mCurrentModel->clearAlbums();
        for (int i = 0; i < entities; i++) {
            QString name;
            in >> name;
            AlbumEntity* entity = new AlbumEntity;
            entity->setDisplayName(name);
            entity->setImage(mRenderer->getDefaultTexture());
            mCurrentModel->addAlbum(entity);
        }
        mCurrentModel->setCurrentDirectoryName(dirName);
        mCurrentEntityImage = 0;
        if (mCurrentModel->currentAlbums().size() > 1)
            sendAlbumImageRequest();
    } break;
    case REP_ALBUMIMAGE: {
        std::cout << "file image received\n";
        QByteArray imageBlock;
        in >> mCurrentEntityImage;
        if (msg.blockSize > sizeof(int)) {
            in >> imageBlock;
            QImage image;
            image.loadFromData(imageBlock);
            mRenderer->makeCurrent();
            unsigned int tex = mRenderer->loadImage(image);
            mRenderer->doneCurrent();
            mCurrentModel->currentAlbums()[mCurrentEntityImage]->setImage(tex);
            //printf("tex id %d\n",tex);
        }
        mCurrentEntityImage++;
        if (mCurrentEntityImage < mCurrentModel->currentAlbums().size())
            sendAlbumImageRequest();
    } break;
    case REP_ALBUMFILE: {
        int port;
        QString fileName, subName;
        QByteArray subData;
        in >> port;
        in >> fileName;
        if (port != -1) {
            in >> subName;
            if (subName.size()) {
                in >> subData;
                QFile file(mParent->tempLocation() + "/" + subName);
                file.open(QIODevice::WriteOnly);
                file.write(subData);
                file.flush();
                file.close();
                subName = mParent->tempLocation() + "/" + subName;
            }
            emit openFile("tcp://" + mTcpClient->peerName() + ":" + QString::number(port), fileName, subName);
        } else
            emit openWebview(fileName);
    } break;
    default:
        break;
    }
}

bool AlbumController::loadSettingsBinary(const QString& fileName, Albumlist* albumList)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        std::cout << "Failed to load " << fileName.toStdString().c_str() << "\n";
        return false;
    }
    QDataStream in(&file);
    int a, b, c, d;
    in.setVersion(QDataStream::Qt_4_6);
    in >> a;
    in >> b;
    mLocalModel->setDimension(a, b);
    mRemoteModel->setDimension(a, b);
    in >> a;
    in >> b;
    mLocalModel->setSpacing(a, b);
    mRemoteModel->setSpacing(a, b);
    in >> a;
    in >> b;
    in >> c;
    in >> d;
    mLocalModel->setMargin(a, b, c, d);
    mRemoteModel->setMargin(a, b, c, d);
    float s;
    in >> s;
    mRenderer->setScrollSpeed(s);
    in >> mThumbnailExtractTime;
    in >> mUsePlaylist;
    in >> mAutostart;
    in >> a;
    for (int i = 0; i < a; i++) {
        QString str;
        in >> str;
        albumList->addItem(str);
        //mParent->getAlbumlist()->addItem(str);
    }
    file.close();
    return true;
}

bool AlbumController::saveSettingsBinary(const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly))
        return false;
    QDataStream out(&file);
    out.setVersion(QDataStream::Qt_4_6);
    AlbumAlignment alignment = mLocalModel->albumAlignment();
    out << alignment.maxI;
    out << alignment.maxJ;
    out << alignment.xSpacing;
    out << alignment.ySpacing;
    out << alignment.uMargin;
    out << alignment.dMargin;
    out << alignment.lMargin;
    out << alignment.rMargin;
    out << mRenderer->scrollSpeed();
    out << mThumbnailExtractTime;
    out << mUsePlaylist;
    out << mAutostart;
    out << mLocalModel->albumRoot().size();
    for (int i = 0; i < mLocalModel->albumRoot().size(); i++)
        out << mLocalModel->albumRoot()[i]->fileName();
    file.close();
    return true;
}

void AlbumController::sendMoveInRequest()
{
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_6);
    out << (int)REQ_MOVEIN;
    out << (int)0;
    out << mCurrentModel->selectedEntity();
    out.device()->seek(sizeof(int));
    out << block.size() - 2 * sizeof(int);
    mTcpClient->sendMessage(block);
}

void AlbumController::sendMoveOutRequest()
{
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_6);
    out << (int)REQ_MOVEOUT;
    out << (int)0;
    mTcpClient->sendMessage(block);
}

void AlbumController::sendAlbumModelRequest()
{
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_6);
    out << (int)REQ_ALBUMMODEL;
    out << (int)0;
    mTcpClient->sendMessage(block);
}

void AlbumController::sendAlbumImageRequest()
{
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_6);
    out << (int)REQ_ALBUMIMAGE;
    out << (int)0;
    out << mCurrentEntityImage;
    out.device()->seek(sizeof(int));
    out << block.size() - 2 * sizeof(int);
    mTcpClient->sendMessage(block);
}

void AlbumController::sendAlbumModelReply()
{
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_6);
    out << (int)REP_ALBUMMODEL;
    out << (int)0;
    out << (int)mLocalModel->currentAlbums().size();
    out << mLocalModel->currentDirectoryName();
    for (int i = 0; i < mLocalModel->currentAlbums().size(); i++)
        out << mLocalModel->currentAlbums()[i]->displayName();
    out.device()->seek(sizeof(int));
    out << block.size() - 2 * sizeof(int);
    mTcpServer->sendMessage(block);
}

void AlbumController::sendAlbumFileReply(bool noStream)
{
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_6);
    out << (int)REP_ALBUMFILE;
    out << (int)0;
    if (noStream) {
        out << (int)-1;
        QFileInfo fileInfo(mCurrentModel->currentSelectedFile());
        QFile file(mCurrentModel->currentSelectedFile());
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            return;
        QTextStream in(&file);
        out << in.readLine();
        file.close();
    } else {
        out << (int)mTcpServer->serverStreamPort(); // asdfasdfasdf
        QFileInfo fileInfo(mCurrentModel->currentSelectedFile());
        out << fileInfo.fileName();
        QFileInfo srtInfo(fileInfo.path() + "/" + fileInfo.completeBaseName() + ".srt");
        QFileInfo subInfo(fileInfo.path() + "/" + fileInfo.completeBaseName() + ".sub");
        if (srtInfo.exists()) {
            out << srtInfo.fileName();
            QFile file(srtInfo.absoluteFilePath());
            if (file.open(QIODevice::ReadOnly | QIODevice::Text))
                out << file.readAll();
        } else if (subInfo.exists()) {
            out << subInfo.fileName();
            QFile file(subInfo.absoluteFilePath());
            if (file.open(QIODevice::ReadOnly | QIODevice::Text))
                out << file.readAll();
        } else
            out << "";
    }

    out.device()->seek(sizeof(int));
    out << block.size() - 2 * sizeof(int);
    mTcpServer->sendMessage(block);
}

void AlbumController::sendAlbumImageReply()
{
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_6);
    out << (int)REP_ALBUMIMAGE;
    out << (int)0;
    out << mCurrentEntityImage;
    QFileInfo info(mLocalModel->currentAlbums()[mCurrentEntityImage]->fileImage());
    if (info.isFile()) {
        QFile file(info.absoluteFilePath());
        file.open(QIODevice::ReadOnly);
        out << file.readAll();
    }
    out.device()->seek(sizeof(int));
    out << block.size() - 2 * sizeof(int);
    mTcpServer->sendMessage(block);
}

bool AlbumController::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        handleKeyEvent(keyEvent);
        return false;
    } else if (event->type() == QEvent::MouseMove) {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        handleMouseEvent(mouseEvent);
        return false;
    } else if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        handleMouseEvent(mouseEvent);
        return false;
    } else if (event->type() == QEvent::MouseButtonDblClick) {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        handleMouseEvent(mouseEvent);
        return false;
    } else if (event->type() == QEvent::Leave) {
        mRenderer->highlightLeftSide(false);
        mRenderer->highlightRightSide(false);
        return false;
    }

    return QObject::eventFilter(obj, event);
}

void AlbumController::handleKeyEvent(QKeyEvent* event)
{
    switch (event->key()) {
    case Qt::Key_Down:
        mCurrentModel->moveDown();
        break;
    case Qt::Key_Up:
        mCurrentModel->moveUp();
        break;
    case Qt::Key_Left:
        mCurrentModel->moveLeft();
        break;
    case Qt::Key_Right:
        mCurrentModel->moveRight();
        break;
    case Qt::Key_Return: {
        if (mState == CONNECTION_LOCAL) {
            QFileInfo inf(mCurrentModel->currentSelectedFile());
            const QList<AlbumEntity*>& prevAlbums = mCurrentModel->currentAlbums();
            if (inf.isFile()) {
                mRenderer->unloadAlbumEntityImages(prevAlbums);
                if (inf.suffix().compare("web") == 0) {
                    QFile file(mCurrentModel->currentSelectedFile());
                    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
                        return;
                    QTextStream in(&file);
                    emit openWebview(in.readLine());
                    file.close();
                } else
                    emit openFile(mCurrentModel->currentSelectedFile(), inf.fileName(), "");
            } else {
                int moveInState = mCurrentModel->moveIn();
                if (moveInState == MOVEIN_FAILED)
                    std::cout << "AlbumController: Directory empty\n";
                else {
                    mRenderer->unloadAlbumEntityImages(prevAlbums);
                    mRenderer->loadAlbumEntityImages(mCurrentModel->currentAlbums());
                }
            }
        } else
            sendMoveInRequest();
        break;
    }
    case Qt::Key_Backspace: {
        if (mState == CONNECTION_LOCAL) {
            mRenderer->unloadAlbumEntityImages(mCurrentModel->currentAlbums());
            mCurrentModel->moveOut();
            mRenderer->loadAlbumEntityImages(mCurrentModel->currentAlbums());
        } else
            sendMoveOutRequest();
        break;
    }
    case Qt::Key_Back: {
        if (mState == CONNECTION_LOCAL) {
            mRenderer->unloadAlbumEntityImages(mCurrentModel->currentAlbums());
            mCurrentModel->moveOut();
            mRenderer->loadAlbumEntityImages(mCurrentModel->currentAlbums());
        } else
            sendMoveOutRequest();
        break;
    } break;
    default:
        break;
    }
}

void AlbumController::handleMouseEvent(QMouseEvent* mouseEvent)
{
    QPoint mousePos = mouseEvent->pos();
    mousePos.setY(mRenderer->height() - mousePos.y());
    AlbumAlignment albumAlignment = mCurrentModel->albumAlignment();
    int startX = albumAlignment.lMargin;
    int startY = mRenderer->height() - albumAlignment.uMargin;
    int entityWidth = (mRenderer->width() - albumAlignment.lMargin - albumAlignment.rMargin - ((albumAlignment.maxI - 1) * albumAlignment.xSpacing)) / albumAlignment.maxI;
    int entityHeight = (mRenderer->height() - albumAlignment.uMargin - albumAlignment.dMargin - ((albumAlignment.maxJ - 1) * albumAlignment.ySpacing)) / albumAlignment.maxJ;

    bool hovered = false;
    int visibleEntities = albumAlignment.maxI * albumAlignment.maxJ;
    int startEntity = mCurrentModel->currentPage() * visibleEntities;
    int stopEntity = startEntity + visibleEntities;
    if (stopEntity > mCurrentModel->currentAlbums().size())
        stopEntity = mCurrentModel->currentAlbums().size();

    int entity = startEntity;
    int x = startX;
    int y = startY;
    hovered = false;
    while (entity < stopEntity) {
        for (int i = 0; i < albumAlignment.maxI && entity < stopEntity; i++) {
            if (mousePos.x() >= x && mousePos.x() <= x + entityWidth)
                if (mousePos.y() <= y && mousePos.y() >= y - entityHeight) {
                    hovered = true;
                    if (mouseEvent->button() == Qt::NoButton)
                        mCurrentModel->setSelectedEntity(entity);
                    else if (mouseEvent->button() == Qt::LeftButton) {
                        QApplication::sendEvent(mRenderer, new QKeyEvent(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier));
                        //return;
                    }
                }
            x += entityWidth + albumAlignment.xSpacing;
            entity++;
        }
        x = startX;
        y -= entityHeight + albumAlignment.ySpacing;
    }

    if (mouseEvent->button() == Qt::RightButton)
        QApplication::sendEvent(mRenderer, new QKeyEvent(QEvent::KeyPress, Qt::Key_Backspace, Qt::NoModifier));

    if (hovered && !mSetArrowCursor) {
        if (mRenderer->cursor().shape() != Qt::PointingHandCursor)
            mRenderer->setCursor(Qt::PointingHandCursor);
    } else if (mRenderer->cursor().shape() != Qt::ArrowCursor)
        mRenderer->setCursor(Qt::ArrowCursor);

    if (mCurrentModel->numberOfPages() > 1) {
        if (mousePos.x() > mRenderer->width() - albumAlignment.rMargin) {
            if (mouseEvent->button() == Qt::LeftButton)
                mCurrentModel->pageUp();
            mRenderer->highlightRightSide(true);
        } else
            mRenderer->highlightRightSide(false);
        if (mousePos.x() < albumAlignment.lMargin) {
            if (mouseEvent->button() == Qt::LeftButton)
                mCurrentModel->pageDown();
            mRenderer->highlightLeftSide(true);
        } else
            mRenderer->highlightLeftSide(false);
    } else {
        mRenderer->highlightLeftSide(false);
        mRenderer->highlightRightSide(false);
    }
}

void AlbumController::generateThumbnail()
{
    const QString& file = mCurrentModel->currentSelectedFile();
    if (mState == CONNECTION_LOCAL) {
        srand(time(NULL));
        int variance = (rand() % 25);
        mThumbnailExtractor.extractThumbnail(file, mThumbnailExtractTime + variance);
    } else {
        // TODO: send thumbnailextract msg
    }
}

void AlbumController::generateThumbnailView()
{
    QStringList files;
    const QList<AlbumEntity*>& entities = mCurrentModel->currentAlbums();
    for (int i = 0; i < entities.size(); i++)
        files << entities[i]->fileName();
    if (mState == CONNECTION_LOCAL) {
        srand(time(NULL));
        int variance = (rand() % 25);
        mThumbnailExtractor.extractThumbnails(files, mThumbnailExtractTime + variance);
    } else {
        // TODO: send thumbnailextract msg
    }
}

void AlbumController::generateThumbnailViewSub()
{
    // TODO
}

void AlbumController::thumbnailsExtracted()
{
    mRenderer->unloadAlbumEntityImages(mCurrentModel->currentAlbums());
    mRenderer->loadAlbumEntityImages(mCurrentModel->currentAlbums());
}
