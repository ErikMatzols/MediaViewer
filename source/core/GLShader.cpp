#include "GLShader.hpp"
#include <fstream>
#include <iostream>

GLShader::GLShader(GLenum type)
{
    mObject = 0;
    mType = type;
}

GLShader::~GLShader()
{
    glDeleteShader(mObject);
}

GLuint GLShader::load(const std::string& fileName)
{
    char* buffer;

    mObject = glCreateShader(mType);

    std::ifstream file;
    file.open(fileName.c_str(), std::ios::binary);

    if (!file.is_open())
        return 0;

    std::cout << "loading " << fileName << "\n";

    file.seekg(0, std::ios::end);
    int length = static_cast<int>(file.tellg());
    file.seekg(0, std::ios::beg);

    buffer = new char[length + 1];
    file.read(buffer, length);
    file.close();
    buffer[length] = '\0';

    glShaderSource(mObject, 1, static_cast<char**>(&buffer), NULL);

    delete[] buffer;
    return mObject;
}

GLint GLShader::compile()
{
    GLint status;
    glCompileShader(mObject);
    glGetShaderiv(mObject, GL_COMPILE_STATUS, &status);
    return status;
}

void GLShader::printLog()
{
    int length;
    glGetShaderiv(mObject, GL_INFO_LOG_LENGTH, &length);
    if (length > 1) {
        char* buffer = new char[length];
        glGetShaderInfoLog(mObject, length, NULL, buffer);
        buffer[length - 1] = '\0';
        if (buffer[length - 2] != '\n')
            std::cout << buffer << "\n";
        else
            std::cout << buffer;
        delete[] buffer;
    }
}

GLuint GLShader::shaderID() const
{
    return mObject;
}
