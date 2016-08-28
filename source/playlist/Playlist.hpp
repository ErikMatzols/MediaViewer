#ifndef PLAYLIST_HPP
#define PLAYLIST_HPP

#include "CustomWindow.hpp"
#include <QDir>
#include <QListWidgetItem>
#include <QTableWidgetItem>
#include <QWidget>

class PlaylistTableWidget;
class QMenu;
class QPushButton;

class Playlist : public CustomWindow {
    Q_OBJECT
public:
    Playlist(QWidget* parent, const QString& fileFilter);
    ~Playlist();

    const QString* prevFile();
    const QString* currentFile();
    const QString* nextFile();

    void setFileFilter(const QString& filter);

signals:
    void openFile(const QString& fileName);

public slots:
    void clearPlaylist();
    void addFilesInDirectory(QDir dir);

protected:
    void setupUI();
    //void dragEnterEvent(QDragEnterEvent *event);
    //void dropEvent(QDropEvent *event);

private slots:
    void addMenuPopup();
    void removeMenuPopup();

    void addFiles();
    void removeFiles();
    void addDirectory();
    void addURL();
    void itemActivated(QTableWidgetItem* item);

private:
    //QTableWidget *mTableWidget;
    PlaylistTableWidget* mTableWidget;

    QPushButton* mAddButton;
    QPushButton* mRemoveButton;
    QMenu* mAddMenu;
    QMenu* mRemoveMenu;

    QString mFileFilter;
    QString mPreviousPathDialog;
};

class PlaylistTableItem : public QTableWidgetItem {
public:
    PlaylistTableItem(const QString& fileName, const QString& title, int duration);
    PlaylistTableItem();

    const QString& getFileName();
    int getDuration();

protected:
private:
    QString mFileName;
    int mDuration;
};

#endif
