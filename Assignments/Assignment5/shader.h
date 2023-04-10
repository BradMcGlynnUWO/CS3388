#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <GL/glew.h>

class Shader
{
public:
    // Constructor reads and builds the shader
    Shader(const char* vertexPath, const char* fragmentPath);

    // Use the shader program
    void use();

    // Utility uniform functions
    void setBool(const std::string &name, bool value) const;
    void setInt(const std::string &name, int value) const;
    void setFloat(const std::string &name, float value) const;

private:
    // The shader program ID
    unsigned int programID;

    // Utility function for reading the shader code from a file
    std::string readFile(const char* filePath);

    // Utility function for compiling shader code
    unsigned int compileShader(unsigned int type, const char* source);

    // Utility function for linking shaders into a program
    unsigned int linkProgram(unsigned int vertexShader, unsigned int fragmentShader);
};

#endif
