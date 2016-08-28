#include <GL/glew.h>
#include "VideoRenderer.hpp"
#include "AlbumModel.hpp"
#include "Defines.hpp"
#include "GLProgram.hpp"
#include "GLShader.hpp"
#include "PixelFormatConverter.hpp"
#include "VideoRenderer.hpp"
#include <FTGL/ftgl.h>
#include <QApplication>
#include <QDesktopWidget>
#include <QDir>
#include <QFile>
#include <QGLFramebufferObject>
#include <QImageReader>
#include <QTime>
#include <QVector4D>
#include <iostream>

VideoRenderer::VideoRenderer(QWidget* parent)
    : QGLWidget(QGLFormat(QGL::DoubleBuffer), parent)
{
    makeCurrent();

    setAutoBufferSwap(false);
    isOpen = false;
    mFullScreen = false;
    mResizeOnPlay = true;
    mKeepAspectRatio = false;
    mRenderState = RENDER_LOGO;
    mNrOfFilters = 0;
    cornerDefault();
    mLeftStretch = 0;
    mRightStretch = 0;
    mTopStretch = 0;
    mBottomStretch = 0;
    mUpdateColorFilter = false;
    mColorFilterEnabled = false;
    mBrightness = 0;
    mContrast = 0;
    mLeftSide = false;
    mRightSide = false;
    mScrollSpeed = 1.5f;

    QRect rect = QApplication::desktop()->screenGeometry();

    mScreenWidth = rect.width();
    mScreenHeight = rect.height();

    GLenum err = glewInit();
    if (GLEW_OK != err) {
        std::cout << "GLEW initialization failed!\n";
    }
    std::cout << "GLEW initialization successfull!\n";

    mOverlayFontName = "../resources/Fonts/DejaVuSans.ttf";
    mOverlayFontColor = "Yellow";
    mOverlayFontSize = 26;
    mOverlayFontPos = QPoint(10, 10);
    mOverlayFadeTime = 3.5f;
    mOverlayFontRGB.r = mOverlayFontRGB.g = mOverlayFontRGB.b = 1.0f;
    mOverlayFont = new FTBufferFont("../resources/Fonts/DejaVuSans.ttf");

    mSubtitleFontName = "../resources/Fonts/DejaVuSans.ttf";
    mSubtitleFontColor = "White";
    mSubtitleFontSize = 26;
    mSubtitleFontPos = QPoint(0, 0);
    mSubtitleFontRGB.r = mSubtitleFontRGB.g = mSubtitleFontRGB.b = 1.0f;
    mSubtitleFont = new FTBufferFont("../resources/Fonts/DejaVuSans.ttf");

    mEntityFont = new FTBufferFont("../resources/Fonts/DejaVuSans.ttf");
    mEntityFont->FaceSize(20);
    mHeaderFont = new FTBufferFont("../resources/Fonts/DejaVuSans.ttf");
    mHeaderFont->FaceSize(26);
    mFooterFont = new FTBufferFont("../resources/Fonts/DejaVuSans.ttf");
    mFooterFont->FaceSize(14);

    m_defaultTexture = loadImage("../resources/Images/filmreel.jpg");

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glGenFramebuffersEXT(1, &mFBOVideo);

    glGenTextures(2, &mVideoTex[0]);
    for (int i = 0; i < 2; i++) {
        glBindTexture(GL_TEXTURE_2D, mVideoTex[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }

    mConverter = new PixelFormatConverter;

    mVertexShader = new GLShader(GL_VERTEX_SHADER);
    mVertexShader->load("../Shaders/Main.vert");
    if (!mVertexShader->compile())
        std::cout << "Vertex shader failed to compile!\n";
    mVertexShader->printLog();

    QDir dir("../shaders/Filters");
    QStringList files = dir.entryList(QDir::Files);
    foreach (QString file, files) {
        QString fileName = dir.absolutePath() + "/" + file;
        GLShader* shaderFragment = new GLShader(GL_FRAGMENT_SHADER);
        shaderFragment->load(fileName.toStdString());
        if (!shaderFragment->compile())
            std::cout << file.toStdString().c_str() << " fragment shader failed to compile\n";
        shaderFragment->printLog();
        mFragmentShaders.push_back(shaderFragment);
        GLProgram* shaderProgram = new GLProgram;
        shaderProgram->attachShader(*mVertexShader);
        shaderProgram->attachShader(*shaderFragment);
        if (!shaderProgram->link())
            std::cout << "Program failed to link\n";
        shaderProgram->printLog();
        mPrograms.push_back(shaderProgram);
        mProgramName.push_back(file);
    }

    mColorFilterFragment = new GLShader(GL_FRAGMENT_SHADER);
    mColorFilterFragment->load("../shaders/ColorControl/ColorFilter.frag");
    if (!mColorFilterFragment->compile())
        std::cout << "Colorfilter.frag fragment shader failed to compile!\n";
    mColorFilterFragment->printLog();
    mColorFilterProgram = new GLProgram;
    mColorFilterProgram->attachShader(*mVertexShader);
    mColorFilterProgram->attachShader(*mColorFilterFragment);
    if (!mColorFilterProgram->link())
        std::cout << "Program failed to link!\n";
    mColorFilterProgram->printLog();

    mOriginalThread = thread();
    doneCurrent();

    qRegisterMetaType<RenderImage>();
}

VideoRenderer::~VideoRenderer()
{
    makeCurrent();
    delete mEntityFont;
    delete mHeaderFont;
    delete mFooterFont;
    delete mConverter;
    delete mOverlayFont;
    delete mSubtitleFont;

    glDeleteFramebuffersEXT(1, &mFBOVideo);
    glDeleteTextures(2, &mVideoTex[0]);
    glDeleteTextures(1, &m_defaultTexture);

    for (int i = 0; i < mPrograms.size(); i++) {
        mPrograms[i]->detachShader(*mVertexShader);
        mPrograms[i]->detachShader(*mFragmentShaders[i]);
        delete mFragmentShaders[i];
        delete mPrograms[i];
    }
    mColorFilterProgram->detachShader(*mVertexShader);
    mColorFilterProgram->detachShader(*mColorFilterFragment);
    delete mVertexShader;
    delete mColorFilterFragment;
    delete mColorFilterProgram;
    doneCurrent();
}

void VideoRenderer::openVideoRenderer(int width, int height, enum AVPixelFormat format, float aspect)
{
    makeCurrent();
    isOpen = true;
    mVideoWidth = width;
    mVideoHeight = height;

    changeAspectRatio(aspect);

    for (int i = 0; i < 2; i++) {
        glBindTexture(GL_TEXTURE_2D, mVideoTex[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, mVideoWidth, mVideoHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    }

    mConverter->changeContext(format, width, height);

    for (int i = 0; i < mPrograms.size(); i++) {
        mPrograms[i]->enable();
        mPrograms[i]->uniform1i("sourceTex", 0);
        mPrograms[i]->uniform1f("HFrac", 1 / (float)(mVideoWidth - 1));
        mPrograms[i]->uniform1f("VFrac", 1 / (float)(mVideoHeight - 1));
        mPrograms[i]->disable();
    }

    mColorFilterProgram->enable();
    mColorFilterProgram->uniform1f("brightness", mBrightness);
    mColorFilterProgram->uniform1f("contrast", mContrast);
    mColorFilterProgram->disable();

    if (mOverlayFont)
        delete mOverlayFont;
    mOverlayFont = new FTBufferFont(mOverlayFontName.toStdString().c_str());
    mOverlayFont->FaceSize(mOverlayFontSize);

    if (mSubtitleFont)
        delete mSubtitleFont;
    mSubtitleFont = new FTBufferFont(mSubtitleFontName.toStdString().c_str());
    mSubtitleFont->FaceSize(mSubtitleFontSize);

    mRenderState = RENDER_VIDEO;
    doneCurrent();
}

void VideoRenderer::closeVideoRenderer()
{
    makeCurrent();
    isOpen = false;
    delete mOverlayFont;
    mOverlayFont = NULL;
    delete mSubtitleFont;
    mSubtitleFont = NULL;

    mRenderState = RENDER_LOGO;
    doneCurrent();
    update();
}

void VideoRenderer::changeAspectRatio(float aspect)
{
    mAspectRatio = aspect;
    if (aspect >= 1.76 && aspect < 1.78)
        emit resizeWindow(mVideoHeight * mAspectRatio, mVideoHeight); // aspect as 16:9 anamorphic of source
    else if (aspect >= 1.32 && aspect < 1.34)
        emit resizeWindow(mVideoHeight * mAspectRatio, mVideoHeight); // aspect as 4:3 of source
    else
        emit resizeWindow(mVideoWidth, mVideoHeight); // aspect as source aspect
}

void VideoRenderer::openAlbumRenderer(int width, int height)
{
    std::cout << "VideoRenderer openAlbumRenderer\n";

    emit resizeWindow(width, height);
    mRenderState = RENDER_ALBUM;

    connect(&mAlbumTimer, SIGNAL(timeout()), this, SLOT(renderAlbums()));
    mAlbumTimer.start(16);
}

void VideoRenderer::closeAlbumRenderer()
{
    std::cout << "VideoRenderer closeAlbumRenderer\n";

    if (mRenderState != RENDER_VIDEO) {
        mAlbumTimer.stop();
        mAlbumTimer.disconnect();
        mRenderState = RENDER_LOGO;
        update();
    }
}

void VideoRenderer::renderVideo(uint8_t* data[4], int linesize[4], Mux* state)
{
    makeCurrent();

    if (mChangeFilterPrograms) {
        mCurrentFilterPrograms = mNextFilterPrograms;
        mChangeFilterPrograms = false;
    }

    if (mKeepAspectRatio) {
        float aspect = mAspectRatio;
        if (mAspectRatio >= 0.99 && mAspectRatio < 1.01)
            aspect = mVideoWidth / (float)mVideoHeight;
        float diff_width = (aspect * height()) - width();
        float diff_height = (width() / aspect) - height();
        if (diff_width < 0 || diff_height > 0) {
            // reduce width to fit aspect ratio
            float reduceAmount = (2.0f / width()) * diff_width * 0.5f;
            mLeftX = -1.0f - reduceAmount;
            mRightX = 1.0f + reduceAmount;
            mTopY = 1.0f;
            mBottomY = -1.0f;
        } else {
            // reduce height to fit aspect ratio
            float reduceAmount = (2.0f / height()) * diff_height * 0.5f;
            mTopY = 1.0f + reduceAmount;
            mBottomY = -1.0f - reduceAmount;
            mLeftX = -1.0f;
            mRightX = 1.0f;
        }
    }

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mFBOVideo);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, mVideoTex[0], 0);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_2D, mVideoTex[1], 0);

    GLenum attachments[] = { GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT1_EXT };

    glDrawBuffer(attachments[0]);
    mConverter->renderToRGB(data, linesize, true);

    for (int i = 0; i < mCurrentFilterPrograms.size(); i++) {
        int read = i % 2;
        int write = (i + 1) % 2;

        glDrawBuffer(attachments[write]);
        glBindTexture(GL_TEXTURE_2D, mVideoTex[read]);

        mCurrentFilterPrograms[i]->enable();
        glBegin(GL_QUADS);
        glMultiTexCoord2f(GL_TEXTURE0, 0.0f, 0.0f);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glMultiTexCoord2f(GL_TEXTURE0, 1.0f, 0.0f);
        glVertex3f(mVideoWidth, 0.0f, 0.0f);
        glMultiTexCoord2f(GL_TEXTURE0, 1.0f, 1.0f);
        glVertex3f(mVideoWidth, mVideoHeight, 0.0f);
        glMultiTexCoord2f(GL_TEXTURE0, 0.0f, 1.0f);
        glVertex3f(0.0f, mVideoHeight, 0.0f);
        glEnd();
        mCurrentFilterPrograms[i]->disable();
    }

    int finalTex = mCurrentFilterPrograms.size() % 2;

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

    glViewport(0, 0, width(), height());
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Render offscreen framebuffer to main framebuffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glColor3f(1.0f, 1.0f, 1.0f);
    //glEnable(GL_TEXTURE_2D);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mVideoTex[finalTex]);

    if (mColorFilterEnabled) {
        mColorFilterProgram->enable();
        if (mUpdateColorFilter) {
            mColorFilterProgram->enable();
            mColorFilterProgram->uniform1f("brightness", mBrightness);
            mColorFilterProgram->uniform1f("contrast", mContrast);
            mUpdateColorFilter = false;
        }
        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(mLeftX + mLeftStretch * 0.01f, mBottomY + mBottomStretch * 0.01f, 0.0f);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(mRightX + mRightStretch * 0.01f, mBottomY + mBottomStretch * 0.01f, 0.0f);
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(mRightX + mRightStretch * 0.01f, mTopY + mTopStretch * 0.01f, 0.0f);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(mLeftX + mLeftStretch * 0.01f, mTopY + mTopStretch * 0.01f, 0.0f);
        glEnd();
        mColorFilterProgram->disable();
    } else {
        glEnable(GL_TEXTURE_2D);
        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(mLeftX + mLeftStretch * 0.01f, mBottomY + mBottomStretch * 0.01f, 0.0f);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(mRightX + mRightStretch * 0.01f, mBottomY + mBottomStretch * 0.01f, 0.0f);
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(mRightX + mRightStretch * 0.01f, mTopY + mTopStretch * 0.01f, 0.0f);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(mLeftX + mLeftStretch * 0.01f, mTopY + mTopStretch * 0.01f, 0.0f);
        glEnd();
        glDisable(GL_TEXTURE_2D);
    }

    //glDisable(GL_TEXTURE_2D);

    // Render overlay to desktop resolution
    glViewport(0, 0, width(), height());
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, mScreenWidth, 0, mScreenHeight, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
    if (state && state->mDebug) {
        float overlayFontAsc = mOverlayFont->Ascender();
        float xOverlay = mOverlayFontPos.x();
        float yOverlay = mScreenHeight - overlayFontAsc - mOverlayFontPos.y();

        QStringList strLst;
        strLst << "Video Queue: " + QString::number(state->mVideoQueue.size());
        strLst << "Audio Queue: " + QString::number(state->mAudioQueue.size());
        strLst << "Subtitle Queue: " + QString::number(state->mSubtitleQueue.size());
        strLst << "Master Pts: " + QString::number(state->masterPts);
        strLst << "Video Pts: " + QString::number(state->videoPts);
        strLst << "Audio Pts: " + QString::number(state->audioPts);

        for (int i = 0; i < strLst.size(); i++) {
            float y = yOverlay - overlayFontAsc * (i + 3);
            glColor3f(0.0f, 0.0f, 0.0f);
            mOverlayFont->Render(strLst[i].toUtf8(), -1, FTPoint(xOverlay - 2, y));
            mOverlayFont->Render(strLst[i].toUtf8(), -1, FTPoint(xOverlay + 2, y));
            mOverlayFont->Render(strLst[i].toUtf8(), -1, FTPoint(xOverlay, y - 2));
            mOverlayFont->Render(strLst[i].toUtf8(), -1, FTPoint(xOverlay, y + 2));

            glColor3f(1.0f, 0.0f, 0.0f);
            mOverlayFont->Render(strLst[i].toUtf8(), -1, FTPoint(xOverlay, y));
        }
    }
    if (mOverlayFadeTimer.getTime() < mOverlayFadeTime) {
        float overlayFontAsc = mOverlayFont->Ascender();
        float xOverlay = mOverlayFontPos.x();
        float yOverlay = mScreenHeight - overlayFontAsc - mOverlayFontPos.y();

        glColor3f(0.0f, 0.0f, 0.0f);
        mOverlayFont->Render(mOverlayText.toUtf8(), -1, FTPoint(xOverlay - 2, yOverlay));
        mOverlayFont->Render(mOverlayText.toUtf8(), -1, FTPoint(xOverlay + 2, yOverlay));
        mOverlayFont->Render(mOverlayText.toUtf8(), -1, FTPoint(xOverlay, yOverlay - 2));
        mOverlayFont->Render(mOverlayText.toUtf8(), -1, FTPoint(xOverlay, yOverlay + 2));

        glColor3f(mOverlayFontRGB.r, mOverlayFontRGB.g, mOverlayFontRGB.b);
        mOverlayFont->Render(mOverlayText.toUtf8(), -1, FTPoint(xOverlay, yOverlay));
    }

    if (mSubtitleFadeTimer.getTime() < mSubtitleFadeTime) {
        for (int i = 0; i < mSubtitleText.size(); i++) {
            float subtitleFontAdv = mSubtitleFont->Advance(mSubtitleText[i].toUtf8());
            float subtitleFontAsc = mSubtitleFont->Ascender();
            //float subtitleFontDec = mSubtitleFont->Descender();

            float xSub = mScreenWidth * 0.5f - subtitleFontAdv * 0.5f;
            float ySub = (subtitleFontAsc * mSubtitleText.size()) - (i * subtitleFontAsc);

            glColor3f(0.0f, 0.0f, 0.0f);
            mSubtitleFont->Render(mSubtitleText[i].toUtf8(), -1, FTPoint(xSub - 2, ySub));
            mSubtitleFont->Render(mSubtitleText[i].toUtf8(), -1, FTPoint(xSub + 2, ySub));
            mSubtitleFont->Render(mSubtitleText[i].toUtf8(), -1, FTPoint(xSub, ySub - 2));
            mSubtitleFont->Render(mSubtitleText[i].toUtf8(), -1, FTPoint(xSub, ySub + 2));

            glColor3f(mSubtitleFontRGB.r, mSubtitleFontRGB.g, mSubtitleFontRGB.b);
            mSubtitleFont->Render(mSubtitleText[i].toUtf8(), -1, FTPoint(xSub, ySub));
        }
    }
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);

    swapBuffers();
    doneCurrent();
}

void VideoRenderer::cornerDefault()
{
    mLeftX = -1.0f;
    mRightX = 1.0f;
    mTopY = 1.0f;
    mBottomY = -1.0f;
}

void VideoRenderer::paintEvent(QPaintEvent* /*evt*/)
{
    switch (mRenderState) {
    case RENDER_LOGO:
        makeCurrent();
        glViewport(0, 0, width(), height());
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        swapBuffers();
        doneCurrent();
        break;
    case RENDER_ALBUM:
        renderAlbums();
        break;
    case RENDER_VIDEO:
    default:
        break;
    }
}

void VideoRenderer::renderAlbums()
{
    makeCurrent();

    glViewport(0, 0, width(), height());
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, mScreenWidth, 0, mScreenHeight, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const QList<AlbumEntity*>& albums = m_model->currentAlbums();
    AlbumAlignment alignment = m_model->albumAlignment();

    int visibleEntities = alignment.maxI * alignment.maxJ;
    int currEntity = m_model->currentPage() * visibleEntities;
    int stopEntity = (currEntity + visibleEntities) > albums.size() ? albums.size() : (currEntity + visibleEntities);

    int x = alignment.lMargin;
    int y = mScreenHeight - alignment.uMargin;
    int w = (mScreenWidth - alignment.lMargin - alignment.rMargin - (alignment.maxI - 1) * alignment.xSpacing) / alignment.maxI;
    int h = (mScreenHeight - alignment.uMargin - alignment.dMargin - (alignment.maxJ - 1) * alignment.ySpacing) / alignment.maxJ;

    while (currEntity < stopEntity) {
        for (int i = 0; i < alignment.maxI && currEntity < stopEntity; i++) {
            renderAlbumEntity(currEntity, QPointF(x, y), QPointF(x + w, y - h));
            x += w + alignment.xSpacing;
            currEntity++;
        }
        x = alignment.lMargin;
        y -= h + alignment.ySpacing;
    }

    glColor3f(1.0f, 1.0f, 1.0f);
    if (mLeftSide) {
        glBegin(GL_TRIANGLES);
        glVertex3f(alignment.lMargin - 10, mScreenHeight * 0.5f - 50, 0.0f);
        glVertex3f(alignment.lMargin - 10, mScreenHeight * 0.5f + 50, 0.0f);
        glVertex3f(0.0f, mScreenHeight * 0.5f, 0.0f);
        glEnd();
    } else if (mRightSide) {
        glBegin(GL_TRIANGLES);
        glVertex3f(mScreenWidth - alignment.rMargin + 10, mScreenHeight * 0.5f + 50, 0.0f);
        glVertex3f(mScreenWidth - alignment.rMargin + 10, mScreenHeight * 0.5f - 50, 0.0f);
        glVertex3f(mScreenWidth, mScreenHeight * 0.5f, 0.0f);
        glEnd();
    }

    float headerFontAdv = mHeaderFont->Advance(m_model->currentDirectoryName().toUtf8());
    float headerFontAsc = mHeaderFont->Ascender();
    glEnable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
    glActiveTexture(GL_TEXTURE0);
    mHeaderFont->Render(m_model->currentDirectoryName().toUtf8(), -1, FTPoint(mScreenWidth * 0.5f - headerFontAdv * 0.5f, mScreenHeight - headerFontAsc));
    int pages = m_model->numberOfPages();
    if (pages > 1) {
        QString str = QString::number(m_model->currentPage() + 1) + "/" + QString::number(pages);
        float footerFontAdv = mFooterFont->Advance(str.toUtf8());
        float footerFontDec = mFooterFont->Descender();
        mFooterFont->Render(str.toUtf8(), -1, FTPoint(mScreenWidth * 0.5f - footerFontAdv * 0.5f, -footerFontDec));
    }
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);

    swapBuffers();

    //    doneCurrent();
}

//void VideoRenderer::open(int index)
//{
//    mStreamState = mDemuxer->getStreamState();
//    openVideoRenderer(mStreamState->mVideoWidth, mStreamState->mVideoHeight, mStreamState->mFormatCtx->streams[index]->codec->pix_fmt, 1);
//}

//void VideoRenderer::close()
//{
//   closeVideoRenderer();
//}

void VideoRenderer::render()
{
}

void VideoRenderer::moveContextToThread(void* thread)
{
    if (thread)
        context()->moveToThread(static_cast<QThread*>(thread));
    else
        context()->moveToThread(mOriginalThread);
}

void VideoRenderer::renderAlbumEntity(int nr, QPointF topLeft, QPointF bottomRight)
{
    float x1 = topLeft.x();
    float y1 = topLeft.y();
    float x2 = bottomRight.x();
    float y2 = bottomRight.y();
    //float borderWidth = 2.0f;
    float color[3] = { 0.5f, 0.5f, 0.5f };
    static int prevSelectedEntity = m_model->selectedEntity();
    static float currentTextXPos = 0.0f;
    static QTime timer;
    float textX = 0;

    AlbumEntity* entity = m_model->currentAlbums()[nr];
    float entityFontAdv = mEntityFont->Advance(entity->displayName().toUtf8());
    float entityFontAsc = mEntityFont->Ascender();
    float entityFontDec = mEntityFont->Descender();

    if (m_model->selectedEntity() == nr) {
        if (prevSelectedEntity != nr) {
            currentTextXPos = 0.0f;
            prevSelectedEntity = nr;
        }
        if ((x2 - x1) < entityFontAdv) {
            if (timer.elapsed() > 25) {
                currentTextXPos += mScrollSpeed;
                timer.restart();
            }
            if (currentTextXPos > entityFontAdv)
                currentTextXPos = -(x2 - x1);
        } else
            currentTextXPos = 0.0f;
        textX = currentTextXPos;
        color[0] = 1.0f;
        color[1] = 1.0f;
        color[2] = 1.0f;
    } else
        textX = 0.0f;

    glColor3f(1.0f, 1.0f, 1.0f);
    glEnable(GL_TEXTURE_2D);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, entity->getImage());
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(x1, y2, 0.0f);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(x2, y2, 0.0f);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(x2, y1, 0.0f);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(x1, y1, 0.0f);
    glEnd();
    glDisable(GL_TEXTURE_2D);

    glColor3fv(color);
    glBegin(GL_LINES);
    glVertex3f(x1 - 1, y1 + 1, 0.0f);
    glVertex3f(x1 - 1, y2 - 1, 0.0f);
    glVertex3f(x1 - 1, y2 - 1, 0.0f);
    glVertex3f(x2 + 1, y2 - 1, 0.0f);
    glVertex3f(x2 + 1, y2 - 1, 0.0f);
    glVertex3f(x2 + 1, y1 + 1, 0.0f);
    glVertex3f(x2 + 1, y1 + 1, 0.0f);
    glVertex3f(x1 - 1, y1 + 1, 0.0f);
    glEnd();

    glEnable(GL_SCISSOR_TEST);
    float xfrac = width() / (float)mScreenWidth;
    float yfrac = height() / (float)mScreenHeight;
    glScissor(x1 * xfrac, (y2 - (entityFontAsc - entityFontDec)) * yfrac, (x2 - x1) * xfrac, (entityFontAsc - entityFontDec) * yfrac);

    glEnable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
    mEntityFont->Render(entity->displayName().toUtf8(), -1, FTPoint(x1 - textX, y2 - entityFontAsc));
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
    glDisable(GL_SCISSOR_TEST);
}

void VideoRenderer::resizeEvent(QResizeEvent* /*evt*/)
{
}

void VideoRenderer::closeEvent(QCloseEvent* /*evt*/)
{
}

void VideoRenderer::resizeOnPlay(bool resize)
{
    mResizeOnPlay = resize;
}

void VideoRenderer::keepAspectRatio(bool keep)
{
    mKeepAspectRatio = keep;
}

void VideoRenderer::setFullScreen(bool fullscreen)
{
    mFullScreen = fullscreen;
}

void VideoRenderer::getOverlayFont(QString& fontName, QString& fontColor, int& fontSize, QPoint& fontPos, float& fadeTime)
{
    fontName = mOverlayFontName;
    fontColor = mOverlayFontColor;
    fontSize = mOverlayFontSize;
    fontPos = mOverlayFontPos;
    fadeTime = mOverlayFadeTime;
}

void VideoRenderer::setOverlayFont(const QString& fontName, const QString& fontColor, int fontSize, QPoint pos, float fadeTime)
{
    mOverlayFontName = fontName;
    mOverlayFontColor = fontColor;
    mOverlayFontSize = fontSize;
    mOverlayFontPos = pos;
    mOverlayFadeTime = fadeTime;
    textToRGB(mOverlayFontColor, mOverlayFontRGB.r, mOverlayFontRGB.g, mOverlayFontRGB.b);
}

void VideoRenderer::getSubtitleFont(QString& fontName, QString& fontColor, int& fontSize, QPoint& fontPos)
{
    fontName = mSubtitleFontName;
    fontColor = mSubtitleFontColor;
    fontSize = mSubtitleFontSize;
    fontPos = mSubtitleFontPos;
}

void VideoRenderer::setSubtitleFont(const QString& fontName, const QString& fontColor, int fontSize, QPoint pos)
{
    mSubtitleFontName = fontName;
    mSubtitleFontColor = fontColor;
    mSubtitleFontSize = fontSize;
    mSubtitleFontPos = pos;
    textToRGB(mSubtitleFontColor, mSubtitleFontRGB.r, mSubtitleFontRGB.g, mSubtitleFontRGB.b);
}

void VideoRenderer::updateOverlayText(const QString& overlayText)
{
    mOverlayText = overlayText;
    mOverlayFadeTimer.reset();
}

bool VideoRenderer::loadSettingsBinary(const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        std::cout << "Failed to load " << fileName.toStdString().c_str() << "\n";
        return false;
    }
    QDataStream in(&file);

    QString fontName;
    QString fontColor;
    int fontSize;
    QPoint fontPos;
    float fadeTime;
    in.setVersion(QDataStream::Qt_4_6);
    in >> fontName;
    in >> fontColor;
    in >> fontSize;
    in >> fontPos;
    in >> fadeTime;
    setOverlayFont(fontName, fontColor, fontSize, fontPos, fadeTime);
    in >> fontName;
    in >> fontColor;
    in >> fontSize;
    in >> fontPos;
    setSubtitleFont(fontName, fontColor, fontSize, fontPos);
    file.close();
    return true;
}

bool VideoRenderer::saveSettingsBinary(const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly))
        return false;
    QDataStream out(&file);

    out.setVersion(QDataStream::Qt_4_6);
    out << mOverlayFontName;
    out << mOverlayFontColor;
    out << mOverlayFontSize;
    out << mOverlayFontPos;
    out << mOverlayFadeTime;

    out << mSubtitleFontName;
    out << mSubtitleFontColor;
    out << mSubtitleFontSize;
    out << mSubtitleFontPos;
    file.close();
    return true;
}

void VideoRenderer::textToRGB(const QString& color, float& r, float& g, float& b)
{
    if (color.compare("White") == 0) {
        r = 1.0f;
        g = 1.0f;
        b = 1.0f;
    } else if (color.compare("Black") == 0) {
        r = 0.0f;
        g = 0.0f;
        b = 0.0f;
    } else if (color.compare("Red") == 0) {
        r = 1.0f;
        g = 0.0f;
        b = 0.0f;
    } else if (color.compare("Green") == 0) {
        r = 0.0f;
        g = 1.0f;
        b = 0.0f;
    } else if (color.compare("Yellow") == 0) {
        r = 1.0f;
        g = 1.0f;
        b = 0.0f;
    } else if (color.compare("Blue") == 0) {
        r = 0.0f;
        g = 0.0f;
        b = 1.0f;
    } else if (color.compare("Brown") == 0) {
        r = 0.54f;
        g = 0.27f;
        b = 0.075f;
    } else if (color.compare("Orange") == 0) {
        r = 1.0f;
        g = 0.27f;
        b = 0.0f;
    } else if (color.compare("Pink") == 0) {
        r = 1.0f;
        g = 0.08f;
        b = 0.58f;
    } else if (color.compare("Purple") == 0) {
        r = 0.58f;
        g = 0.0f;
        b = 0.83f;
    } else if (color.compare("Gray") == 0) {
        r = 0.41f;
        g = 0.41f;
        b = 0.41f;
    }
}

void VideoRenderer::renderSubtitle(const QStringList& subtitleText, double duration)
{
    mSubtitleText = subtitleText;
    mSubtitleFadeTimer.reset();
    mSubtitleFadeTime = duration;
}

void VideoRenderer::renderToFile(RenderImage renderImage)
{
    makeCurrent();

    glBindTexture(GL_TEXTURE_2D, mVideoTex[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, renderImage.w, renderImage.h, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    mConverter->changeContext(renderImage.format, renderImage.w, renderImage.h);

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mFBOVideo);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, mVideoTex[0], 0);

    mConverter->renderToRGB(renderImage.data, renderImage.linesize, false);

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mVideoTex[0]);
    uint8_t* pixels = new uint8_t[3 * renderImage.w * renderImage.h];
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);
    glPixelStorei(GL_PACK_ALIGNMENT, 4);
    QImage tmp(pixels, renderImage.w, renderImage.h, QImage::Format_RGB888);
    tmp.save(renderImage.filename, "JPG", -1);
    delete[] pixels;

    doneCurrent();
}

QList<QString> VideoRenderer::retreiveFilters()
{
    return mProgramName;
}

void VideoRenderer::changeVideoFilterPreset(const QStringList& filterChain)
{
    mNextFilterPrograms.clear();
    for (int i = 0; i < filterChain.size(); i++) {
        for (int j = 0; j < mProgramName.size(); j++) {
            if (filterChain[i].compare(mProgramName[j]) == 0) {
                mNextFilterPrograms << mPrograms[j];
                break;
            }
        }
    }
    mChangeFilterPrograms = true;
}

void VideoRenderer::changeStretchPreset(QVector4D preset)
{
    mLeftStretch = preset.w();
    mRightStretch = preset.x();
    mTopStretch = preset.y();
    mBottomStretch = preset.z();
}

void VideoRenderer::setColorFilterEnabled(bool enabled)
{
    mColorFilterEnabled = enabled;
}

void VideoRenderer::setBrightness(float b)
{
    mBrightness = b * 1 / 256.0f;
    mUpdateColorFilter = true;
}

void VideoRenderer::setContrast(float c)
{
    mContrast = c * 1 / 256.0f;
    mUpdateColorFilter = true;
}

void VideoRenderer::loadAlbumEntityImages(const QList<AlbumEntity*>& albums)
{
    makeCurrent();
    for (int i = 0; i < albums.size(); i++) {
        QFileInfo info(albums[i]->fileImage());
        if (info.isFile())
            albums[i]->setImage(loadImage(albums[i]->fileImage()));
        else
            albums[i]->setImage(m_defaultTexture);
    }
    doneCurrent();
}

void VideoRenderer::unloadAlbumEntityImages(const QList<AlbumEntity*>& albums)
{
    makeCurrent();
    for (int i = 0; i < albums.size(); i++) {
        GLuint image = albums[i]->getImage();
        if (image != m_defaultTexture)
            glDeleteTextures(1, &image);
    }
    doneCurrent();
}

void VideoRenderer::setModel(AlbumModel* model)
{
    m_model = model;
}

GLuint VideoRenderer::getDefaultTexture()
{
    return m_defaultTexture;
}

void VideoRenderer::highlightLeftSide(bool highlight)
{
    mLeftSide = highlight;
}

void VideoRenderer::highlightRightSide(bool highlight)
{
    mRightSide = highlight;
}

float VideoRenderer::scrollSpeed()
{
    return mScrollSpeed;
}

void VideoRenderer::setScrollSpeed(float scrollspeed)
{
    mScrollSpeed = scrollspeed;
}

unsigned int VideoRenderer::loadImage(const QString& file)
{
    GLuint texture;
    glEnable(GL_TEXTURE_2D);
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
#ifndef NO_IMAGE_ALLOC
    QImage image(file);
    if (image.isNull()) {
        std::cout << "Failed to load image\n";
        glDeleteTextures(1, &texture);
        return this->m_defaultTexture;
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image.width(), image.height(), 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image.width(), image.height(), GL_BGRA, GL_UNSIGNED_BYTE, (GLvoid*)image.bits());
#endif
    glDisable(GL_TEXTURE_2D);

    //printf("Texture ID %d created\n", texture);

    return texture;
}

unsigned int VideoRenderer::loadImage(const QImage& image)
{
    GLuint texture;
    glEnable(GL_TEXTURE_2D);
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image.width(), image.height(), 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image.width(), image.height(), GL_BGRA_EXT, GL_UNSIGNED_BYTE, (GLvoid*)image.bits());
    glDisable(GL_TEXTURE_2D);

    return texture;
}

void VideoRenderer::setDemuxer(Demuxer* demuxer)
{
    mDemuxer = demuxer;
}

void VideoRenderer::loadAlbumEntityImage(AlbumEntity* album)
{
    makeCurrent();
    QFileInfo info(album->fileImage());
    if (info.isFile())
        album->setImage(loadImage(album->fileImage()));
    else
        album->setImage(m_defaultTexture);
    doneCurrent();
}

void VideoRenderer::unloadAlbumEntityImage(AlbumEntity* album)
{
    makeCurrent();
    GLuint image = album->getImage();
    if (image != m_defaultTexture)
        glDeleteTextures(1, &image);
    doneCurrent();
}
