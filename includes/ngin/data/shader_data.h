#ifndef SHADER_DATA_H
#define SHADER_DATA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <ngin/data/i_data.h>

class ShaderData : public IData {
public:
    unsigned int ID;

    ShaderData() {} 
    ShaderData(const Nevf& data) {

    }
    ShaderData(const char* vertexPath, const char* fragmentPath, const char* headerPath,
        const char* geometryPath = nullptr, const char* includePath = nullptr) : vertPath_(vertexPath),
        fragPath_(fragmentPath), headPath_(headerPath), geomPath_(geometryPath), inclPath_(includePath)
    {
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
            std::cerr << "Vertex Path: " << vertPath_ << std::endl;
            std::cerr << "Fragment Path: " << fragPath_ << std::endl;
            if (geomPath_)
                std::cerr << "Geometry Path: " << geomPath_ << std::endl;
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
        if(geomPath_ != nullptr)
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
        if(geomPath_ != nullptr)
            glAttachShader(ID, geometry);
        glLinkProgram(ID);
        checkCompileErrors(ID, "PROGRAM");
        // delete the shaders as they're linked into our program now and no longer necessary
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        if(geomPath_ != nullptr)
            glDeleteShader(geometry);
    }

private:
    const char* headPath_;
    const char* vertPath_;
    const char* fragPath_;
    const char* geomPath_;
    const char* inclPath_;

    std::string name_;

    unsigned int loadFrag() {
        std::string fragmentCode;
        std::ifstream fShaderFile;
        std::ifstream iShaderFile;
        std::ifstream hShaderFile;

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
        std::ifstream hShaderFile;

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
        std::string geometryCode;
        std::ifstream hShaderFile;
        std::ifstream gShaderFile;

        gShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
        try 
        {
            hShaderFile.open(headPath_);
            std::stringstream hShaderStream;
            hShaderStream << hShaderFile.rdbuf();
            hShaderFile.close();

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
            if (geomPath_)
                std::cerr << "Geometry Path: " << geomPath_ << std::endl;
        }
        // if geometry shader is given, compile geometry shader
        unsigned int geometry;
        if(geomPath_ != nullptr)
        {
            const char * gShaderCode = geometryCode.c_str();
            geometry = glCreateShader(GL_GEOMETRY_SHADER);
            glShaderSource(geometry, 1, &gShaderCode, NULL);
            glCompileShader(geometry);
            checkCompileErrors(geometry, "GEOMETRY");
        }

        return geometry;
    }
    void checkCompileErrors(GLuint shader, std::string type)
    {
        GLint success;
        GLchar infoLog[1024];
        if(type != "PROGRAM")
        {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if(!success)
            {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
        else
        {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if(!success)
            {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
    }
};

#endif