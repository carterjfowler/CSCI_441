#ifndef __CSCI441_LAB03_BLACK_MAGIC_H___
#define __CSCI441_LAB03_BLACK_MAGIC_H___

#include <GL/glew.h>

#include <glm/glm.hpp>

namespace Lab03BlackMagic {
    void setupShaders();
    void setMVPMatrices( glm::mat4 projMtx, glm::mat4 viewMtx );
    void setMD5Shader();
    void setSkeletonShader();
    void deleteShaders();

    struct ShaderAttributes {
        GLint vertexPosition;
        GLint vertexColor;
        GLint vertexTextureCoord;
    };
    static const ShaderAttributes SHADER_ATTRIBUTES = {0, 1, 2 };
}

#endif // __CSCI441_LAB03_BLACK_MAGIC_H___
