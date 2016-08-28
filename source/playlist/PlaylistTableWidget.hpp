#ifndef PLAYLISTTABLEWIDGET_HPP
#define PLAYLISTTABLEWIDGET_HPP

#include <QTableWidget>

class QDragEnterEvent;
class QDragMoveEvent;
class QDropEvent;
class Playlist;

class PlaylistTableWidget : public QTableWidget {
    Q_OBJECT
public:
    PlaylistTableWidget(Playlist* parent);
    ~PlaylistTableWidget();

protected:
    void dragEnterEvent(QDragEnterEvent* event);
    void dragMoveEvent(QDragMoveEvent* event);
    void dropEvent(QDropEvent* event);

private:
    Playlist* mParent;
    bool mDraggingToItself;
};

#endif
