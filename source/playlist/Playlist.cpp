#include "Playlist.hpp"
#include "Defines.hpp"
#include "Demuxer.hpp"
#include "MainWindow.hpp"
#include "PlaylistTableWidget.hpp"
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QHeaderView>
#include <QMenu>
#include <QPushButton>
#include <QSpacerItem>
#include <QUrl>
#include <QVBoxLayout>
#include <iostream>

Playlist::Playlist(QWidget* parent, const QString& fileFilter)
    : CustomWindow(parent)
{
    mFileFilter = fileFilter;

    setupUI();

    setTitle("Playlist");
    setWindowTitle("Playlist");
#ifndef NO_IMAGE_ALLOC
    setIcon("../resources/Images/title16.png");
    setWindowIcon(QIcon("../resources/Images/title16.png"));
#endif
    resize(320, 200);
}

Playlist::~Playlist()
{
    //std::cout << "Playlist: Destructor() called\n";
}

void Playlist::setupUI()
{
    QWidget* mainWidget = new QWidget;

    QVBoxLayout* vLayout = new QVBoxLayout(mainWidget);

    mTableWidget = new PlaylistTableWidget(this);
    mTableWidget->horizontalHeader()->setStretchLastSection(true);
    mTableWidget->horizontalHeader()->setVisible(false);
    mTableWidget->verticalHeader()->setVisible(false);
    mTableWidget->verticalHeader()->setDefaultSectionSize(15);
    mTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

    mTableWidget->setShowGrid(false);
    mTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    mTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    mTableWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    mTableWidget->setColumnCount(2);
    QTableWidgetItem* item_1 = new QTableWidgetItem("Title");
    mTableWidget->setHorizontalHeaderItem(0, item_1);
    QTableWidgetItem* item_2 = new QTableWidgetItem("Duration");
    mTableWidget->setHorizontalHeaderItem(1, item_2);

    mTableWidget->setColumnWidth(0, 250);
    //mTableWidget->setDragDropMode(QAbstractItemView::DragDrop);
    //mTableWidget->setColumnWidth(1,50);
    //mTableWidget->horizontalHeader()->setResizeMode(0,QHeaderView::Stretch);
    //mTableWidget->horizontalHeader()->setResizeMode(1,QHeaderView::Fixed);

    QWidget* lowerWidget = new QWidget(mainWidget);

    vLayout->addWidget(mTableWidget);
    vLayout->addWidget(lowerWidget);
    vLayout->setMargin(0);
    vLayout->setSpacing(0);

    QHBoxLayout* hLayout = new QHBoxLayout(lowerWidget);

    mAddButton = new QPushButton("Add", this);
    mRemoveButton = new QPushButton("Remove", this);

    mAddMenu = new QMenu(mAddButton);
    mAddMenu->addAction("Add File(s)", this, SLOT(addFiles()));
    mAddMenu->addAction("Add Directory", this, SLOT(addDirectory()));
    mAddMenu->addAction("Add URL", this, SLOT(addURL()));

    mRemoveMenu = new QMenu(mRemoveButton);
    mRemoveMenu->addAction("Remove Selected File(s)", this, SLOT(removeFiles()));
    mRemoveMenu->addAction("Clear Playlist", this, SLOT(clearPlaylist()));

    connect(mAddButton, SIGNAL(clicked()), this, SLOT(addMenuPopup()));
    connect(mRemoveButton, SIGNAL(clicked()), this, SLOT(removeMenuPopup()));
    connect(mTableWidget, SIGNAL(itemActivated(QTableWidgetItem*)), this, SLOT(itemActivated(QTableWidgetItem*)));
    QSpacerItem* hSpacer1 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    QSpacerItem* hSpacer2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    hLayout->addItem(hSpacer1);
    hLayout->addWidget(mAddButton);
    hLayout->addWidget(mRemoveButton);
    hLayout->addItem(hSpacer2);

    mTableWidget->setAcceptDrops(true);
    mTableWidget->setDragEnabled(true);

    setMainWidget(mainWidget);

    setStyleSheet(MainWindow::loadStyleSheet("../style/PlaylistStyle.css"));
}

const QString* Playlist::prevFile()
{
    int prevRow = mTableWidget->currentRow() - 1;
    if (prevRow >= 0) {
        mTableWidget->setCurrentCell(prevRow, 0);
        PlaylistTableItem* item = static_cast<PlaylistTableItem*>(mTableWidget->currentItem());
        return &item->getFileName();
    }
    return NULL;
}

const QString* Playlist::currentFile()
{
    QTableWidgetItem* item = mTableWidget->currentItem();
    if (item) {
        PlaylistTableItem* pItem = static_cast<PlaylistTableItem*>(item);
        return &pItem->getFileName();
    }
    return NULL;
}

const QString* Playlist::nextFile()
{
    int nextRow = mTableWidget->currentRow() + 1;
    if (mTableWidget->rowCount() > nextRow) {
        mTableWidget->setCurrentCell(nextRow, 0);
        PlaylistTableItem* item = static_cast<PlaylistTableItem*>(mTableWidget->currentItem());
        return &item->getFileName();
    }
    return NULL;
}

void Playlist::setFileFilter(const QString& filter)
{
    mFileFilter = filter;
}
/*
void Playlist::dragEnterEvent(QDragEnterEvent *event)
{
    QTableWidgetItem* item = NULL;
    if (event->mimeData()->hasFormat("text/uri-list"))
        event->acceptProposedAction();
}
*/
/*
void Playlist::dropEvent(QDropEvent *event)
{
    // TODO: fix filter 
    QList<QUrl> urlList;
    QString fName;
    QFileInfo info;

    if (event->mimeData()->hasUrls()) {
        urlList = event->mimeData()->urls();
        for (int i = 0; i < urlList.size(); i++){
            fName = urlList[i].toLocalFile();
            QFileInfo info(fName);
            if (info.isFile()) {
                int duration = Demuxer::queryMediaDuration(fName);
                int row = mTableWidget->rowCount() + 1;
                mTableWidget->setRowCount(row);
                mTableWidget->setItem(row-1,0,new PlaylistTableItem(fName,info.completeBaseName(),duration));
                QTableWidgetItem *item = new QTableWidgetItem(QString("%1:%2").arg(duration / 60, 2, 10, QChar('0')).arg(duration % 60, 2, 10, QChar('0')));
                item->setTextAlignment(Qt::AlignRight);
                mTableWidget->setItem(row-1,1,item);
            }
            else
                addFilesInDirectory(QDir(fName));
        }
    }
    event->acceptProposedAction();
}
*/
void Playlist::clearPlaylist()
{
    mTableWidget->clearContents();
    mTableWidget->setRowCount(0);
}

void Playlist::addFilesInDirectory(QDir dir)
{
    dir.setNameFilters(mFileFilter.split(" "));
    QStringList lst = dir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    QStringList::const_iterator iter;
    for (iter = lst.constBegin(); iter != lst.constEnd(); ++iter) {
        QFileInfo info(dir.absolutePath() + "/" + *iter);
        if (info.isFile()) {
            int row = mTableWidget->rowCount() + 1;
            mTableWidget->setRowCount(row);
            int duration = Demuxer::queryMediaDuration(info.absoluteFilePath());
            mTableWidget->setItem(row - 1, 0, new PlaylistTableItem(info.absoluteFilePath(), info.completeBaseName(), duration));
            QTableWidgetItem* item = new QTableWidgetItem(QString("%1:%2").arg(duration / 60, 2, 10, QChar('0')).arg(duration % 60, 2, 10, QChar('0')));
            item->setTextAlignment(Qt::AlignRight);
            mTableWidget->setItem(row - 1, 1, item);
        } else if (info.isDir()) {
            addFilesInDirectory(info.absoluteDir());
        }
    }
}

void Playlist::addMenuPopup()
{
    mAddMenu->popup(mAddButton->mapToGlobal(QPoint(0, mAddButton->height())));
}

void Playlist::removeMenuPopup()
{
    mRemoveMenu->popup(mRemoveButton->mapToGlobal(QPoint(0, mRemoveButton->height())));
}

void Playlist::addFiles()
{
    QString filter = "(" + mFileFilter + ")";
    QStringList fileNames = QFileDialog::getOpenFileNames(this, "Add File(s)", mPreviousPathDialog, filter);
    int row = mTableWidget->rowCount();
    if (fileNames.count()) {
        QFileInfo info(fileNames[0]);
        mPreviousPathDialog = info.path();
        mTableWidget->setRowCount(row + fileNames.count());
    }

    for (int i = 0; i < fileNames.size(); i++) {
        QFileInfo info(fileNames[i]);
        int duration = Demuxer::queryMediaDuration(fileNames[i]);
        mTableWidget->setItem(row + i, 0, new PlaylistTableItem(fileNames[i], info.completeBaseName(), duration));

        QTableWidgetItem* item = new QTableWidgetItem(QString("%1:%2").arg(duration / 60, 2, 10, QChar('0')).arg(duration % 60, 2, 10, QChar('0')));

        item->setTextAlignment(Qt::AlignRight);
        mTableWidget->setItem(row + i, 1, item);
    }
}

void Playlist::removeFiles()
{
    QList<QTableWidgetItem*> items = mTableWidget->selectedItems();
    for (int i = 0; i < items.size(); i += 2) {
        int row = items.at(i)->row();
        mTableWidget->removeRow(row);
    }
}

void Playlist::addDirectory()
{
    QString directory = QFileDialog::getExistingDirectory(this, "Add Directory", mPreviousPathDialog);
    if (directory.size()) {
        mPreviousPathDialog = directory;
        QDir dir = QDir(directory);
        addFilesInDirectory(dir);
    }
}

void Playlist::addURL() //TODO: implement function
{
    std::cout << "addURL\n";
}

void Playlist::itemActivated(QTableWidgetItem* item)
{
    int currentRow = item->tableWidget()->currentRow();
    QTableWidgetItem* i = mTableWidget->item(currentRow, 0);
    if (i) {
        PlaylistTableItem* ti = static_cast<PlaylistTableItem*>(i);
        emit openFile(ti->getFileName());
    }
}

PlaylistTableItem::PlaylistTableItem(const QString& fileName, const QString& title, int duration)
    : QTableWidgetItem(title)
{
    mFileName = fileName;
    mDuration = duration;
}

PlaylistTableItem::PlaylistTableItem()
{
}

const QString& PlaylistTableItem::getFileName()
{
    return mFileName;
}

int PlaylistTableItem::getDuration()
{
    return mDuration;
}
