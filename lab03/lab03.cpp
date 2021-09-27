#include "lab03.h"

#include <glm/gtc/matrix_transform.hpp>

#include <string>

#include <CSCI441/ShaderUtils.hpp>

struct ShaderUniforms {
    GLint model;
    GLint view;
    GLint projection;
    GLint texture;
};

GLuint passThroughShaderHandle = 0, md5ShaderHandle = 0, skeletonShaderHandle = 0;
ShaderUniforms passThroughUniforms, md5Uniforms, skeletonUniforms;

GLuint registerShader( const std::string& VERTEX_SHADER_SRC, const std::string& FRAGMENT_SHADER_SRC ) {
    const char* VERTEX_SHADER[1] = { VERTEX_SHADER_SRC.c_str() };
    const char* FRAGMENT_SHADER[1] = { FRAGMENT_SHADER_SRC.c_str() };

    GLuint vertexShaderHandle = glCreateShader( GL_VERTEX_SHADER );
    glShaderSource( vertexShaderHandle, 1, VERTEX_SHADER, nullptr );
    glCompileShader( vertexShaderHandle );
    CSCI441_INTERNAL::ShaderUtils::printLog( vertexShaderHandle );

    GLuint fragmentShaderHandle = glCreateShader( GL_FRAGMENT_SHADER );
    glShaderSource( fragmentShaderHandle, 1, FRAGMENT_SHADER, nullptr );
    glCompileShader( fragmentShaderHandle );
    CSCI441_INTERNAL::ShaderUtils::printLog( fragmentShaderHandle );

    GLuint shaderHandle = glCreateProgram();
    glAttachShader( shaderHandle, vertexShaderHandle );
    glAttachShader( shaderHandle, fragmentShaderHandle );
    glLinkProgram( shaderHandle );
    CSCI441_INTERNAL::ShaderUtils::printLog( shaderHandle );

    glDetachShader( shaderHandle, vertexShaderHandle );
    glDeleteShader( vertexShaderHandle );

    glDetachShader( shaderHandle, fragmentShaderHandle );
    glDeleteShader( fragmentShaderHandle );

    CSCI441_INTERNAL::ShaderUtils::printShaderProgramInfo( shaderHandle );

    return shaderHandle;
}

void setupPlatformShader_internal() {
    const std::string PLATFORM_VERTEX_SHADER_SRC =
            "#version 410 core\n \
              \n \
              uniform mat4 model;\n \
              uniform mat4 view;\n \
              uniform mat4 projection;\n \
              \n \
              layout(location=0) in vec3 vPos;\n \
              layout(location=1) in vec3 vColor;\n \
              \n \
              layout(location=0) out vec3 color;\n \
              \n \
              void main() {\n \
                gl_Position = projection * view * model * vec4(vPos, 1.0);\n \
                color = vColor;\n \
              }\n";

    const std::string PLATFORM_FRAGMENT_SHADER_SRC =
            "#version 410 core\n \
              \n \
              layout(location=0) in vec3 color;\n \
              \n \
              layout(location=0) out vec4 fragColorOut;\n \
              \n \
              void main() {\n \
                fragColorOut = vec4(color, 1.0);\n \
              }\n";

    passThroughShaderHandle = registerShader( PLATFORM_VERTEX_SHADER_SRC, PLATFORM_FRAGMENT_SHADER_SRC );

    passThroughUniforms.model      = glGetUniformLocation( passThroughShaderHandle, "model" );
    passThroughUniforms.view       = glGetUniformLocation( passThroughShaderHandle, "view" );
    passThroughUniforms.projection = glGetUniformLocation( passThroughShaderHandle, "projection" );
}

void setupMD5Shader_internal() {
    const std::string MD5_VERTEX_SHADER_SRC =
            "#version 410 core\n \
              \n \
              uniform mat4 model;\n \
              uniform mat4 view;\n \
              uniform mat4 projection;\n \
              \n \
              layout(location=0) in vec3 vPos;\n \
              layout(location=2) in vec2 vTexCoord;\n \
              \n \
              layout(location=0) out vec2 texCoord;\n \
              \n \
              void main() {\n \
                gl_Position = projection * view * model * vec4(vPos, 1.0);\n \
                texCoord = vTexCoord;\n \
              }\n";

    const std::string MD5_FRAGMENT_SHADER_SRC =
            "#version 410 core\n \
              \n \
              uniform sampler2D md5Texture;\
              \n \
              layout(location=0) in vec2 texCoord;\n \
              \n \
              layout(location=0) out vec4 fragColorOut;\n \
              \n \
              void main() {\n \
                fragColorOut = texture(md5Texture, texCoord);\n \
              }\n";

    md5ShaderHandle = registerShader( MD5_VERTEX_SHADER_SRC, MD5_FRAGMENT_SHADER_SRC );

    md5Uniforms.model      = glGetUniformLocation( md5ShaderHandle, "model" );
    md5Uniforms.view       = glGetUniformLocation( md5ShaderHandle, "view" );
    md5Uniforms.projection = glGetUniformLocation( md5ShaderHandle, "projection" );
    md5Uniforms.texture    = glGetUniformLocation( md5ShaderHandle, "md5Texture" );

    glUseProgram( md5ShaderHandle );
    glUniform1i( md5Uniforms.texture, 0 );
}

void setupSkeletonShader_internal() {
    const std::string SKELETON_VERTEX_SHADER_SRC =
            "#version 410 core\n \
              \n \
              uniform mat4 model;\n \
              uniform mat4 view;\n \
              uniform mat4 projection;\n \
              \n \
              layout(location=0)in vec3 vPos;\n \
              layout(location=1)in vec3 vColor;\n \
              \n \
              layout(location=0)out vec3 color;\n \
              \n \
              void main() {\n \
                gl_Position = projection * view * model * vec4(vPos, 1.0);\n \
                color = vColor;\n \
              }\n";
    const std::string SKELETON_FRAGMENT_SHADER_SRC =
            "#version 410 core\n \
              \n \
              layout(location=0)in vec3 color;\n \
              \n \
              layout(location=0)out vec4 fragColorOut;\n \
              \n \
              void main() {\n \
                fragColorOut = vec4(color, 1.0);\n \
              }\n";

    skeletonShaderHandle = registerShader( SKELETON_VERTEX_SHADER_SRC, SKELETON_FRAGMENT_SHADER_SRC );

    skeletonUniforms.model      = glGetUniformLocation( skeletonShaderHandle, "model" );
    skeletonUniforms.view       = glGetUniformLocation( skeletonShaderHandle, "view" );
    skeletonUniforms.projection = glGetUniformLocation( skeletonShaderHandle, "projection" );
}

void setupShaders_internal() {
    setupPlatformShader_internal();
    setupMD5Shader_internal();
    setupSkeletonShader_internal();
}

void Lab03BlackMagic::setupShaders() {
    glActiveTexture(GL_TEXTURE0);
    setupShaders_internal();
    glUseProgram( passThroughShaderHandle );
}

void Lab03BlackMagic::setMVPMatrices( glm::mat4 projMtx, glm::mat4 viewMtx ) {
    glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);

    glUseProgram( passThroughShaderHandle );
    glm::mat4 m(1.0f);
    glUniformMatrix4fv(passThroughUniforms.model, 1, GL_FALSE, &m[0][0]);
    glUniformMatrix4fv(passThroughUniforms.view, 1, GL_FALSE, &viewMtx[0][0]);
    glUniformMatrix4fv(passThroughUniforms.projection, 1, GL_FALSE, &projMtx[0][0]);

    glUseProgram( md5ShaderHandle );
    m = glm::rotate( glm::mat4(1.0f), -90.0f * 3.14159f / 180.0f, glm::vec3( 1.0f, 0.0f, 0.0f ) );
    m = glm::scale( m, glm::vec3( 0.09f, 0.09f, 0.09f ) );
    glUniformMatrix4fv(md5Uniforms.model, 1, GL_FALSE, &m[0][0]);
    glUniformMatrix4fv(md5Uniforms.view, 1, GL_FALSE, &viewMtx[0][0]);
    glUniformMatrix4fv(md5Uniforms.projection, 1, GL_FALSE, &projMtx[0][0]);

    glUseProgram( skeletonShaderHandle );
    glUniformMatrix4fv(skeletonUniforms.model, 1, GL_FALSE, &m[0][0]);
    glUniformMatrix4fv(skeletonUniforms.view, 1, GL_FALSE, &viewMtx[0][0]);
    glUniformMatrix4fv(skeletonUniforms.projection, 1, GL_FALSE, &projMtx[0][0]);

    glUseProgram( passThroughShaderHandle );
}

void Lab03BlackMagic::deleteShaders() {
    glDeleteProgram( passThroughShaderHandle );
    glDeleteProgram( md5ShaderHandle );
    glDeleteProgram( skeletonShaderHandle );
}

void Lab03BlackMagic::setMD5Shader() {
    glUseProgram( md5ShaderHandle );
}

void Lab03BlackMagic::setSkeletonShader() {
    glUseProgram( skeletonShaderHandle );
}
