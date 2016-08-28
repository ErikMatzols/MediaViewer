#ifndef GLPROGRAM_HPP
#define GLPROGRAM_HPP

#include <GL/glew.h>
class GLShader;

class GLProgram {
public:
    GLProgram();
    ~GLProgram();

    void attachShader(const GLShader& shader);
    void detachShader(const GLShader& shader);
    GLint link();

    void enable();
    void disable();

    void bindAttribLocation(GLuint loc, const GLchar* name);

    void uniformMatrix4fv(const char* name, int size, const GLfloat* value);
    void uniformMatrix3fv(const char* name, int size, const GLfloat* value);

    void uniform1fv(const char* name, int size, const GLfloat* value);
    void uniform2fv(const char* name, int size, const GLfloat* value);
    void uniform3fv(const char* name, int size, const GLfloat* value);
    void uniform4fv(const char* name, int size, const GLfloat* value);

    void uniform1iv(const char* name, int size, const GLint* value);
    void uniform2iv(const char* name, int size, const GLint* value);
    void uniform3iv(const char* name, int size, const GLint* value);
    void uniform4iv(const char* name, int size, const GLint* value);

    void uniform1f(const char* name, GLfloat value);
    void uniform2f(const char* name, GLfloat value1, GLfloat value2);
    void uniform3f(const char* name, GLfloat value1, GLfloat value2, GLfloat value3);
    void uniform4f(const char* name, GLfloat value1, GLfloat value2, GLfloat value3, GLfloat value4);

    void uniform1i(const char* name, GLint value);
    void uniform2i(const char* name, GLint value1, GLint value2);
    void uniform3i(const char* name, GLint value1, GLint value2, GLint value3);
    void uniform4i(const char* name, GLint value1, GLint value2, GLint value3, GLint value4);

    void printLog();

protected:
private:
    GLuint mProgram;
};

#endif
