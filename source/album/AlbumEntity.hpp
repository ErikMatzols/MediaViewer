#ifndef ALBUMENTITY_HPP
#define ALBUMENTITY_HPP

#include <QDir>
#include <QList>
#include <QString>

struct AlbumAlignment {
    int maxI, maxJ;
    int xSpacing, ySpacing;
    int uMargin, dMargin, lMargin, rMargin;
};

class AlbumEntity {
public:
    AlbumEntity()
        : m_parent(NULL)
    {
    }
    ~AlbumEntity()
    {
        for (int i = 0; i < m_children.size(); i++)
            delete m_children[i];
        m_children.clear();
    }

    void createAlbum(AlbumEntity* parent, QDir directory, const QString& filter)
    {
        QFileInfo dirInfo(directory.absolutePath());

        m_parent = parent;
        m_displayName = dirInfo.completeBaseName();
        m_fileImage = dirInfo.absoluteFilePath() + ".jpg";
        m_fileName = dirInfo.absoluteFilePath();

        directory.setFilter(QDir::AllDirs | QDir::Files | QDir::Hidden | QDir::NoDotAndDotDot);
        directory.setNameFilters(filter.split(" "));

        QFileInfoList entryList = directory.entryInfoList();
        for (int i = 0; i < entryList.size(); i++) {
            QString tmp = entryList[i].absoluteFilePath();
            if (entryList[i].isDir()) {
                AlbumEntity* entity = new AlbumEntity;
                entity->createAlbum(this, QDir(entryList[i].absoluteFilePath()), filter);
                m_children.append(entity);
            } else if (entryList[i].isFile()) {
                AlbumEntity* entity = new AlbumEntity;
                entity->createLeaf(this, entryList[i]);
                m_children.append(entity);
            }
        }
    }

    void createLeaf(AlbumEntity* parent, const QFileInfo& fileInfo)
    {
        m_parent = parent;
        m_displayName = fileInfo.completeBaseName();
        m_fileImage = fileInfo.absolutePath() + "/" + m_displayName + ".jpg";
        m_fileName = fileInfo.absoluteFilePath();
    }

    const AlbumEntity* getParent() const
    {
        return m_parent;
    }

    const QList<AlbumEntity*>* getChildren() const
    {
        return &m_children;
    }

    const QString& displayName() const
    {
        return m_displayName;
    }

    void setDisplayName(const QString& displayName)
    {
        m_displayName = displayName;
    }

    const QString& fileName() const
    {
        return m_fileName;
    }

    void setFileName(const QString& fileName)
    {
        m_fileName = fileName;
    }

    const QString& fileImage() const
    {
        return m_fileImage;
    }

    void setFileImage(const QString& fileImage)
    {
        m_fileImage = fileImage;
    }

    void setImage(unsigned int imageID)
    {
        m_image = imageID;
    }

    unsigned int getImage() const
    {
        return m_image;
    }

protected:
private:
    AlbumEntity* m_parent;
    QList<AlbumEntity*> m_children;

    QString m_displayName;
    QString m_fileName;
    QString m_fileImage;
    unsigned int m_image;
};

#endif
