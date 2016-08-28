#include "PixelFormatConverter.hpp"
#include "GLProgram.hpp"
#include "GLShader.hpp"
#include <QDir>
#include <iostream>

PixelFormatConverter::PixelFormatConverter()
{
    for (int i = 0; i < 3; i++) {
        glGenTextures(1, &mYUVTex[i]);
        glBindTexture(GL_TEXTURE_2D, mYUVTex[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }

    std::cout << "Loading and compiling shader programs...\n";

    mVertexShader = new GLShader(GL_VERTEX_SHADER);
    mVertexShader->load("../shaders/Main.vert");

    if (!mVertexShader->compile())
        std::cout << "Vertex shader failed to compile!\n";

    mVertexShader->printLog();

    QDir graphDirectory("../shaders/PixelFormat");
    QStringList files = graphDirectory.entryList(QDir::Files | QDir::NoDotAndDotDot);

    foreach (QString file, files) {
        QString name = file.split(".").first();
        AVPixelFormat format = av_get_pix_fmt(name.toStdString().c_str());

        mFragmentShaders[format] = new GLShader(GL_FRAGMENT_SHADER);
        mFragmentShaders[format]->load(("../shaders/PixelFormat/" + file).toStdString());

        if (!mFragmentShaders[format]->compile())
            std::cout << "Fragment shader " << file.toStdString().c_str() << " failed to compile\n";

        mFragmentShaders[format]->printLog();
        mPrograms[format] = new GLProgram;
        mPrograms[format]->attachShader(*mVertexShader);
        mPrograms[format]->attachShader(*mFragmentShaders[format]);

        if (!mPrograms[format]->link())
            std::cout << "Program failed to link!\n";

        mPrograms[format]->printLog();
    }
}

PixelFormatConverter::~PixelFormatConverter()
{
    glDeleteTextures(3, &mYUVTex[0]);
    QDir graphDirectory("../shaders/PixelFormat");

    QStringList files = graphDirectory.entryList(QDir::Files | QDir::NoDotAndDotDot);
    foreach (QString file, files) {
        QString name = file.split(".").first();
        AVPixelFormat format = av_get_pix_fmt(name.toStdString().c_str());

        mPrograms[format]->detachShader(*mVertexShader);
        mPrograms[format]->detachShader(*mFragmentShaders[format]);

        delete mPrograms[format];
        delete mFragmentShaders[format];
    }

    delete mVertexShader;
}

void PixelFormatConverter::changeContext(enum AVPixelFormat format, int width, int height)
{
    if (format == mCurrentFormat && mCurrentWidth == width && mCurrentHeight == height)
        return;

    mCurrentFormat = format;
    mCurrentWidth = width;
    mCurrentHeight = height;

    switch (format) {
    case AV_PIX_FMT_YUVJ420P:
        mPrograms[AV_PIX_FMT_YUVJ420P]->enable();
        mPrograms[AV_PIX_FMT_YUVJ420P]->uniform1f("YTex", 0);
        mPrograms[AV_PIX_FMT_YUVJ420P]->uniform1f("UTex", 1);
        mPrograms[AV_PIX_FMT_YUVJ420P]->uniform1f("VTex", 2);
        mPrograms[AV_PIX_FMT_YUVJ420P]->disable();
        break;
    case AV_PIX_FMT_YUV420P:
        mPrograms[AV_PIX_FMT_YUV420P]->enable();
        mPrograms[AV_PIX_FMT_YUV420P]->uniform1i("YTex", 0);
        mPrograms[AV_PIX_FMT_YUV420P]->uniform1i("UTex", 1);
        mPrograms[AV_PIX_FMT_YUV420P]->uniform1i("VTex", 2);
        mPrograms[AV_PIX_FMT_YUV420P]->disable();
        break;
    case AV_PIX_FMT_RGB24:
        mPrograms[AV_PIX_FMT_RGB24]->enable();
        mPrograms[AV_PIX_FMT_RGB24]->uniform1i("YTex", 0);
        mPrograms[AV_PIX_FMT_RGB24]->disable();
        break;
    case AV_PIX_FMT_BGR24:
        mPrograms[AV_PIX_FMT_BGR24]->enable();
        mPrograms[AV_PIX_FMT_BGR24]->uniform1i("YTex", 0);
        mPrograms[AV_PIX_FMT_BGR24]->disable();
        break;
    case AV_PIX_FMT_RGB555LE:
        mPrograms[AV_PIX_FMT_RGB555LE]->enable();
        mPrograms[AV_PIX_FMT_RGB555LE]->uniform1i("YTex", 0);
        mPrograms[AV_PIX_FMT_RGB555LE]->disable();
        break;
    case AV_PIX_FMT_YUV410P:
        mPrograms[AV_PIX_FMT_YUV410P]->enable();
        mPrograms[AV_PIX_FMT_YUV410P]->uniform1i("YTex", 0);
        mPrograms[AV_PIX_FMT_YUV410P]->uniform1i("UTex", 1);
        mPrograms[AV_PIX_FMT_YUV410P]->uniform1i("VTex", 2);
        mPrograms[AV_PIX_FMT_YUV410P]->disable();
        break;
    case AV_PIX_FMT_YUYV422:
        mPrograms[AV_PIX_FMT_YUYV422]->enable();
        mPrograms[AV_PIX_FMT_YUYV422]->uniform1i("YTex", 0);
        mPrograms[AV_PIX_FMT_YUYV422]->uniform1f("Frac", 1 / (float)(mCurrentWidth - 1));
        mPrograms[AV_PIX_FMT_YUYV422]->disable();
        break;
    case AV_PIX_FMT_NV12:
        mPrograms[AV_PIX_FMT_NV12]->enable();
        mPrograms[AV_PIX_FMT_NV12]->uniform1i("YTex", 0);
        mPrograms[AV_PIX_FMT_NV12]->uniform1i("UTex", 1);
        mPrograms[AV_PIX_FMT_NV12]->uniform1f("Frac", 1 / (float)(mCurrentWidth - 1));
        mPrograms[AV_PIX_FMT_NV12]->disable();
        break;
    default:
        std::cout << "Error: PixelFormat " << av_get_pix_fmt_name(mCurrentFormat) << " conversion not supported\n";
        break;
    }
}

void PixelFormatConverter::renderToRGB(uint8_t* data[4], int linesize[4], bool flip)
{
    int texWidth = abs(linesize[0]);

    switch (mCurrentFormat) {
    case AV_PIX_FMT_YUVJ420P:
    case AV_PIX_FMT_YUV420P: {
        int videoH[3] = { mCurrentHeight, mCurrentHeight / 2, mCurrentHeight / 2 };
        for (int i = 0; i < 3; i++) {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, mYUVTex[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, linesize[i], videoH[i], 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, (GLvoid*)data[i]);
        }
        break;
    }
    case AV_PIX_FMT_RGB24: {
        texWidth /= 3;
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mYUVTex[0]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, texWidth, mCurrentHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, data[0]);
        break;
    }
    case AV_PIX_FMT_BGR24: {
        texWidth /= 3;
        flip = !flip;
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mYUVTex[0]);
        int start = (mCurrentHeight - 1) * linesize[0];
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, texWidth, mCurrentHeight, 0, GL_BGR, GL_UNSIGNED_BYTE, &data[0][start]);
        break;
    }
    case AV_PIX_FMT_RGB555LE: {
        texWidth /= 2;
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mYUVTex[0]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB5_A1, texWidth, mCurrentHeight, 0, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV, data[0]);
        break;
    }
    case AV_PIX_FMT_YUV410P: {
        int videoH[3] = { mCurrentHeight, mCurrentHeight / 4, mCurrentHeight / 4 };
        for (int i = 2; i >= 0; i--) {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, mYUVTex[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, linesize[i], videoH[i], 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, (GLvoid*)data[i]);
        }
        break;
    }
    case AV_PIX_FMT_YUYV422: {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mYUVTex[0]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE8_ALPHA8, mCurrentWidth, mCurrentHeight, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, data[0]);
        break;
    }
    case AV_PIX_FMT_NV12: { /*
            int videoH[2] = {mCurrentHeight, mCurrentHeight/2};
            for (int i = 1; i >= 0; i--) {
                glActiveTexture(GL_TEXTURE0 + i);
                glBindTexture(GL_TEXTURE_2D, mYUVTex[i]);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, linesize[i], videoH[i], 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, data[i]); 
            }
          */
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mYUVTex[0]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, mCurrentWidth, mCurrentHeight, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, (GLvoid*)data[0]);
    } break;
    default:
        std::cout << "Warning: PixelFormat " << av_get_pix_fmt_name(mCurrentFormat) << " conversion not supported\n";
        return;
    }

    glViewport(0, 0, mCurrentWidth, mCurrentHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, mCurrentWidth, 0, mCurrentHeight, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    mPrograms[mCurrentFormat]->enable();

    if (flip) {
        glBegin(GL_QUADS);
        glMultiTexCoord2f(GL_TEXTURE0, 0.0f, 1.0f);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glMultiTexCoord2f(GL_TEXTURE0, (mCurrentWidth / (float)texWidth), 1.0f);
        glVertex3f(mCurrentWidth, 0.0f, 0.0f);
        glMultiTexCoord2f(GL_TEXTURE0, (mCurrentWidth / (float)texWidth), 0.0f);
        glVertex3f(mCurrentWidth, mCurrentHeight, 0.0f);
        glMultiTexCoord2f(GL_TEXTURE0, 0.0f, 0.0f);
        glVertex3f(0.0f, mCurrentHeight, 0.0f);
        glEnd();
    } else {
        glBegin(GL_QUADS);
        glMultiTexCoord2f(GL_TEXTURE0, 0.0f, 0.0f);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glMultiTexCoord2f(GL_TEXTURE0, (mCurrentWidth / (float)texWidth), 0.0f);
        glVertex3f(mCurrentWidth, 0.0f, 0.0f);
        glMultiTexCoord2f(GL_TEXTURE0, (mCurrentWidth / (float)texWidth), 1.0f);
        glVertex3f(mCurrentWidth, mCurrentHeight, 0.0f);
        glMultiTexCoord2f(GL_TEXTURE0, 0.0f, 1.0f);
        glVertex3f(0.0f, mCurrentHeight, 0.0f);
        glEnd();
    }

    mPrograms[mCurrentFormat]->disable();
}
