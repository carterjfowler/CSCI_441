#ifndef __441_WORLD_MAP_H__
#define __441_WORLD_MAP_H__

// here's a fun fact....glew internally includes gl.h
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>		// used for loading an image...we will get to this later on.  You can ignore it for now.

// include GLM libraries and matrix functions
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <CSCI441/ShaderUtils.hpp>

#include <cstdio>
#include <cstdlib>
#include <ctime>

#include <sstream>			// I don't fully love C...let's use some C++
#include <string>
using namespace std;

//////////////////////////////////////////////////////////
//
//	Do not modify any of these values.  They handle
//	the world map operation.  We will eventually
//	discuss how textures work so that we can display
//	images like is being done here.

const GLint MAP_WIDTH = 7, MAP_HEIGHT = 7;				// sets the width and height of our 2D map array
GLint mapHandles[ MAP_WIDTH ][ MAP_HEIGHT ];			// a 2D storing the texture handles of our world map backgrounds
GLint currMapLocationX = MAP_WIDTH / 2,
      currMapLocationY = MAP_HEIGHT / 2;	        	// our current location within the map
GLuint mapShaderProgramHandle;
GLuint mapVAO;

void loadMap();
void randomizeMap();
void loadMapShader(GLint windowWidth, GLint windowHeight);
void loadMapBuffers(GLint windowWidth, GLint windowHeight);
void drawMap();	                                        // draws a rectangle with our world map background

// these next four methods update the current location and loads the correct map.  they will wrap
// around the edges, so the Hero can move through an infinite world
void moveDown();										// moves our world map down one square
void moveLeft();										// moves our world map left one square
void moveRight();										// moves our world map right one square
void moveUp();											// moves our world map up one square
//////////////////////////////////////////////////////////

// draw a quad with our texture mapped to it
void drawMap() {
    glEnable( GL_TEXTURE_2D );
    glUseProgram(mapShaderProgramHandle);
    glBindVertexArray(mapVAO);
    glBindTexture( GL_TEXTURE_2D, mapHandles[ currMapLocationX ][ currMapLocationY ] );
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glDisable( GL_TEXTURE_2D );
}

// I hope you can figure out what's going on here
void moveDown() {
	currMapLocationY++;
	if( currMapLocationY >= MAP_HEIGHT )
		currMapLocationY = 0;
}

void moveLeft() {
	currMapLocationX--;
	if( currMapLocationX < 0 )
		currMapLocationX = MAP_WIDTH-1;
}

void moveRight() {
	currMapLocationX++;
	if( currMapLocationX >= MAP_WIDTH )
		currMapLocationX = 0;
}

void moveUp() {
	currMapLocationY--;
	if( currMapLocationY < 0 )
		currMapLocationY = MAP_HEIGHT-1;
}

// initialize our map
void initMap(GLint windowWidth, GLint windowHeight) {
    // I'm going to leave this uncommented for now.  Trust that it works.  It controls
    // the loading of all the background images that get displayed
    loadMapShader(windowWidth, windowHeight);
    loadMapBuffers(windowWidth, windowHeight);
    loadMap();
    randomizeMap();
}

void loadMap() {
    glEnable( GL_TEXTURE_2D );
    printf( "[INFO]: Loading map images..." );
    fflush( stdout );

    int imageWidth, imageHeight, imageChannels;
    GLuint mapTexHandle;
    stbi_set_flip_vertically_on_load(true);

    for( GLint j = 0; j < MAP_HEIGHT; j++ ) {
		for( GLint i = 0; i < MAP_WIDTH; i++ ) {
			stringstream filenameSS;
			filenameSS << "./images/map" << (j*MAP_WIDTH + i) << ".png";
			string filename = filenameSS.str();

            glGenTextures(1, &mapTexHandle);
            glBindTexture(GL_TEXTURE_2D, mapTexHandle);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            mapHandles[ i ][ j ] = mapTexHandle;

            unsigned char *data = stbi_load( filename.c_str(), &imageWidth, &imageHeight, &imageChannels, 0);

			// failed to load image
			if( !data ) {
				filename = "./images/default.png";
                data = stbi_load( filename.c_str(), &imageWidth, &imageHeight, &imageChannels, 0);
                if( !data ) {
                    fprintf( stderr, "Could not load default image ./images/default.png.  Be sure to update Run > Edit Configurations and set your Working Directory to be ..\n" );
                    fflush( stderr );
                }
			}

			const GLint STORAGE_TYPE = (imageChannels == 4 ? GL_RGBA : GL_RGB);
            glTexImage2D( GL_TEXTURE_2D, 0, STORAGE_TYPE, imageWidth, imageHeight, 0, STORAGE_TYPE, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);

            stbi_image_free(data);
		}
		printf( "." );
		fflush( stdout );
	}
    printf( "...done!\n" );
    fflush( stdout );
    glDisable( GL_TEXTURE_2D );
}

void randomizeMap() {
    srand( time(nullptr) );
    printf( "[INFO]: Randomizing map locations..." );
    const GLint RAND_ITERATIONS = MAP_HEIGHT * MAP_WIDTH * 1000;
    for( GLint k = 0; k < RAND_ITERATIONS; k++ ) {
        GLint x1, y1, x2, y2;
        x1 = rand() % MAP_WIDTH;
        y1 = rand() % MAP_HEIGHT;
        x2 = rand() % MAP_WIDTH;
        y2 = rand() % MAP_HEIGHT;

        GLint tempTexHandle = mapHandles[ x1 ][ y1 ];
        mapHandles[ x1 ][ y1 ] = mapHandles[ x2 ][ y2 ];
        mapHandles[ x2 ][ y2 ] = tempTexHandle;

        if( k % 2500 == 0 ) {
            printf( "." );
            fflush( stdout );
        }
    }
    printf( "...done!\n" );
    fflush( stdout );
}

void loadMapShader(GLint windowWidth, GLint windowHeight) {
    std::string vertex_shader_src = "#version 410 core\n \
                                    \n \
                                    uniform mat4 model;\n \
                                    uniform mat4 view;\n \
                                    uniform mat4 projection;\n \
                                    \n \
                                    layout(location=0) in vec2 vPos;\n \
                                    layout(location=1) in vec2 vTexCoord;\n \
                                    \n \
                                    layout(location=0) \
                                    out vec2 texCoord;\n \
                                    \n \
                                    void main() {\n \
                                        gl_Position = projection * view * model * vec4(vPos, 0.0, 1.0);\n \
                                        texCoord = vTexCoord;\n \
                                    }";
    const char* vertexShaders[1] = { vertex_shader_src.c_str() };

    std::string fragment_shader_src = "#version 410 core\n \
                                      \n \
                                      uniform sampler2D mapTex;\n \
                                      \n \
                                      layout(location=0) in vec2 texCoord;\n \
                                      \n \
                                      layout(location=0) out vec4 fragColorOut;\n \
                                      \n \
                                      void main() {\n \
                                          fragColorOut = texture(mapTex, texCoord);\n \
                                      }";
    const char* fragmentShaders[1] = { fragment_shader_src.c_str() };

    GLuint vertexShaderHandle = glCreateShader( GL_VERTEX_SHADER );
    glShaderSource(vertexShaderHandle, 1, vertexShaders, nullptr);
    glCompileShader(vertexShaderHandle);
    CSCI441_INTERNAL::ShaderUtils::printLog(vertexShaderHandle);

    GLuint fragmentShaderHandle = glCreateShader( GL_FRAGMENT_SHADER );
    glShaderSource(fragmentShaderHandle, 1, fragmentShaders, nullptr);
    glCompileShader(fragmentShaderHandle);
    CSCI441_INTERNAL::ShaderUtils::printLog(fragmentShaderHandle);

    mapShaderProgramHandle = glCreateProgram();
    glAttachShader(mapShaderProgramHandle, vertexShaderHandle);
    glAttachShader(mapShaderProgramHandle, fragmentShaderHandle);
    glLinkProgram(mapShaderProgramHandle);
    CSCI441_INTERNAL::ShaderUtils::printLog(mapShaderProgramHandle);

    glDetachShader(mapShaderProgramHandle, vertexShaderHandle);
    glDeleteShader(vertexShaderHandle);

    glDetachShader(mapShaderProgramHandle, fragmentShaderHandle);
    glDeleteShader(fragmentShaderHandle);

    CSCI441_INTERNAL::ShaderUtils::printShaderProgramInfo(mapShaderProgramHandle);

    GLuint modelLocation       = glGetUniformLocation(mapShaderProgramHandle, "model");
    GLuint viewLocation        = glGetUniformLocation(mapShaderProgramHandle, "view");
    GLuint projectionLocation  = glGetUniformLocation(mapShaderProgramHandle, "projection");

    glUseProgram(mapShaderProgramHandle);

    glm::mat4 identity(1.0);
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &identity[0][0]);
    glUniformMatrix4fv(viewLocation, 1, GL_FALSE, &identity[0][0]);

    glm::mat4 projMtx = glm::ortho( 0.0f, (GLfloat)windowWidth, 0.0f, (GLfloat)windowHeight );
    glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, &projMtx[0][0]);

    printf( "[INFO]: Map shader loaded!\n" );
    fflush( stdout );
}

void loadMapBuffers(GLint windowWidth, GLint windowHeight) {
    glm::vec2 vertices[4] = { glm::vec2(0.0, 0.0),
                              glm::vec2(0.0, (GLfloat)windowHeight),
                              glm::vec2((GLfloat)windowWidth, 0.0),
                              glm::vec2((GLfloat)windowWidth, (GLfloat)windowHeight) };
    glm::vec2 texCoords[4] = { glm::vec2(0.0, 0.0),
                               glm::vec2(0.0, 1.0),
                               glm::vec2(1.0, 0.0),
                               glm::vec2(1.0, 1.0) };

    glGenVertexArrays(1, &mapVAO);
    glBindVertexArray(mapVAO);

    GLuint mapVBO;
    glGenBuffers(1, &mapVBO);
    glBindBuffer(GL_ARRAY_BUFFER, mapVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*4*2 + sizeof(GLfloat)*4*2, nullptr, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat)*4*2, &vertices[0]);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(GLfloat)*4*2, sizeof(GLfloat)*4*2, &texCoords[0]);

    GLuint vertexLocation      = glGetAttribLocation(mapShaderProgramHandle, "vPos");
    GLuint textureLocation     = glGetAttribLocation(mapShaderProgramHandle, "vTexCoord");

    glEnableVertexAttribArray(vertexLocation);
    glVertexAttribPointer(vertexLocation, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glEnableVertexAttribArray(textureLocation);
    glVertexAttribPointer(textureLocation, 2, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(GLfloat)*4*2));

    printf( "[INFO]: Map buffers registered!\n" );
    fflush( stdout );
}

#endif // __441_WORLD_MAP_H__
