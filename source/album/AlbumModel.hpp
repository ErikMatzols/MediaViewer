#ifndef ALBUMMODEL_HPP
#define ALBUMMODEL_HPP

#include "AlbumEntity.hpp"
#include <QList>
#include <QStack>
#include <QString>
#include <cmath>

//enum AlbumType {ALBUM_FILE, ALBUM_FOLDER};
enum MoveInState { MOVEIN_SUCCESS,
    MOVEIN_FAILED };
struct DirectoryView {
    int pages;
    int currentPage;
    int selectedEntity;
    QList<AlbumEntity*>* currentAlbums;
    QString currentDirectoryName;
};

class AlbumModel {
public:
    AlbumModel()
    {
        mCurrentView.pages = 0;
        mCurrentView.currentPage = 0;
        mCurrentView.selectedEntity = 0;
        mCurrentView.currentAlbums = &mAlbums;
        mCurrentView.currentDirectoryName = "";
        setDimension(3, 3);
        setSpacing(25, 25);
        setMargin(40, 40, 40, 40);
    }

    ~AlbumModel()
    {
        for (int i = 0; i < mAlbums.size(); i++)
            delete mAlbums[i];
        mAlbums.clear();
        mDirectoryStack.clear();
    }

    void addAlbum(AlbumEntity* entity)
    {
        mDirectoryStack.clear();
        mAlbums.append(entity);
        mCurrentView.currentAlbums = &mAlbums;
        mCurrentView.currentDirectoryName = "";
        mCurrentView.pages = ceil(mAlbums.size() / (float)(mAlbumAlignment.maxI * mAlbumAlignment.maxJ));
        mCurrentView.currentPage = 0;
        mCurrentView.selectedEntity = 0;
    }

    void removeAlbum(int index)
    {
        AlbumEntity* entity = mAlbums.takeAt(index);
        delete entity;
        mDirectoryStack.clear();
        mCurrentView.currentAlbums = &mAlbums;
        mCurrentView.currentDirectoryName = "";
        mCurrentView.pages = ceil(mAlbums.size() / (float)(mAlbumAlignment.maxI * mAlbumAlignment.maxJ));
        mCurrentView.currentPage = 0;
        mCurrentView.selectedEntity = 0;
    }

    void clearAlbums()
    {
        for (int i = 0; i < mAlbums.size(); i++)
            delete mAlbums[i];
        mAlbums.clear();
        mDirectoryStack.clear();
        mCurrentView.currentAlbums = &mAlbums;
        mCurrentView.currentDirectoryName = "";
        mCurrentView.pages = ceil(mAlbums.size() / (float)(mAlbumAlignment.maxI * mAlbumAlignment.maxJ));
        mCurrentView.currentPage = 0;
        mCurrentView.selectedEntity = 0;
    }

    void setDimension(int i, int j)
    {
        mAlbumAlignment.maxI = i;
        mAlbumAlignment.maxJ = j;
    }

    void setMargin(int u, int d, int l, int r)
    {
        mAlbumAlignment.uMargin = u;
        mAlbumAlignment.dMargin = d;
        mAlbumAlignment.lMargin = l;
        mAlbumAlignment.rMargin = r;
    }

    void setSpacing(int x, int y)
    {
        mAlbumAlignment.xSpacing = x;
        mAlbumAlignment.ySpacing = y;
    }

    AlbumAlignment albumAlignment()
    {
        return mAlbumAlignment;
    }

    const QList<AlbumEntity*>& currentAlbums()
    {
        return *mCurrentView.currentAlbums;
    }

    const QList<AlbumEntity*>& albumRoot()
    {
        return mAlbums;
    }

    int numberOfPages()
    {
        return mCurrentView.pages;
    }

    int currentPage()
    {
        return mCurrentView.currentPage;
    }

    int selectedEntity()
    {
        return mCurrentView.selectedEntity;
    }

    void setSelectedEntity(int select)
    {
        mCurrentView.selectedEntity = select;
    }

    void moveDown()
    {
        int maxi = mAlbumAlignment.maxI;
        int maxj = mAlbumAlignment.maxJ;
        if (mCurrentView.selectedEntity + maxi < mCurrentView.currentAlbums->size()) {
            int lastLineIndex = (mCurrentView.currentPage * maxi * maxj) + (maxi * (maxj - 1));
            if (mCurrentView.selectedEntity < lastLineIndex)
                mCurrentView.selectedEntity += maxi;
        }
    }

    void moveUp()
    {
        int maxi = mAlbumAlignment.maxI;
        int maxj = mAlbumAlignment.maxJ;
        int firstLineIndex = (mCurrentView.currentPage * maxi * maxj);
        if (mCurrentView.selectedEntity - maxi >= firstLineIndex)
            mCurrentView.selectedEntity -= maxi;
    }

    void moveLeft()
    {
        if (mCurrentView.selectedEntity == 0)
            mCurrentView.selectedEntity = mCurrentView.currentAlbums->size() - 1;
        else
            mCurrentView.selectedEntity--;
        mCurrentView.currentPage = mCurrentView.selectedEntity / (mAlbumAlignment.maxI * mAlbumAlignment.maxJ);
    }

    void moveRight()
    {
        if (mCurrentView.selectedEntity == mCurrentView.currentAlbums->size() - 1)
            mCurrentView.selectedEntity = 0;
        else
            mCurrentView.selectedEntity++;
        mCurrentView.currentPage = mCurrentView.selectedEntity / (mAlbumAlignment.maxI * mAlbumAlignment.maxJ);
    }

    int moveIn()
    {
        if ((*mCurrentView.currentAlbums)[mCurrentView.selectedEntity]->getChildren()->size()) {
            mDirectoryStack.push(mCurrentView);
            QList<AlbumEntity*>* childList = const_cast<QList<AlbumEntity*>*>((*mCurrentView.currentAlbums)[mCurrentView.selectedEntity]->getChildren());
            mCurrentView.currentDirectoryName = (*mCurrentView.currentAlbums)[mCurrentView.selectedEntity]->displayName();
            mCurrentView.currentAlbums = childList;
            mCurrentView.pages = ceil(childList->size() / (float)(mAlbumAlignment.maxI * mAlbumAlignment.maxJ));
            mCurrentView.currentPage = 0;
            mCurrentView.selectedEntity = 0;
        } else
            return MOVEIN_FAILED;
        return MOVEIN_SUCCESS;
    }

    void moveOut()
    {
        if (!mDirectoryStack.isEmpty())
            mCurrentView = mDirectoryStack.pop();
    }

    void pageUp()
    {
        mCurrentView.currentPage++;
        if (mCurrentView.currentPage == mCurrentView.pages)
            mCurrentView.currentPage = 0;
        mCurrentView.selectedEntity = mCurrentView.currentPage * mAlbumAlignment.maxI * mAlbumAlignment.maxJ;
    }

    void pageDown()
    {
        mCurrentView.currentPage--;
        if (mCurrentView.currentPage < 0)
            mCurrentView.currentPage = mCurrentView.pages - 1;
        mCurrentView.selectedEntity = mCurrentView.currentPage * mAlbumAlignment.maxI * mAlbumAlignment.maxJ;
    }

    const QString& currentSelectedFile()
    {
        return (*mCurrentView.currentAlbums)[mCurrentView.selectedEntity]->fileName();
    }

    const QString& currentDirectoryName()
    {
        return mCurrentView.currentDirectoryName;
    }

    void setCurrentDirectoryName(const QString& dirName)
    {
        mCurrentView.currentDirectoryName = dirName;
    }

    void setCurrentView(DirectoryView currentView)
    {
        mCurrentView = currentView;
    }

protected:
private:
    AlbumAlignment mAlbumAlignment;
    QList<AlbumEntity*> mAlbums;
    DirectoryView mCurrentView;
    QStack<DirectoryView> mDirectoryStack;
};

#endif
