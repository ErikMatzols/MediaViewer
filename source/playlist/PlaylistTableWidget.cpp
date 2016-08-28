#include "PlaylistTableWidget.hpp"
#include "Demuxer.hpp"
#include "Playlist.hpp"
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>

PlaylistTableWidget::PlaylistTableWidget(Playlist* parent)
    : QTableWidget(parent)
{
    mParent = parent;
    mDraggingToItself = false;
}

PlaylistTableWidget::~PlaylistTableWidget()
{
}

void PlaylistTableWidget::dragEnterEvent(QDragEnterEvent* event)
{
    //printf("tablewidget dragEnterEvent\n");
    if (event->mimeData()->hasFormat("text/uri-list")) {
        //printf("event accepted\n");
        event->acceptProposedAction();
    } else if (event->source() == this) {
        //printf("dragging to itself\n");
        mDraggingToItself = true;
        event->acceptProposedAction();
    }
}

void PlaylistTableWidget::dragMoveEvent(QDragMoveEvent*)
{
    //if (mDraggingToItself) {
    //    QTableWidgetItem *item = itemAt(event->pos());
    //    if (item) {
    //        int r = row(item);
    //printf("row %d\n",r);
    //    }
    //}
}

void PlaylistTableWidget::dropEvent(QDropEvent* event)
{
    //printf("tablewidget dropEvent\n");
    // TODO: fix filter
    QList<QUrl> urlList;
    QString fName;
    QFileInfo info;
    if (mDraggingToItself) {
        QList<QTableWidgetItem*> lst = selectedItems();

        QTableWidgetItem* itm = itemAt(event->pos());
        if (itm) {
            int r = row(itm);
            //PlaylistTableItem *item1 = static_cast<PlaylistTableItem*>(item(r,0));
            //QTableWidgetItem *item2 = item(r,1);

            //QString fileName = item1->getFileName();
            //int duration = item1->getDuration();

            for (int i = 0; i < lst.count(); i += 2) {
                PlaylistTableItem* selItem = static_cast<PlaylistTableItem*>(lst[i]);
                QString fileName = selItem->getFileName();
                QString title = selItem->text();
                QString durationTxt = lst[i + 1]->text();
                int duration = selItem->getDuration();
                removeRow(selItem->row());
                insertRow(r);
                setItem(r, 0, new PlaylistTableItem(fileName, title, duration));
                QTableWidgetItem* dg = new QTableWidgetItem(durationTxt);
                dg->setTextAlignment(Qt::AlignRight);
                setItem(r, 1, dg);
            }
        }

        mDraggingToItself = false;
        return;
    }
    if (event->mimeData()->hasUrls()) {
        urlList = event->mimeData()->urls();
        for (int i = 0; i < urlList.size(); i++) {
            fName = urlList[i].toLocalFile();
            QFileInfo info(fName);
            if (info.isFile()) {
                int duration = Demuxer::queryMediaDuration(fName);
                int row = rowCount() + 1;
                setRowCount(row);
                setItem(row - 1, 0, new PlaylistTableItem(fName, info.completeBaseName(), duration));
                QTableWidgetItem* item = new QTableWidgetItem(QString("%1:%2").arg(duration / 60, 2, 10, QChar('0')).arg(duration % 60, 2, 10, QChar('0')));
                item->setTextAlignment(Qt::AlignRight);
                setItem(row - 1, 1, item);
            } else
                mParent->addFilesInDirectory(QDir(fName));
        }
    }
    event->acceptProposedAction();
}
