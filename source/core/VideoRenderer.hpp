#ifndef VIDEORENDERER_HPP
#define VIDEORENDERER_HPP

#include <QGLWidget>
#include <QPoint>
#include <QTimer>

#include "Demuxer.hpp"
#include "StreamState.hpp"

enum RenderState { RENDER_LOGO,
    RENDER_VIDEO,
    RENDER_ALBUM };

struct FontColor {
    float r, g, b;
};

struct RenderImage {
    int w;
    int h;
    uint8_t* data[AV_NUM_DATA_POINTERS];
    int linesize[AV_NUM_DATA_POINTERS];
    enum AVPixelFormat format;
    QString filename;
};

Q_DECLARE_METATYPE(RenderImage)

class FTBufferFont;
class GLProgram;
class GLShader;
class PixelFormatConverter;
class AlbumModel;
class AlbumEntity;

class VideoRenderer : public QGLWidget {
    Q_OBJECT
public:
    VideoRenderer(QWidget* parent);
    ~VideoRenderer();

    void openAlbumRenderer(int width, int height);
    void closeAlbumRenderer();

    void openVideoRenderer(int width, int height, AVPixelFormat format, float aspect);
    void closeVideoRenderer();

    void renderVideo(uint8_t* data[4], int linesize[4], Mux* state);
    void renderSubtitle(const QStringList& subtitleText, double duration);

    void updateOverlayText(const QString& overlayText);

    void changeAspectRatio(float aspect);

    void cornerDefault();

    void resizeOnPlay(bool resize);
    void keepAspectRatio(bool keep);

    bool getKeepAspectRatio()
    {
        return mKeepAspectRatio;
    }

    void setFullScreen(bool fullscreen);

    void getOverlayFont(QString& fontName, QString& fontColor, int& fontSize,
        QPoint& fontPos, float& fadeTime);
    void setOverlayFont(const QString& fontName, const QString& fontColor,
        int fontSize, QPoint pos, float fadeTime);
    void getSubtitleFont(QString& fontName, QString& fontColor, int& fontSize,
        QPoint& fontPos);
    void setSubtitleFont(const QString& fontName, const QString& fontColor,
        int fontSize, QPoint pos);

    void changeVideoFilterPreset(const QStringList& filterChain);
    QList<QString> retreiveFilters();

    void changeStretchPreset(QVector4D preset);

    bool loadSettingsBinary(const QString& fileName);
    bool saveSettingsBinary(const QString& fileName);

    void setColorFilterEnabled(bool enabled);

    void setBrightness(float b);
    void setContrast(float c);

    void setModel(AlbumModel* model);

    void highlightRightSide(bool highlight);
    void highlightLeftSide(bool higlight);

    float scrollSpeed();
    void setScrollSpeed(float scrollspeed);

    GLuint getDefaultTexture();
    void loadAlbumEntityImages(const QList<AlbumEntity*>& albums);
    void unloadAlbumEntityImages(const QList<AlbumEntity*>& albums);

    void loadAlbumEntityImage(AlbumEntity* album);
    void unloadAlbumEntityImage(AlbumEntity* album);

    unsigned int loadImage(const QString& file);
    unsigned int loadImage(const QImage& image);

    int screenWidth()
    {
        return mScreenWidth;
    }

    int screenHeight()
    {
        return mScreenHeight;
    }

    bool isVideoRendererOpen()
    {
        return isOpen;
    }

    void setDemuxer(Demuxer* demuxer);

    void aquireMutex() { mMutex.lock(); }
    void releaseMutex() { mMutex.unlock(); }

public slots:
    void renderToFile(RenderImage renderImage);
    void renderAlbums();

    void render();

    void moveContextToThread(void* thread);

signals:
    void resizeWindow(int w, int h);

protected:
    void paintEvent(QPaintEvent* evt);
    void resizeEvent(QResizeEvent* evt);
    void closeEvent(QCloseEvent* evt);

    void textToRGB(const QString& color, float& r, float& g, float& b);

    void renderAlbumEntity(int entity, QPointF topLeft, QPointF bottomRight);

private:
    int mVideoWidth, mVideoHeight;
    int mScreenWidth, mScreenHeight;
    float mLeftX, mRightX, mTopY, mBottomY;
    float mLeftStretch, mRightStretch, mTopStretch, mBottomStretch;
    float mAspectRatio;
    bool mResizeOnPlay, mKeepAspectRatio, mFullScreen;
    int mNrOfFilters;
    float mBrightness, mContrast;
    RenderState mRenderState;
    QMutex mMutex;

    FTBufferFont* mOverlayFont;
    QString mOverlayFontName, mOverlayFontColor;
    int mOverlayFontSize;
    QPoint mOverlayFontPos;
    FontColor mOverlayFontRGB;

    FTBufferFont* mSubtitleFont;
    QString mSubtitleFontName, mSubtitleFontColor;
    int mSubtitleFontSize;
    QPoint mSubtitleFontPos;
    FontColor mSubtitleFontRGB;

    QString mOverlayText;
    Timer mOverlayFadeTimer;
    float mOverlayFadeTime;
    QStringList mSubtitleText;
    Timer mSubtitleFadeTimer;
    float mSubtitleFadeTime;

    PixelFormatConverter* mConverter;

    GLuint mVideoTex[2];
    GLuint mFBOVideo;

    QList<GLProgram*> mPrograms;
    QList<QString> mProgramName;
    QList<GLShader*> mFragmentShaders;
    GLShader* mVertexShader;

    bool mChangeFilterPrograms;
    QList<GLProgram*> mNextFilterPrograms;
    QList<GLProgram*> mCurrentFilterPrograms;

    bool mUpdateColorFilter;
    bool mColorFilterEnabled;
    GLProgram* mColorFilterProgram;
    GLShader* mColorFilterFragment;
    GLShader* mColorFilterVertex;

    AlbumModel* m_model;

    FTBufferFont* mEntityFont;
    FTBufferFont* mHeaderFont;
    FTBufferFont* mFooterFont;

    GLuint m_defaultTexture;

    bool mLeftSide;
    bool mRightSide;
    float mScrollSpeed;

    bool isOpen;

    Demuxer* mDemuxer;
    Mux* mStreamState;
    QThread* mOriginalThread;
    QTimer mAlbumTimer;
};

#endif
