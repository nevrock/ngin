#ifndef SHADER_DATA_H
#define SHADER_DATA_H

#include <ngin/data/i_data.h>

class ShaderData : public IData {
public:

  ShaderData() {} 
  ShaderData(const Nevf& data) {

  }
  void execute() override {}

  std::string getName() override { return name_; } 

  void load() {
    std::string vertexCode;
    std::string fragmentCode;
    std::string geometryCode;
    std::ifstream hShaderFile;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;
    std::ifstream gShaderFile;
    std::ifstream iShaderFile;

    vShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
    gShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
    try 
    {
        vShaderFile.open(vertPath_);
        fShaderFile.open(fragPath_);
        std::stringstream vShaderStream, fShaderStream;

        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();		

        vShaderFile.close();
        fShaderFile.close();

        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();		

        if (inclPath_ != nullptr)
        {
            iShaderFile.open(inclPath_);
            std::stringstream iShaderStream;
            iShaderStream << iShaderFile.rdbuf();
            iShaderFile.close();
            fragmentCode = iShaderStream.str() + + "\n" + fragmentCode;
        }

        hShaderFile.open(headPath_);
        std::stringstream hShaderStream;
        hShaderStream << hShaderFile.rdbuf();
        hShaderFile.close();
        fragmentCode = hShaderStream.str() + + "\n" + fragmentCode;
        vertexCode = hShaderStream.str() + + "\n" + vertexCode;

        if (geomPath_ != nullptr)
        {
            gShaderFile.open(geomPath_);
            std::stringstream gShaderStream;
            gShaderStream << gShaderFile.rdbuf();
            gShaderFile.close();
            geometryCode = gShaderStream.str();
            geometryCode = hShaderStream.str() + + "\n" + geometryCode;
        }
    }
    catch (std::ifstream::failure& e)
    {
        std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
        std::cerr << "Vertex Path: " << vertexPath << std::endl;
        std::cerr << "Fragment Path: " << fragmentPath << std::endl;
        if (geometryPath)
            std::cerr << "Geometry Path: " << geometryPath << std::endl;
    }
    const char* vShaderCode = vertexCode.c_str();
    const char * fShaderCode = fragmentCode.c_str();
    // 2. compile shaders
    unsigned int vertex, fragment;
    // vertex shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    checkCompileErrors(vertex, "VERTEX");
    // fragment Shader
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    checkCompileErrors(fragment, "FRAGMENT");
    // if geometry shader is given, compile geometry shader
    unsigned int geometry;
    if(geometryPath != nullptr)
    {
        const char * gShaderCode = geometryCode.c_str();
        geometry = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(geometry, 1, &gShaderCode, NULL);
        glCompileShader(geometry);
        checkCompileErrors(geometry, "GEOMETRY");
    }
    // shader Program
    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    if(geometryPath != nullptr)
        glAttachShader(ID, geometry);
    glLinkProgram(ID);
    checkCompileErrors(ID, "PROGRAM");
    // delete the shaders as they're linked into our program now and no longer necessary
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    if(geometryPath != nullptr)
        glDeleteShader(geometry);
}


private:
    char* headPath_;
    char* vertPath_;
    char* fragPath_;
    char* geomPath_;
    char* inclPath_;

    unsigned int loadFrag() {
        std::string fragmentCode;
        std::ifstream fShaderFile;
        fShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
        try 
        {
            fShaderFile.open(fragPath_);
            std::stringstream fShaderStream;
            fShaderStream << fShaderFile.rdbuf();		
            fShaderFile.close();
            fragmentCode = fShaderStream.str();		

            if (inclPath_ != nullptr)
            {
                iShaderFile.open(inclPath_);
                std::stringstream iShaderStream;
                iShaderStream << iShaderFile.rdbuf();
                iShaderFile.close();
                fragmentCode = iShaderStream.str() + + "\n" + fragmentCode;
            }

            hShaderFile.open(headPath_);
            std::stringstream hShaderStream;
            hShaderStream << hShaderFile.rdbuf();
            hShaderFile.close();
            fragmentCode = hShaderStream.str() + + "\n" + fragmentCode;
        }
        catch (std::ifstream::failure& e)
        {
            std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
            std::cerr << "Vertex Path: " << fragPath_ << std::endl;
        }
        const char * fShaderCode = fragmentCode.c_str();
        unsigned int fragment;
        // fragment Shader
        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fShaderCode, NULL);
        glCompileShader(fragment);
        checkCompileErrors(fragment, "FRAGMENT");

        return fragment;
    }
    unsigned int loadVert() {
        std::string vertexCode;
        std::ifstream vShaderFile;

        vShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
        try 
        {
            vShaderFile.open(vertPath_);
            std::stringstream vShaderStream;

            vShaderStream << vShaderFile.rdbuf();

            vShaderFile.close();

            vertexCode = vShaderStream.str();

            hShaderFile.open(headPath_);
            std::stringstream hShaderStream;
            hShaderStream << hShaderFile.rdbuf();
            hShaderFile.close();
            vertexCode = hShaderStream.str() + + "\n" + vertexCode;
        }
        catch (std::ifstream::failure& e)
        {
            std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
            std::cerr << "Vertex Path: " << vertPath_ << std::endl;
        }
        const char* vShaderCode = vertexCode.c_str();
        // 2. compile shaders
        unsigned int vertex;
        // vertex shader
        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vShaderCode, NULL);
        glCompileShader(vertex);
        checkCompileErrors(vertex, "VERTEX");

        return vertex;
    }
    unsigned int loadGeom() {
        std::string vertexCode;
        std::string fragmentCode;
        std::string geometryCode;
        std::ifstream hShaderFile;
        std::ifstream vShaderFile;
        std::ifstream fShaderFile;
        std::ifstream gShaderFile;
        std::ifstream iShaderFile;

        vShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
        fShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
        gShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
        try 
        {
            vShaderFile.open(vertPath_);
            fShaderFile.open(fragPath_);
            std::stringstream vShaderStream, fShaderStream;

            vShaderStream << vShaderFile.rdbuf();
            fShaderStream << fShaderFile.rdbuf();		

            vShaderFile.close();
            fShaderFile.close();

            vertexCode = vShaderStream.str();
            fragmentCode = fShaderStream.str();		

            if (inclPath_ != nullptr)
            {
                iShaderFile.open(inclPath_);
                std::stringstream iShaderStream;
                iShaderStream << iShaderFile.rdbuf();
                iShaderFile.close();
                fragmentCode = iShaderStream.str() + + "\n" + fragmentCode;
            }

            hShaderFile.open(headPath_);
            std::stringstream hShaderStream;
            hShaderStream << hShaderFile.rdbuf();
            hShaderFile.close();
            fragmentCode = hShaderStream.str() + + "\n" + fragmentCode;
            vertexCode = hShaderStream.str() + + "\n" + vertexCode;

            if (geomPath_ != nullptr)
            {
                gShaderFile.open(geomPath_);
                std::stringstream gShaderStream;
                gShaderStream << gShaderFile.rdbuf();
                gShaderFile.close();
                geometryCode = gShaderStream.str();
                geometryCode = hShaderStream.str() + + "\n" + geometryCode;
            }
        }
        catch (std::ifstream::failure& e)
        {
            std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
            std::cerr << "Vertex Path: " << vertexPath << std::endl;
            std::cerr << "Fragment Path: " << fragmentPath << std::endl;
            if (geometryPath)
                std::cerr << "Geometry Path: " << geometryPath << std::endl;
        }
        const char* vShaderCode = vertexCode.c_str();
        const char * fShaderCode = fragmentCode.c_str();
        // 2. compile shaders
        unsigned int vertex, fragment;
        // vertex shader
        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vShaderCode, NULL);
        glCompileShader(vertex);
        checkCompileErrors(vertex, "VERTEX");
        // fragment Shader
        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fShaderCode, NULL);
        glCompileShader(fragment);
        checkCompileErrors(fragment, "FRAGMENT");
        // if geometry shader is given, compile geometry shader
        unsigned int geometry;
        if(geometryPath != nullptr)
        {
            const char * gShaderCode = geometryCode.c_str();
            geometry = glCreateShader(GL_GEOMETRY_SHADER);
            glShaderSource(geometry, 1, &gShaderCode, NULL);
            glCompileShader(geometry);
            checkCompileErrors(geometry, "GEOMETRY");
        }
        // shader Program
        ID = glCreateProgram();
        glAttachShader(ID, vertex);
        glAttachShader(ID, fragment);
        if(geometryPath != nullptr)
            glAttachShader(ID, geometry);
        glLinkProgram(ID);
        checkCompileErrors(ID, "PROGRAM");
        // delete the shaders as they're linked into our program now and no longer necessary
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        if(geometryPath != nullptr)
            glDeleteShader(geometry);
    }
};

#endif