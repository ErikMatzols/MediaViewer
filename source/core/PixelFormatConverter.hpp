#ifndef PIXELFORMATCONVERTER_HPP
#define PIXELFORMATCONVERTER_HPP

#include "StreamState.hpp"
#include <GL/glew.h>
#include <libavutil/pixfmt.h>
class GLProgram;
class GLShader;

class PixelFormatConverter {
public:
    PixelFormatConverter();
    ~PixelFormatConverter();

    void changeContext(AVPixelFormat format, int width, int height);
    void renderToRGB(uint8_t* data[4], int linesize[4], bool flip);

protected:
private:
    enum AVPixelFormat mCurrentFormat;
    int mCurrentWidth, mCurrentHeight;

    GLProgram* mPrograms[AV_PIX_FMT_NB];
    GLShader* mFragmentShaders[AV_PIX_FMT_NB];
    GLShader* mVertexShader;

    GLuint mYUVTex[3];
};

#endif
