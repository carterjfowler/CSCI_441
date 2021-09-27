#ifndef __LAB_04_HPP__
#define __LAB_04_HPP__

#include <GL/glew.h>

#include <cstdio>

#include <fstream>
#include <string>
using namespace std;

// readTextFromFile() //////////////////////////////////////////////////////////////
//
//  Reads in a text file as a single string. Used to aid in shader loading.
//
////////////////////////////////////////////////////////////////////////////////
void readTextFromFile( const char* filename, char* &output ){
    string buf = string("");
    string line;

    ifstream in( filename );
    while( getline(in, line) ) {
        buf += line + "\n";
    }
    output = new char[buf.length()+1];
    strncpy(output, buf.c_str(), buf.length());
    output[buf.length()] = '\0';

    in.close();
}

// printLog() //////////////////////////////////////////////////////////////////
//
//  Check for errors from compiling or linking a vertex/fragment/shader program
//	Prints to standard out
//
////////////////////////////////////////////////////////////////////////////////
void printLog( GLuint handle ) {
    int infoLogLength = 0;
    int maxLength;

    // check if the handle is to a shader
    bool isShader;
    if( glIsShader( handle ) ) {
        glGetShaderiv(  handle, GL_INFO_LOG_LENGTH, &maxLength );
        isShader = true;
    }
    // check if the handle is to a shader program
    else {
        glGetProgramiv( handle, GL_INFO_LOG_LENGTH, &maxLength );
        isShader = false;
    }

    // create a buffer of designated length
    char infoLog[maxLength];


    if( isShader ) {
        // get the info log for the shader
        glGetShaderInfoLog( handle, maxLength, &infoLogLength, infoLog );
    } else {
        // get the info log for the shader program
        glGetProgramInfoLog( handle, maxLength, &infoLogLength, infoLog );
    }

    // if the length of the log is greater than 0
    if( infoLogLength > 0 ) {
        // print info to terminal
        printf( "[INFO]: %s Handle %d: %s\n", (isShader ? "Shader" : "Program"), handle, infoLog );
    }
}

// compileShader() ///////////////////////////////////////////////////////////////
//
//  Compile a given shader program
//
////////////////////////////////////////////////////////////////////////////////
GLuint compileShader( const char* filename, GLenum shaderType ) {
    char *shaderString;

    // LOOK HERE #1 read in each text file and store the contents in a string
    readTextFromFile( filename, shaderString );

    // TODO #01 create a handle for our shader of the corresponding type
    GLuint shaderHandle = glCreateShader(shaderType);
    if (shaderHandle <= 0) {
        fprintf( stderr, "[ERROR]: glCreateShader returned 0, issue with memory\n" );
    }

    // TODO #02 send the contents of each program to the GPU
    glShaderSource(shaderHandle, 1, (const char**)&shaderString, NULL);

    // we are good programmers so free up the memory used by each buffer
    delete [] shaderString;

    // TODO #03 compile each shader on the GPU
    glCompileShader(shaderHandle);

    // print the log for this shader handle to verify it compiled correctly
    printLog( shaderHandle );

    return shaderHandle;
}

// createShaderProgram() ///////////////////////////////////////////////////////
//
//  Compile and Register our Vertex and Fragment Shaders
//
////////////////////////////////////////////////////////////////////////////////
GLuint createShaderProgram( const char *vertexShaderFilename, const char *fragmentShaderFilename ) {

    // compile each one of our shaders
    GLuint vertexShaderHandle   = compileShader( vertexShaderFilename,   GL_VERTEX_SHADER   );
    GLuint fragmentShaderHandle = compileShader( fragmentShaderFilename, GL_FRAGMENT_SHADER );

    // TODO #04 get a handle to a shader program
    GLuint shaderProgramHandle = glCreateProgram();
    if (shaderProgramHandle <= 0) {
        fprintf( stderr, "[ERROR]: shaderProgramHandle returned 0, issue with memory\n" );
    }

    // TODO #05A attach the vertex shader to the shader program
    glAttachShader(shaderProgramHandle, vertexShaderHandle);

    // TODO #05A attach the fragment shader to the shader program
    glAttachShader(shaderProgramHandle, fragmentShaderHandle);

    // TODO #06 link all the programs together on the GPU
    glLinkProgram(shaderProgramHandle);

    // print the log for this shader program to verify it linked correctly
    printLog( shaderProgramHandle );

    // TODO #07 detach each shader from the shader program
    glDetachShader(shaderProgramHandle, vertexShaderHandle);
    glDetachShader(shaderProgramHandle, fragmentShaderHandle);

    // TODO #08 delete the shaders from the GPU
    glDeleteShader(vertexShaderHandle);
    glDeleteShader(fragmentShaderHandle);

    // return handle
    return shaderProgramHandle;
}

#endif // __LAB_04_HPP__
