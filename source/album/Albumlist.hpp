#ifndef ALBUMLIST_HPP
#define ALBUMLIST_HPP

#include "CustomWindow.hpp"
#include <QListWidgetItem>
class QListWidget;
class AlbumController;
class QPushButton;
class QMenu;

class Albumlist : public CustomWindow {
    Q_OBJECT

public:
    Albumlist(QWidget* parent, AlbumController* albumController, const QString& fileFilter);
    ~Albumlist();

    void setFileFilter(const QString& fileFilter);
    void addItem(const QString& path);

protected:
    void setupUI();
    void dragEnterEvent(QDragEnterEvent* event);
    void dropEvent(QDropEvent* event);

private slots:
    void addItem();
    void removeItem();
    void removeAllItems();
    void removeMenuPopup();

private:
    AlbumController* mAlbumController;
    QListWidget* mListWidget;
    QString mFileFilter;
    QString mPreviousDirectoryDialog;

    QPushButton* mRemoveButton;
    QMenu* mRemoveMenu;
};

class AlbumlistItem : public QListWidgetItem {
public:
    AlbumlistItem(const QString& fileName, const QString& title, QListWidget* listWidget);
    ~AlbumlistItem();

    const QString& getFileName();

protected:
private:
    QString mFileName;
};

#endif
