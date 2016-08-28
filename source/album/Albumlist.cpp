#include "Albumlist.hpp"
#include "AlbumController.hpp"
#include "AlbumEntity.hpp"
#include "Defines.hpp"
#include "MainWindow.hpp"
#include "Playlist.hpp"
#include <QDir>
#include <QFileDialog>
#include <QListWidget>
#include <QMenu>
#include <QMimeData>
#include <QPushButton>
#include <QUrl>
#include <QVBoxLayout>
#include <iostream>

Albumlist::Albumlist(QWidget* parent, AlbumController* albumController, const QString& fileFilter)
    : CustomWindow(parent)
    , mAlbumController(albumController)
    , mFileFilter(fileFilter)
{
    setupUI();
    setTitle("Albumlist");
    setWindowTitle("Albumlist");
#ifndef NO_IMAGE_ALLOC
    setIcon("../resources/Images/title16.png");
    setWindowIcon(QIcon("../resources/Images/title16.png"));
#endif
    resize(320, 200);
}

Albumlist::~Albumlist()
{
}

void Albumlist::setFileFilter(const QString& fileFilter)
{
    mFileFilter = fileFilter;
}

void Albumlist::setupUI()
{
    QWidget* mainWidget = new QWidget;

    QVBoxLayout* vLayout = new QVBoxLayout(mainWidget);
    mListWidget = new QListWidget(mainWidget);
    mListWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    QWidget* lowerWidget = new QWidget(mainWidget);

    vLayout->addWidget(mListWidget);
    vLayout->addWidget(lowerWidget);
    vLayout->setMargin(0);
    vLayout->setSpacing(0);

    QHBoxLayout* hLayout = new QHBoxLayout(lowerWidget);

    QPushButton* addButton = new QPushButton("Add", lowerWidget);
    mRemoveButton = new QPushButton("Remove", lowerWidget);

    mRemoveMenu = new QMenu(mRemoveButton);
    mRemoveMenu->addAction("Remove Selected Album", this, SLOT(removeItem()));
    mRemoveMenu->addAction("Clear Albumlist", this, SLOT(removeAllItems()));

    QSpacerItem* hSpacer1 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    QSpacerItem* hSpacer2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    connect(addButton, SIGNAL(clicked()), this, SLOT(addItem()));
    connect(mRemoveButton, SIGNAL(clicked()), this, SLOT(removeMenuPopup()));

    hLayout->addItem(hSpacer1);
    hLayout->addWidget(addButton);
    hLayout->addWidget(mRemoveButton);
    hLayout->addItem(hSpacer2);

    setAcceptDrops(true);
    setMainWidget(mainWidget);

    setStyleSheet(MainWindow::loadStyleSheet("../style/AlbumlistStyle.css"));
}

void Albumlist::addItem(const QString& path)
{
    AlbumEntity* album = new AlbumEntity;
    QDir directory;
    directory.setPath(path);
    album->createAlbum(NULL, directory, mFileFilter);
    if (mAlbumController->addAlbum(album)) {
        QStringList lst = path.split("/");
        new AlbumlistItem(path, lst.last(), mListWidget);
    } else
        delete album;
}

void Albumlist::addItem()
{
    QString path = QFileDialog::getExistingDirectory(this, "Directory", mPreviousDirectoryDialog);
    if (path.isNull() == false) {
        AlbumEntity* album = new AlbumEntity;
        QDir directory(path);
        mPreviousDirectoryDialog = path;
        album->createAlbum(NULL, directory, mFileFilter);
        if (mAlbumController->addAlbum(album)) {
            QStringList lst = path.split("/");
            new AlbumlistItem(path, lst.last(), mListWidget);
        } else
            delete album;
    }
}

void Albumlist::removeItem()
{
    QListWidgetItem* item = mListWidget->currentItem();
    int row = mListWidget->currentRow();
    delete item;
    mAlbumController->removeAlbum(row);
}

void Albumlist::removeAllItems()
{
    mListWidget->clear();
    mAlbumController->removeAllAlbums();
}

void Albumlist::removeMenuPopup()
{
    mRemoveMenu->popup(mRemoveButton->mapToGlobal(QPoint(0, mRemoveButton->height())));
}

void Albumlist::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasFormat("text/uri-list"))
        event->acceptProposedAction();
}

void Albumlist::dropEvent(QDropEvent* event)
{
    // TODO: fix filter
    QList<QUrl> urlList;
    QString fName;
    QFileInfo info;

    if (event->mimeData()->hasUrls()) {
        urlList = event->mimeData()->urls();
        for (int i = 0; i < urlList.size(); i++) {
            fName = urlList[i].toLocalFile();
            QFileInfo info(fName);
            if (info.isDir())
                addItem(fName);
        }
    }
    event->acceptProposedAction();
}

AlbumlistItem::AlbumlistItem(const QString& fileName, const QString& name, QListWidget* listWidget)
    : QListWidgetItem(name, listWidget)
{
    mFileName = fileName;
}

AlbumlistItem::~AlbumlistItem()
{
}

const QString& AlbumlistItem::getFileName()
{
    return mFileName;
}
