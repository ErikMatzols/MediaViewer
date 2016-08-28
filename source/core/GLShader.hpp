#ifndef _GLSHADER_
#define _GLSHADER_

#include <GL/glew.h>
#include <string>

class GLShader {
public:
    GLShader(GLenum type);
    ~GLShader();

    GLuint load(const std::string& fileName);
    GLint compile();
    void printLog();

    GLuint shaderID() const;

protected:
private:
    GLenum mType;
    GLuint mObject;
};

#endif
