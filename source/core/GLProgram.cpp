#include "GLProgram.hpp"
#include "GLShader.hpp"
#include <iostream>

GLProgram::GLProgram()
{
    mProgram = glCreateProgram();
}

GLProgram::~GLProgram()
{
    glDeleteProgram(mProgram);
}

void GLProgram::attachShader(const GLShader& shader)
{
    glAttachShader(mProgram, shader.shaderID());
}

void GLProgram::detachShader(const GLShader& shader)
{
    glDetachShader(mProgram, shader.shaderID());
}

GLint GLProgram::link()
{
    GLint status;
    glLinkProgram(mProgram);
    glGetProgramiv(mProgram, GL_LINK_STATUS, &status);
    return status;
}

void GLProgram::enable()
{
    glUseProgram(mProgram);
}

void GLProgram::disable()
{
    glUseProgram(0);
}

void GLProgram::bindAttribLocation(GLuint loc, const GLchar* name)
{
    glBindAttribLocation(mProgram, loc, name);
}

void GLProgram::uniformMatrix3fv(const char* name, int size, const GLfloat* value)
{
    glUniformMatrix3fv(glGetUniformLocation(mProgram, name), size, GL_FALSE, value);
}

void GLProgram::uniformMatrix4fv(const char* name, int size, const GLfloat* value)
{
    glUniformMatrix4fv(glGetUniformLocation(mProgram, name), size, GL_FALSE, value);
}

void GLProgram::uniform1iv(const char* name, int size, const GLint* value)
{
    glUniform1iv(glGetUniformLocation(mProgram, name), size, value);
}

void GLProgram::uniform2iv(const char* name, int size, const GLint* value)
{
    glUniform2iv(glGetUniformLocation(mProgram, name), size, value);
}

void GLProgram::uniform3iv(const char* name, int size, const GLint* value)
{
    glUniform3iv(glGetUniformLocation(mProgram, name), size, value);
}

void GLProgram::uniform4iv(const char* name, int size, const GLint* value)
{
    glUniform4iv(glGetUniformLocation(mProgram, name), size, value);
}

void GLProgram::uniform1fv(const char* name, int size, const GLfloat* value)
{
    glUniform1fv(glGetUniformLocation(mProgram, name), size, value);
}

void GLProgram::uniform2fv(const char* name, int size, const GLfloat* value)
{
    glUniform2fv(glGetUniformLocation(mProgram, name), size, value);
}

void GLProgram::uniform3fv(const char* name, int size, const GLfloat* value)
{
    glUniform3fv(glGetUniformLocation(mProgram, name), size, value);
}

void GLProgram::uniform4fv(const char* name, int size, const GLfloat* value)
{
    glUniform4fv(glGetUniformLocation(mProgram, name), size, value);
}

void GLProgram::uniform1f(const char* name, GLfloat value)
{
    glUniform1f(glGetUniformLocation(mProgram, name), value);
}

void GLProgram::uniform2f(const char* name, GLfloat value1, GLfloat value2)
{
    glUniform2f(glGetUniformLocation(mProgram, name), value1, value2);
}

void GLProgram::uniform3f(const char* name, GLfloat value1, GLfloat value2, GLfloat value3)
{
    glUniform3f(glGetUniformLocation(mProgram, name), value1, value2, value3);
}

void GLProgram::uniform4f(const char* name, GLfloat value1, GLfloat value2, GLfloat value3, GLfloat value4)
{
    glUniform4f(glGetUniformLocation(mProgram, name), value1, value2, value3, value4);
}

void GLProgram::uniform1i(const char* name, GLint value)
{
    glUniform1i(glGetUniformLocation(mProgram, name), value);
}

void GLProgram::uniform2i(const char* name, GLint value1, GLint value2)
{
    glUniform2i(glGetUniformLocation(mProgram, name), value1, value2);
}

void GLProgram::uniform3i(const char* name, GLint value1, GLint value2, GLint value3)
{
    glUniform3i(glGetUniformLocation(mProgram, name), value1, value2, value3);
}

void GLProgram::uniform4i(const char* name, GLint value1, GLint value2, GLint value3, GLint value4)
{
    glUniform4i(glGetUniformLocation(mProgram, name), value1, value2, value3, value4);
}

void GLProgram::printLog()
{
    int length;
    glGetProgramiv(mProgram, GL_INFO_LOG_LENGTH, &length);
    if (length > 1) {
        char* buffer = new char[length];
        glGetProgramInfoLog(mProgram, length, NULL, buffer);
        buffer[length - 1] = '\0';
        if (buffer[length - 2] != '\n')
            std::cout << buffer << "\n";
        else
            std::cout << buffer;
        delete[] buffer;
    }
}
