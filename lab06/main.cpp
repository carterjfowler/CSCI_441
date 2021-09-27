/*
 *  CSCI 441, Computer Graphics, Fall 2020
 *
 *  Project: lab06
 *  File: main.cpp
 *
 *  Description:
 *      This file contains the basic setup to work with textures.
 *
 *  Author: Dr. Paone, Colorado School of Mines, 2020
 *
 */

///***********************************************************************************************************************************************************
//
// Library includes

#include <GL/glew.h>                    // define our OpenGL extensions
#include <GLFW/glfw3.h>			        // include GLFW framework header

#include <glm/glm.hpp>                  // include GLM libraries
#include <glm/gtc/matrix_transform.hpp> // and matrix functions

#include <cstdio>				        // for printf functionality
#include <cstdlib>				        // for exit functionality

#include <CSCI441/modelLoader.hpp>      // to load in OBJ models
#include <CSCI441/OpenGLUtils.hpp>      // prints OpenGL information
#include <CSCI441/objects.hpp>          // draws 3D objects
#include <CSCI441/ShaderProgram.hpp>    // wrapper class for GLSL shader programs

///***********************************************************************************************************************************************************
//
// Global Parameters

// fix our window to a specific size
const GLint WINDOW_WIDTH = 640, WINDOW_HEIGHT = 640;

// keep track our mouse information
GLboolean controlDown;                  // if the control button was pressed when the mouse was pressed
GLboolean leftMouseDown;                // if the mouse left button is pressed
glm::vec2 mousePosition;                // current mouse position

// keep track of all our camera information
struct CameraParameters {
    glm::vec3 cameraAngles;             // cameraAngles --> x = theta, y = phi, z = radius
    glm::vec3 camDir;                   // direction to the camera
    glm::vec3 lookAtPoint;              // location of our object of interest to view
    glm::vec3 upVector;                 // the upVector of our camera
} arcballCam;

// keep track of our objects
GLuint platformVAO, platformVBOs[2];    // the ground platform everything is hovering over

GLuint negY_TexHandle = 0, posY_TexHandle = 0, negX_TexHandle = 0,
        posX_TexHandle = 0, negZ_TexHandle = 0, posZ_TexHandle = 0;
GLuint negY_VAO, posY_VAO, negX_VAO, posX_VAO, negZ_VAO, posZ_VAO;
GLuint negY_VBOs[2], posY_VBOs[2], negX_VBOs[2], posX_VBOs[2], negZ_VBOs[2], posZ_VBOs[2];

GLuint quadVAO, quadVBOs[2];            // our own custom quad to display
CSCI441::ModelLoader* model = nullptr;  // stores the object information for an OBJ model
GLuint objectIndex;                     // tracks which object we want to be viewing
GLfloat objectAngle;                    // the current angle of rotation to display our object at
const GLfloat ROTATION_SPEED = 0.01f;   // rate at which to rotate

// keep track of our textures
// TODO #08 create a texture handle for our metal image
GLuint metalTexHandle = 0, minesTexHandle = 0;

// keep track of our shader program
CSCI441::ShaderProgram *texShaderProgram = nullptr;
struct TexShaderProgramUniforms {
    GLint mvpMatrix;                    // the MVP Matrix to apply
    // TODO #11 add a uniform location for our texture map
    GLint texMap_uniform_loc;
} texShaderProgramUniforms;
struct TexShaderProgramAttributes {
    GLint vPos;                         // position of our vertex
    // TODO #10 add an attribute location for our texture coordinate
    GLint texCoord_attr_loc;
} texShaderProgramAttributes;

///***********************************************************************************************************************************************************
//
// Helper Functions

// updateCameraDirection() /////////////////////////////////////////////////////
//
// This function updates the camera's position in cartesian coordinates based
//  on its position in spherical coordinates. Should be called every time
//  cameraAngles is updated.
//
////////////////////////////////////////////////////////////////////////////////
void updateCameraDirection() {
    // ensure the camera does not flip upside down at either pole
    if( arcballCam.cameraAngles.y < 0 )     arcballCam.cameraAngles.y = 0.0f + 0.001f;
    if( arcballCam.cameraAngles.y >= M_PI ) arcballCam.cameraAngles.y = M_PI - 0.001f;

    // do not let our camera get too close or too far away
    if( arcballCam.cameraAngles.z <= 2.0f )  arcballCam.cameraAngles.z = 2.0f;
    if( arcballCam.cameraAngles.z >= 30.0f ) arcballCam.cameraAngles.z = 30.0f;

    // update the new direction to the camera
    arcballCam.camDir.x =  sinf( arcballCam.cameraAngles.x ) * sinf( arcballCam.cameraAngles.y );
    arcballCam.camDir.y = -cosf( arcballCam.cameraAngles.y );
    arcballCam.camDir.z = -cosf( arcballCam.cameraAngles.x ) * sinf( arcballCam.cameraAngles.y );

    // normalize this direction
    arcballCam.camDir = glm::normalize(arcballCam.camDir);
}

// loadAndRegisterTexture() ////////////////////////////////////////////////////
//
// This function loads an image into CPU memory and registers it with the GPU.
//      Will set the texture parameters and send the data to the GPU.
//
////////////////////////////////////////////////////////////////////////////////
GLuint loadAndRegisterTexture( const char* FILENAME) {
    GLuint textureHandle = 0;

    // load the image data from file to a byte array
    int imageWidth, imageHeight, imageChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load( FILENAME, &imageWidth, &imageHeight, &imageChannels, 0);
    const GLint STORAGE_TYPE = (imageChannels == 4 ? GL_RGBA : GL_RGB);

    if( data ) {
        // if image reading was successful, then we can register it
        // TODO #01 generate a new texture handle
        glGenTextures(1, &textureHandle);
        // TODO #02 bind the handle to our 2D texture target
        glBindTexture(GL_TEXTURE_2D, textureHandle);
        // TODO #03 set the mag filter
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // TODO #04 set the min filter
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        // TODO #05 set how to wrap S
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        // TODO #06 set how to wrap T
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // TODO #07 transfer data to the GPU
        glTexImage2D(GL_TEXTURE_2D, 0, STORAGE_TYPE, imageWidth, imageHeight, 0, STORAGE_TYPE, GL_UNSIGNED_BYTE, data);

        fprintf( stdout, "[INFO]: %s texture map read in with handle %d\n", FILENAME, textureHandle);

        stbi_image_free(data);                                  // release the image data from the CPU
    } else {
        // error loading image, alert the user
        fprintf( stderr, "[ERROR]: Could not load texture map \"%s\"\n", FILENAME );
    }

    return textureHandle;
}

///***********************************************************************************************************************************************************
//
// Event Callbacks

// error_callback() ////////////////////////////////////////////////////////////
//
//		We will register this function as GLFW's error callback.
//	When an error within GLFW occurs, GLFW will tell us by calling
//	this function.  We can then print this info to the terminal to
//	alert the user.
//
////////////////////////////////////////////////////////////////////////////////
static void error_callback(int error, const char* description) {
	fprintf(stderr, "[ERROR]: (%d) %s\n", error, description);
}

// key_callback() //////////////////////////////////////////////////////////////
//
//		We will register this function as GLFW's keypress callback.
//	Responds to key presses and key releases
//
////////////////////////////////////////////////////////////////////////////////
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if(action == GLFW_PRESS) {
        switch( key ) {
            case GLFW_KEY_Q:
            case GLFW_KEY_ESCAPE:
                glfwSetWindowShouldClose( window, GLFW_TRUE );
                break;

            // toggles which shape is being drawn
            case GLFW_KEY_1:
            case GLFW_KEY_2:
            case GLFW_KEY_3:
            case GLFW_KEY_4:
            case GLFW_KEY_5:
            case GLFW_KEY_6:
            case GLFW_KEY_7:
            case GLFW_KEY_8:
                objectIndex = key - GLFW_KEY_1; // GLFW_KEY_1 is 49.  they go in sequence from there
                break;

            default: break;
        }
    }
}

// mouse_button_callback() /////////////////////////////////////////////////////
//
//		We will register this function as GLFW's mouse button callback.
//	Responds to mouse button presses and mouse button releases.  Keeps track if
//	the control key was pressed when a left mouse click occurs to allow
//	zooming of our arcball camera.
//
////////////////////////////////////////////////////////////////////////////////
static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	if( button == GLFW_MOUSE_BUTTON_LEFT ) {
        if( action == GLFW_PRESS ) {
            leftMouseDown = GL_TRUE;
            controlDown = (mods & GLFW_MOD_CONTROL);
        } else {
            leftMouseDown = GL_FALSE;
            controlDown = GL_FALSE;
            mousePosition = glm::vec2(-9999.0f, -9999.0f);
        }
    }
}

// cursor_callback() ///////////////////////////////////////////////////////////
//
//		We will register this function as GLFW's cursor movement callback.
//	Responds to mouse movement.  When active motion is used with the left
//	mouse button an arcball camera model is followed.
//
////////////////////////////////////////////////////////////////////////////////
static void cursor_callback( GLFWwindow* window, double xPos, double yPos ) {
	// make sure movement is in bounds of the window
	// glfw captures mouse movement on entire screen
	if( xPos > 0 && xPos < WINDOW_WIDTH ) {
		if( yPos > 0 && yPos < WINDOW_HEIGHT ) {
			// active motion
			if( leftMouseDown ) {
				if( (mousePosition.x - -9999.0f) > 0.001f ) {
					if( !controlDown ) {
                        // if control is not held down, update our camera angles theta & phi
                        arcballCam.cameraAngles.x += (xPos - mousePosition.x) * 0.005f;
                        arcballCam.cameraAngles.y += (mousePosition.y - yPos) * 0.005f;
					} else {
					    // otherwise control was held down, update our camera radius
						double totChgSq = (xPos - mousePosition.x) + (yPos - mousePosition.y);
                        arcballCam.cameraAngles.z += totChgSq*0.01f;
					}
					// recompute our camera direction
                    updateCameraDirection();
				}
				// update the last mouse position
                mousePosition = glm::vec2(xPos, yPos);
			}
			// passive motion
			else {

			}
		}
	}
}

// scroll_callback() ///////////////////////////////////////////////////////////
//
//		We will register this function as GLFW's scroll wheel callback.
//	Responds to movement of the scroll where.  Allows zooming of the arcball
//	camera.
//
////////////////////////////////////////////////////////////////////////////////
static void scroll_callback(GLFWwindow* window, double xOffset, double yOffset ) {
	double totChgSq = yOffset;
    arcballCam.cameraAngles.z += totChgSq*0.2f;
    updateCameraDirection();
}

///***********************************************************************************************************************************************************
//
// Setup Functions

// setupGLFW() /////////////////////////////////////////////////////////////////
//
//		Used to setup everything GLFW related.  This includes the OpenGL context
//	and our window.
//
////////////////////////////////////////////////////////////////////////////////
GLFWwindow* setupGLFW() {
	// set what function to use when registering errors
	// this is the ONLY GLFW function that can be called BEFORE GLFW is initialized
	// all other GLFW calls must be performed after GLFW has been initialized
	glfwSetErrorCallback(error_callback);

	// initialize GLFW
	if (!glfwInit()) {
		fprintf( stderr, "[ERROR]: Could not initialize GLFW\n" );
		exit(EXIT_FAILURE);
	} else {
		fprintf( stdout, "[INFO]: GLFW initialized\n" );
	}

	glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );						// request forward compatible OpenGL context
	glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );	        // request OpenGL Core Profile context
	glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );		                // request OpenGL 4.X context
	glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 1 );		                // request OpenGL X.1 context
	glfwWindowHint( GLFW_DOUBLEBUFFER, GLFW_TRUE );                             // request double buffering

	// create a window for a given size, with a given title
	GLFWwindow *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Lab06: Texture That Teapot", nullptr, nullptr );
	if( !window ) {						                                        // if the window could not be created, NULL is returned
		fprintf( stderr, "[ERROR]: GLFW Window could not be created\n" );
		glfwTerminate();
		exit( EXIT_FAILURE );
	} else {
		fprintf( stdout, "[INFO]: GLFW Window created\n" );
	}

	glfwMakeContextCurrent(	window );	                                        // make the created window the current window
	glfwSwapInterval( 1 );				                                // update our screen after at least 1 screen refresh

	glfwSetKeyCallback(         window, key_callback		  );            	// set our keyboard callback function
	glfwSetMouseButtonCallback( window, mouse_button_callback );	            // set our mouse button callback function
	glfwSetCursorPosCallback(	window, cursor_callback  	  );	            // set our cursor position callback function
	glfwSetScrollCallback(		window, scroll_callback		  );	            // set our scroll wheel callback function

	return window;										                        // return the window that was created
}

// setupOpenGL() ///////////////////////////////////////////////////////////////
//
//      Used to setup everything OpenGL related.
//
////////////////////////////////////////////////////////////////////////////////
void setupOpenGL() {
	glEnable( GL_DEPTH_TEST );					                    // enable depth testing
	glDepthFunc( GL_LESS );							                // use less than depth test

	glEnable(GL_BLEND);									            // enable blending
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	            // use one minus blending equation

	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );	// clear the frame buffer to black
}

// setupGLEW() /////////////////////////////////////////////////////////////////
//
//      Used to initialize GLEW
//
////////////////////////////////////////////////////////////////////////////////
void setupGLEW() {
	glewExperimental = GL_TRUE;
	GLenum glewResult = glewInit();

	// check for an error
	if( glewResult != GLEW_OK ) {
		fprintf( stderr, "[ERROR]: Error initializing GLEW\n");
  	    fprintf( stderr, "[ERROR]: %s\n", glewGetErrorString(glewResult) );
		exit(EXIT_FAILURE);
	} else {
        fprintf( stdout, "\n[INFO]: GLEW initialized\n" );
        fprintf( stdout, "[INFO]: Using GLEW %s\n", glewGetString(GLEW_VERSION) );
	}
}

// setupShaders() //////////////////////////////////////////////////////////////
//
//      Registers our Shader Programs and query locations
//          of uniform/attribute inputs
//
////////////////////////////////////////////////////////////////////////////////
void setupShaders() {
    texShaderProgram = new CSCI441::ShaderProgram( "shaders/lab06.v.glsl", "shaders/lab06.f.glsl" );
    texShaderProgramUniforms.mvpMatrix   = texShaderProgram->getUniformLocation("mvpMatrix");
    texShaderProgramAttributes.vPos      = texShaderProgram->getAttributeLocation("vPos");
    // TODO #12 lookup the uniform and attribute location
    texShaderProgramUniforms.texMap_uniform_loc = texShaderProgram->getUniformLocation("textureMap");
    texShaderProgramAttributes.texCoord_attr_loc = texShaderProgram->getAttributeLocation("textCoordAttrib");
    texShaderProgram->useProgram();                         // set our shader program to be active
    // TODO #13 set the texture map uniform
    glUniform1i(texShaderProgramUniforms.texMap_uniform_loc, 0);
}

// setupBuffers() //////////////////////////////////////////////////////////////
//
//      Create our VAOs & VBOs. Send vertex data to the GPU for future rendering
//
////////////////////////////////////////////////////////////////////////////////
void setupBuffers() {
//	struct VertexTextured {
//		float x, y, z;
//        // TODO #14 add texture coordinates to our vertex struct
//        float s, t;
//	};
//
//	// create our platform
//    VertexTextured platformVertices[4] = {
//            // TODO #15 specify an s,t for each vertex on our ground platform
//			{ -10.0f, 0.0f, -10.0f , 0.0f, 0.0f}, // 0 - BL
//			{  10.0f, 0.0f, -10.0f , 1.0f, 0.0f}, // 1 - BR
//			{ -10.0f, 0.0f,  10.0f , 0.0f, 1.0f}, // 2 - TL
//			{  10.0f, 0.0f,  10.0f , 1.0f, 1.0f}  // 3 - TR
//	};
//
//
//	unsigned short platformIndices[4] = { 0, 1, 2, 3 };
//
//	glGenVertexArrays( 1, &platformVAO );
//	glBindVertexArray( platformVAO );
//
//	glGenBuffers( 2, platformVBOs );
//
//	glBindBuffer( GL_ARRAY_BUFFER, platformVBOs[0] );
//	glBufferData( GL_ARRAY_BUFFER, sizeof( platformVertices ), platformVertices, GL_STATIC_DRAW );
//
//	glEnableVertexAttribArray( texShaderProgramAttributes.vPos );
//	glVertexAttribPointer( texShaderProgramAttributes.vPos, 3, GL_FLOAT, GL_FALSE, sizeof(VertexTextured), (void*) 0 );
//
//    // TODO #16 set up the vertex attribute for texture coordinates
//    glEnableVertexAttribArray( texShaderProgramAttributes.texCoord_attr_loc );
//    glVertexAttribPointer( texShaderProgramAttributes.texCoord_attr_loc, 2, GL_FLOAT, GL_FALSE, sizeof(VertexTextured), (void*)(3 * sizeof(float)));
//
//
//	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, platformVBOs[1] );
//	glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( platformIndices ), platformIndices, GL_STATIC_DRAW );

    struct VertexTextured {
        float x, y, z;
        float s, t;
    };

    // create our skybox
    VertexTextured negY_Vertices[4] = {
            { -25.0f, 0.0f, -25.0f , 0.0f, 0.0f}, { 25.0f, 0.0f, -25.0f , 1.0f, 0.0f},
            { -25.0f, 0.0f, 25.0f , 0.0f, 1.0f}, { 25.0f, 0.0f, 25.0f , 1.0f, 1.0f}
    };
    VertexTextured posY_Vertices[4] = {
            { -25.0f, 50.0f, -25.0f , 0.0f, 1.0f}, { 25.0f, 50.0f, -25.0f , 1.0f, 1.0f},
            { -25.0f, 50.0f, 25.0f , 0.0f, 0.0f}, { 25.0f, 50.0f, 25.0f , 1.0f, 0.0f}
    };
    VertexTextured negX_Vertices[4] = {
            { -25.0f, 0.0f, -25.0f , 0.0f, 0.0f}, { -25.0f, 0.0f, 25.0f , 1.0f, 0.0f},
            { -25.0f, 50.0f, -25.0f , 0.0f, 1.0f}, { -25.0f, 50.0f, 25.0f , 1.0f, 1.0f}
    };
    VertexTextured posX_Vertices[4] = {
            { 25.0f, 0.0f, -25.0f , 1.0f, 0.0f}, { 25.0f, 0.0f, 25.0f , 0.0f, 0.0f},
            { 25.0f, 50.0f, -25.0f , 1.0f, 1.0f}, { 25.0f, 50.0f, 25.0f , 0.0f, 1.0f}
    };
    VertexTextured negZ_Vertices[4] = {
            { -25.0f, 0.0f, -25.0f , 1.0f, 0.0f}, { 25.0f, 0.0f, -25.0f , 0.0f, 0.0f},
            { -25.0f, 50.0f, -25.0f , 1.0f, 1.0f}, { 25.0f, 50.0f, -25.0f , 0.0f, 1.0f}
    };
    VertexTextured posZ_Vertices[4] = {
            { -25.0f, 0.0f, 25.0f , 0.0f, 0.0f}, { 25.0f, 0.0f, 25.0f , 1.0f, 0.0f},
            { -25.0f, 50.0f, 25.0f , 0.0f, 1.0f}, { 25.0f, 50.0f, 25.0f , 1.0f, 1.0f}
    };

    unsigned short platformIndices[4] = { 0, 1, 2, 3 };

    glGenVertexArrays( 1, &negY_VAO );
    glBindVertexArray( negY_VAO );
    glGenBuffers( 2, negY_VBOs );
    glBindBuffer( GL_ARRAY_BUFFER, negY_VBOs[0] );
    glBufferData( GL_ARRAY_BUFFER, sizeof( negY_Vertices ), negY_Vertices, GL_STATIC_DRAW );
    glEnableVertexAttribArray( texShaderProgramAttributes.vPos );
    glVertexAttribPointer( texShaderProgramAttributes.vPos, 3, GL_FLOAT, GL_FALSE, sizeof(VertexTextured), (void*) 0 );
    glEnableVertexAttribArray( texShaderProgramAttributes.texCoord_attr_loc );
    glVertexAttribPointer( texShaderProgramAttributes.texCoord_attr_loc, 2, GL_FLOAT, GL_FALSE, sizeof(VertexTextured), (void*)(3 * sizeof(float)));
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, negY_VBOs[1] );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( platformIndices ), platformIndices, GL_STATIC_DRAW );

    glGenVertexArrays( 1, &posY_VAO );
    glBindVertexArray( posY_VAO );
    glGenBuffers( 2, posY_VBOs );
    glBindBuffer( GL_ARRAY_BUFFER, posY_VBOs[0] );
    glBufferData( GL_ARRAY_BUFFER, sizeof( posY_Vertices ), posY_Vertices, GL_STATIC_DRAW );
    glEnableVertexAttribArray( texShaderProgramAttributes.vPos );
    glVertexAttribPointer( texShaderProgramAttributes.vPos, 3, GL_FLOAT, GL_FALSE, sizeof(VertexTextured), (void*) 0 );
    glEnableVertexAttribArray( texShaderProgramAttributes.texCoord_attr_loc );
    glVertexAttribPointer( texShaderProgramAttributes.texCoord_attr_loc, 2, GL_FLOAT, GL_FALSE, sizeof(VertexTextured), (void*)(3 * sizeof(float)));
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, posY_VBOs[1] );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( platformIndices ), platformIndices, GL_STATIC_DRAW );

    glGenVertexArrays( 1, &negX_VAO );
    glBindVertexArray( negX_VAO );
    glGenBuffers( 2, negX_VBOs );
    glBindBuffer( GL_ARRAY_BUFFER, negX_VBOs[0] );
    glBufferData( GL_ARRAY_BUFFER, sizeof( negX_Vertices ), negX_Vertices, GL_STATIC_DRAW );
    glEnableVertexAttribArray( texShaderProgramAttributes.vPos );
    glVertexAttribPointer( texShaderProgramAttributes.vPos, 3, GL_FLOAT, GL_FALSE, sizeof(VertexTextured), (void*) 0 );
    glEnableVertexAttribArray( texShaderProgramAttributes.texCoord_attr_loc );
    glVertexAttribPointer( texShaderProgramAttributes.texCoord_attr_loc, 2, GL_FLOAT, GL_FALSE, sizeof(VertexTextured), (void*)(3 * sizeof(float)));
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, negX_VBOs[1] );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( platformIndices ), platformIndices, GL_STATIC_DRAW );

    glGenVertexArrays( 1, &posX_VAO );
    glBindVertexArray( posX_VAO );
    glGenBuffers( 2, posX_VBOs );
    glBindBuffer( GL_ARRAY_BUFFER, posX_VBOs[0] );
    glBufferData( GL_ARRAY_BUFFER, sizeof( posX_Vertices ), posX_Vertices, GL_STATIC_DRAW );
    glEnableVertexAttribArray( texShaderProgramAttributes.vPos );
    glVertexAttribPointer( texShaderProgramAttributes.vPos, 3, GL_FLOAT, GL_FALSE, sizeof(VertexTextured), (void*) 0 );
    glEnableVertexAttribArray( texShaderProgramAttributes.texCoord_attr_loc );
    glVertexAttribPointer( texShaderProgramAttributes.texCoord_attr_loc, 2, GL_FLOAT, GL_FALSE, sizeof(VertexTextured), (void*)(3 * sizeof(float)));
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, posX_VBOs[1] );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( platformIndices ), platformIndices, GL_STATIC_DRAW );

    glGenVertexArrays( 1, &negZ_VAO );
    glBindVertexArray( negZ_VAO );
    glGenBuffers( 2, negZ_VBOs );
    glBindBuffer( GL_ARRAY_BUFFER, negZ_VBOs[0] );
    glBufferData( GL_ARRAY_BUFFER, sizeof( negZ_Vertices ), negZ_Vertices, GL_STATIC_DRAW );
    glEnableVertexAttribArray( texShaderProgramAttributes.vPos );
    glVertexAttribPointer( texShaderProgramAttributes.vPos, 3, GL_FLOAT, GL_FALSE, sizeof(VertexTextured), (void*) 0 );
    glEnableVertexAttribArray( texShaderProgramAttributes.texCoord_attr_loc );
    glVertexAttribPointer( texShaderProgramAttributes.texCoord_attr_loc, 2, GL_FLOAT, GL_FALSE, sizeof(VertexTextured), (void*)(3 * sizeof(float)));
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, negZ_VBOs[1] );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( platformIndices ), platformIndices, GL_STATIC_DRAW );

    glGenVertexArrays( 1, &posZ_VAO );
    glBindVertexArray( posZ_VAO );
    glGenBuffers( 2, posZ_VBOs );
    glBindBuffer( GL_ARRAY_BUFFER, posZ_VBOs[0] );
    glBufferData( GL_ARRAY_BUFFER, sizeof( posZ_Vertices ), posZ_Vertices, GL_STATIC_DRAW );
    glEnableVertexAttribArray( texShaderProgramAttributes.vPos );
    glVertexAttribPointer( texShaderProgramAttributes.vPos, 3, GL_FLOAT, GL_FALSE, sizeof(VertexTextured), (void*) 0 );
    glEnableVertexAttribArray( texShaderProgramAttributes.texCoord_attr_loc );
    glVertexAttribPointer( texShaderProgramAttributes.texCoord_attr_loc, 2, GL_FLOAT, GL_FALSE, sizeof(VertexTextured), (void*)(3 * sizeof(float)));
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, posZ_VBOs[1] );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( platformIndices ), platformIndices, GL_STATIC_DRAW );

	fprintf( stdout, "[INFO]: platform read in with VAO %d\n", platformVAO );

    // TODO #21 create our own custom quad
////UNCOMMENT BEGINNING HERE
    VertexTextured quadVertices[4] = {
            { -2.5f, -2.5f,  0.0f, 0.0f, 0.0f }, // 0 - BL
            {  2.5f, -2.5f,  0.0f, 3.0f, 0.0f }, // 1 - BR
            { -2.5f,  2.5f,  0.0f, 0.0f, 3.0f }, // 2 - TL
            {  2.5f,  2.5f,  0.0f, 3.0f, 3.0f }  // 3 - TR
    };

    unsigned short quadIndices[4] = { 0, 1, 2, 3 };

    glGenVertexArrays( 1, &quadVAO );
    glBindVertexArray( quadVAO );

    glGenBuffers( 2, quadVBOs );

    glBindBuffer( GL_ARRAY_BUFFER, quadVBOs[0] );
    glBufferData( GL_ARRAY_BUFFER, sizeof( quadVertices ), quadVertices, GL_STATIC_DRAW );

    glEnableVertexAttribArray( texShaderProgramAttributes.vPos );
    glVertexAttribPointer( texShaderProgramAttributes.vPos, 3, GL_FLOAT, GL_FALSE, sizeof(VertexTextured), (void*) 0 );

    // Repeat TODO #16 to connect this VBO with our shader
    glEnableVertexAttribArray( texShaderProgramAttributes.texCoord_attr_loc );
    glVertexAttribPointer( texShaderProgramAttributes.texCoord_attr_loc, 2, GL_FLOAT, GL_FALSE, sizeof(VertexTextured), (void*)(sizeof(float) * 3) );

    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, quadVBOs[1] );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( quadIndices ), quadIndices, GL_STATIC_DRAW );

    fprintf( stdout, "[INFO]: quad read in with VAO %d\n\n", quadVAO );
////UNCOMMENT ENDING HERE

	// load suzanne
    model = new CSCI441::ModelLoader();
    model->loadModelFile( "assets/models/suzanne/suzanne.obj" );
}

// setupTextures() /////////////////////////////////////////////////////////////
//
//      Register all of our textures with the GPU
//
////////////////////////////////////////////////////////////////////////////////
void setupTextures() {
    // TODO #09 load and register the metal texture
    metalTexHandle = loadAndRegisterTexture("assets/textures/metal.jpg");
    minesTexHandle = loadAndRegisterTexture("assets/textures/mines.png");

    negY_TexHandle = loadAndRegisterTexture("assets/negy.jpg");
    posY_TexHandle = loadAndRegisterTexture("assets/posy.jpg");
    negX_TexHandle = loadAndRegisterTexture("assets/negx.jpg");
    posX_TexHandle = loadAndRegisterTexture("assets/posx.jpg");
    negZ_TexHandle = loadAndRegisterTexture("assets/negz.jpg");
    posZ_TexHandle = loadAndRegisterTexture("assets/posz.jpg");
}

// setupScene() ////////////////////////////////////////////////////////////////
//
//      Initialize all of our scene information here
//
////////////////////////////////////////////////////////////////////////////////
void setupScene() {
    leftMouseDown = GL_FALSE;
    controlDown = GL_FALSE;
    mousePosition = glm::vec2( -9999.0f, -9999.0f );

    arcballCam.cameraAngles = glm::vec3( 3.52f, 1.9f, 15.0f );
    arcballCam.camDir = glm::vec3(-1.0f, -1.0f, -1.0f);
    arcballCam.lookAtPoint = glm::vec3(0.0f, 0.0f, 0.0f);
    arcballCam.upVector = glm::vec3(    0.0f,  1.0f,  0.0f );
    updateCameraDirection();

    objectIndex = 2;            // start with a sphere
}

// initialize() /////////////////////////////////////////////////////////////////
//
//      Create our OpenGL context,
//          load all information to the GPU,
//          initialize scene information
//
////////////////////////////////////////////////////////////////////////////////
GLFWwindow* initialize() {
    // GLFW sets up our OpenGL context so must be done first
    GLFWwindow* window = setupGLFW();	                // initialize all of the GLFW specific information related to OpenGL and our window
    setupGLEW();										// initialize all of the GLEW specific information
    setupOpenGL();										// initialize all of the OpenGL specific information

    CSCI441::OpenGLUtils::printOpenGLInfo();            // print our OpenGL information

    setupShaders();                                     // load all of our shader programs onto the GPU and get shader input locations
    setupBuffers();										// load all our VAOs and VBOs onto the GPU
    setupTextures();                                    // load all of our textures onto the GPU
    setupScene();                                       // initialize all of our scene information

    fprintf( stdout, "\n[INFO]: Setup complete\n" );

    return window;
}

///***********************************************************************************************************************************************************
//
// Cleanup Functions

// cleanupShaders() ////////////////////////////////////////////////////////////
//
//      Delete shaders off of the GPU
//
////////////////////////////////////////////////////////////////////////////////
void cleanupShaders() {
    fprintf( stdout, "[INFO]: ...deleting shaders.\n" );

    delete texShaderProgram;
}

// cleanupBuffers() ////////////////////////////////////////////////////////////
//
//      Delete VAOs and VBOs off of the GPU
//
////////////////////////////////////////////////////////////////////////////////
void cleanupBuffers() {
    fprintf( stdout, "[INFO]: ...deleting VBOs....\n" );

    glDeleteBuffers( 2, platformVBOs );

    glDeleteBuffers(2, negY_VBOs);
    glDeleteBuffers(2, posY_VBOs);
    glDeleteBuffers(2, negX_VBOs);
    glDeleteBuffers(2, posX_VBOs);
    glDeleteBuffers(2, negZ_VBOs);
    glDeleteBuffers(2, posZ_VBOs);

    glDeleteBuffers( 2, quadVBOs );
    CSCI441::deleteObjectVBOs();

    fprintf( stdout, "[INFO]: ...deleting VAOs....\n" );

    glDeleteBuffers( 1, &platformVAO );

    glDeleteBuffers(1, &negY_VAO);
    glDeleteBuffers(1, &posY_VAO);
    glDeleteBuffers(1, &negX_VAO);
    glDeleteBuffers(1, &posX_VAO);
    glDeleteBuffers(1, &negZ_VAO);
    glDeleteBuffers(1, &posZ_VAO);

    glDeleteBuffers( 1, &quadVAO );
    CSCI441::deleteObjectVAOs();

    delete model;
}

// cleanupTextures() ///////////////////////////////////////////////////////////
//
//      Delete textures off of the GPU
//
////////////////////////////////////////////////////////////////////////////////
void cleanupTextures() {
    fprintf( stdout, "[INFO]: ...deleting textures\n" );

    // TODO #22 delete our textures from the GPU
    glDeleteTextures(1, &metalTexHandle);
    glDeleteTextures(1, &minesTexHandle);

    glDeleteTextures(1, &negY_TexHandle);
    glDeleteTextures(1, &posY_TexHandle);
    glDeleteTextures(1, &negX_TexHandle);
    glDeleteTextures(1, &posX_TexHandle);
    glDeleteTextures(1, &negZ_TexHandle);
    glDeleteTextures(1, &posZ_TexHandle);
}

// shutdown() ///////////////////////////////////////////////////////////////////
//
//      Free all memory on the CPU/GPU and close our OpenGL context
//
////////////////////////////////////////////////////////////////////////////////
void shutdown(GLFWwindow* window) {
    fprintf( stdout, "\n[INFO]: Shutting down.......\n" );
    fprintf( stdout, "[INFO]: ...closing window...\n" );
    glfwDestroyWindow( window );                        // close our window
    cleanupShaders();                                   // delete shaders from GPU
    cleanupBuffers();                                   // delete VAOs/VBOs from GPU
    cleanupTextures();                                  // delete textures from GPU
    fprintf( stdout, "[INFO]: ...closing GLFW.....\n" );
    glfwTerminate();						            // shut down GLFW to clean up our context
    fprintf( stdout, "[INFO]: ..shut down complete!\n" );
}

///***********************************************************************************************************************************************************
//
// Rendering / Drawing Functions - this is where the magic happens!

// renderScene() ///////////////////////////////////////////////////////////////
//
//		This method will contain all of the objects to be drawn.
//
////////////////////////////////////////////////////////////////////////////////
void renderScene( glm::mat4 viewMatrix, glm::mat4 projectionMatrix ) {
    texShaderProgram->useProgram();

    glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.1f, 0.0f));
    glm::mat4 mvpMtx = projectionMatrix * viewMatrix * modelMatrix;
    glUniformMatrix4fv(texShaderProgramUniforms.mvpMatrix, 1, GL_FALSE, &mvpMtx[0][0]);

    // TODO #19 apply the metal texture to the ground platform
//    glBindTexture(GL_TEXTURE_2D, metalTexHandle);
//	glBindVertexArray( platformVAO );
//	glDrawElements( GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, (void*)0 );

    glBindTexture(GL_TEXTURE_2D, negY_TexHandle);
    glBindVertexArray( negY_VAO );
    glDrawElements( GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, (void*)0 );

    glBindTexture(GL_TEXTURE_2D, posY_TexHandle);
    glBindVertexArray( posY_VAO );
    glDrawElements( GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, (void*)0 );

    glBindTexture(GL_TEXTURE_2D, negX_TexHandle);
    glBindVertexArray( negX_VAO );
    glDrawElements( GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, (void*)0 );

    glBindTexture(GL_TEXTURE_2D, posX_TexHandle);
    glBindVertexArray( posX_VAO );
    glDrawElements( GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, (void*)0 );

    glBindTexture(GL_TEXTURE_2D, negZ_TexHandle);
    glBindVertexArray( negZ_VAO );
    glDrawElements( GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, (void*)0 );

    glBindTexture(GL_TEXTURE_2D, posZ_TexHandle);
    glBindVertexArray( posZ_VAO );
    glDrawElements( GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, (void*)0 );

    modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 2.1f, 0.0f));
    modelMatrix = glm::rotate( modelMatrix, objectAngle, CSCI441::Y_AXIS );
    mvpMtx = projectionMatrix * viewMatrix * modelMatrix;
    glUniformMatrix4fv(texShaderProgramUniforms.mvpMatrix, 1, GL_FALSE, &mvpMtx[0][0]);

    // TODO #20 apply the Mines texture to all of our objects
    glBindTexture(GL_TEXTURE_2D, minesTexHandle);
    // draw all the cool stuff!
    switch( objectIndex ) {
        case 0: CSCI441::drawSolidTeapot( 2.0f );                                                break;
        case 1: CSCI441::drawSolidCubeFlat( 4.0f );                                          break;
        case 2: CSCI441::drawSolidSphere( 3.0f, 16, 16 );                          break;
        case 3: CSCI441::drawSolidTorus( 1.0f, 2.0f, 16, 16 );        break;
        case 4: CSCI441::drawSolidCone( 2.0f, 4.0f, 16, 16 );                break;
        case 5: CSCI441::drawSolidCylinder( 2.0f, 2.0f, 4.0f, 16, 16 ); break;
        case 6:
            model->draw( texShaderProgramAttributes.vPos,
                         -1,          // vertex normals not used in this lab
                         texShaderProgramAttributes.texCoord_attr_loc );      // TODO #18 set actual vertex texture coordinate location
            break;
        case 7:
            glBindVertexArray(quadVAO);
            glDrawElements( GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, (void*)0 );
            break;
        default: break;
    }
}

// updateScene() ////////////////////////////////////////////////////////////////
//
//      Update all of our scene objects - perform animation here
//
////////////////////////////////////////////////////////////////////////////////
void updateScene() {
    objectAngle += ROTATION_SPEED;
    if( objectAngle >= 6.28f ) objectAngle -= 6.28f;
}

// run() ///////////////////////////////////////////////////////////////////////
//
//      Runs our draw loop and renders/updates our scene
//
////////////////////////////////////////////////////////////////////////////////
void run(GLFWwindow* window) {
    // hook up the CSCI441 object library to our shader program - MUST be done after the shader is used and before the objects are drawn
    // if we have multiple shaders the flow would be:
    //      1) shader->useProgram()
    //      2) CSCI441::setVertexAttributeLocations()
    //      3) CSCI441::draw*()
    // but this lab only has one shader program ever in use so we are safe to assign these values just once
    //
    // OR the alternative is to ensure that all of your shader programs use the
    // same attribute locations for the vertex position, normal, and texture coordinate
    CSCI441::setVertexAttributeLocations(texShaderProgramAttributes.vPos,   // vertex position location
                                         -1,                   // vertex normal location not used, set to -1
                                         texShaderProgramAttributes.texCoord_attr_loc );               // TODO #17 set actual vertex texture coordinate location

    //  This is our draw loop - all rendering is done here.  We use a loop to keep the window open
    //	until the user decides to close the window and quit the program.  Without a loop, the
    //	window will display once and then the program exits.
    while( !glfwWindowShouldClose(window) ) {	        // check if the window was instructed to be closed
        glDrawBuffer( GL_BACK );				        // work with our back frame buffer
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );	// clear the current color contents and depth buffer in the window

        // Get the size of our framebuffer.  Ideally this should be the same dimensions as our window, but
        // when using a Retina display the actual window can be larger than the requested window.  Therefore
        // query what the actual size of the window we are rendering to is.
        GLint framebufferWidth, framebufferHeight;
        glfwGetFramebufferSize( window, &framebufferWidth, &framebufferHeight );

        // update the viewport - tell OpenGL we want to render to the whole window
        glViewport( 0, 0, framebufferWidth, framebufferHeight );

        // set the projection matrix based on the window size
        // use a perspective projection that ranges
        // with a FOV of 45 degrees, for our current aspect ratio, and Z ranges from [0.001, 1000].
        glm::mat4 projectionMatrix = glm::perspective( 45.0f, (GLfloat) WINDOW_WIDTH / (GLfloat) WINDOW_HEIGHT, 0.001f, 100.0f );

        // set up our look at matrix to position our camera
        glm::mat4 viewMatrix = glm::lookAt( arcballCam.lookAtPoint + arcballCam.camDir * arcballCam.cameraAngles.z,
                                            arcballCam.lookAtPoint,
                                            arcballCam.upVector );

        // draw everything to the window
        // pass our view and projection matrices
        renderScene( viewMatrix, projectionMatrix );

        glfwSwapBuffers(window);                        // flush the OpenGL commands and make sure they get rendered!
        glfwPollEvents();				                // check for any events and signal to redraw screen

        updateScene();                                  // update the objects in our scene
    }
}

///**********************************************************************************************************************************************************
//
// Our main function

// main() //////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
int main() {
	GLFWwindow *window = initialize();                  // create OpenGL context and setup EVERYTHING for our program
	run(window);                                        // enter our draw loop and run our program
    shutdown(window);                                   // free up all the memory used and close OpenGL context
    return EXIT_SUCCESS;				                // exit our program successfully!
}
