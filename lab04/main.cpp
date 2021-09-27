/*
 *  CSCI 441, Computer Graphics, Fall 2020
 *
 *  Project: lab04
 *  File: main.cpp
 *
 *  Description:
 *      This file contains the basic setup to work with GLSL shaders.
 *
 *  Author: Dr. Paone, Colorado School of Mines, 2020
 *
 */

//*************************************************************************************

#include <GL/glew.h>                    // include GLEW to get our OpenGL 3.0+ bindings
#include <GLFW/glfw3.h>			        // include GLFW framework header

// include GLM libraries and matrix functions
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <cstdio>				        // for printf functionality
#include <cstdlib>				        // for exit functionality

// note that all of these headers end in *3.hpp
// these class library files will only work with OpenGL 3.0+
#include <CSCI441/modelLoader.hpp>      // to load in OBJ models
#include <CSCI441/OpenGLUtils.hpp>      // to print info about OpenGL
#include <CSCI441/objects.hpp>          // to render our 3D primitives

#include "lab04.hpp"                    // our shader helper functions

// THIS MUST BE THE LAST INCLUDE OF YOUR PROJECT!!
// needed for texturing which is included internally in the modelLoader
// this define must be present in only one file within your build path
// so we will always put it in our main.cpp file
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

//*************************************************************************************
//
// Global Parameters

int windowWidth, windowHeight;

// global variables to keep track of all our mouse information
GLboolean leftMouseDown;
glm::vec2 mousePosition;

// global variables to keep track of all our camera information
// cameraAngles --> x = theta, y = phi, z = radius
glm::vec3 cameraAngles, eyePoint, lookAtPoint, camDir, upVector;
// cameraSpeed --> x = forward/backward delta, y = rotational delta
glm::vec2 cameraSpeed;

// keep track of our key states
GLboolean keys[256] = {0};

GLuint objectIndex;

CSCI441::ModelLoader* model = nullptr;  // assign as a null pointer to delay creation until
                                        // after OpenGL context has been created

GLuint shaderProgramHandle;
GLint mvp_uniform_location, time_uniform_location;
GLint vpos_attrib_location;

//******************************************************************************
//
// Helper Functions

// updateLookAtPoint() /////////////////////////////////////////////////////////
//
// This function updates the camera's lookAt position in cartesian coordinates
//   It should be called every time the eyePoint or camera Direction changes
//
////////////////////////////////////////////////////////////////////////////////
void updateLookAtPoint() {
    lookAtPoint = eyePoint + camDir;
}

// updateCameraDirection() /////////////////////////////////////////////////////
//
// This function updates the camera's direction in cartesian coordinates based
//  on its position in spherical coordinates. Should be called every time
//  cameraAngles is updated.
//
////////////////////////////////////////////////////////////////////////////////
void updateCameraDirection() {
    if( cameraAngles.y <= 0 ) cameraAngles.y = 0.0f + 0.001f;
    if( cameraAngles.y >= M_PI ) cameraAngles.y = M_PI - 0.001f;

    camDir.x = cameraAngles.z * sinf( cameraAngles.x ) * sinf( cameraAngles.y );
    camDir.y = cameraAngles.z * -cosf( cameraAngles.y );
    camDir.z = cameraAngles.z * -cosf( cameraAngles.x ) * sinf( cameraAngles.y );
    camDir = glm::normalize(camDir);
    updateLookAtPoint();
}

//******************************************************************************
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
    if( action == GLFW_PRESS ) {
        // store that a key was pressed and is being held down
        keys[key] = true;

        switch( key ) {
            // closes the window
            case GLFW_KEY_ESCAPE:
            case GLFW_KEY_Q:
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
                objectIndex = key - GLFW_KEY_1; // GLFW_KEY_1 is 49.  they go in sequence from there
                break;

            default:
                break;
        }
    } else if( action == GLFW_RELEASE ) {
        // store that a key was released and is no longer being held down
        keys[key] = false;
    }
}

// mouse_button_callback() /////////////////////////////////////////////////////
//
//		We will register this function as GLFW's mouse button callback.
//	Responds to mouse button presses and mouse button releases.
//
////////////////////////////////////////////////////////////////////////////////
static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	if( button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS ) {
		leftMouseDown = true;
	} else {
		leftMouseDown = false;
		mousePosition.x = -9999.0f;
		mousePosition.y = -9999.0f;
	}
}

// cursor_callback() ///////////////////////////////////////////////////////////
//
//		We will register this function as GLFW's cursor movement callback.
//	Responds to mouse movement.  When active motion is used with the left
//	mouse button a free cam camera model is followed.
//
////////////////////////////////////////////////////////////////////////////////
static void cursor_callback( GLFWwindow* window, double xPos, double yPos) {
    // make sure movement is in bounds of the window
    // glfw captures mouse movement on entire screen
    if( xPos > 0 && xPos < windowWidth ) {
        if( yPos > 0 && yPos < windowHeight ) {
            // active motion
            if( leftMouseDown ) {
                if( (mousePosition.x - -9999.0f) < 0.001f ) {
                    mousePosition.x = xPos;
                    mousePosition.y = yPos;
                } else {
                    cameraAngles.x += (xPos - mousePosition.x) * 0.005f;
                    cameraAngles.y += (mousePosition.y - yPos) * 0.005f;

                    updateCameraDirection();

                    mousePosition.x = xPos;
                    mousePosition.y = yPos;
                }
            }
                // passive motion
            else {

            }
        }
    }
}

//******************************************************************************
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

	glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );			// request forward compatible OpenGL context
	glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );// request OpenGL Core Profile context
	glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );		    // request OpenGL 4.x context
	glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 1 );		    // request OpenGL x.1 context
	glfwWindowHint( GLFW_DOUBLEBUFFER, GLFW_TRUE );                 // request double buffering

	// create a window for a given size, with a given title
	GLFWwindow *window = glfwCreateWindow(640, 480, "Lab04: GLSL Shaders", nullptr, nullptr);
	if( !window ) {						                            // if the window could not be created, NULL is returned
		fprintf( stderr, "[ERROR]: GLFW Window could not be created\n" );
		glfwTerminate();
		exit( EXIT_FAILURE );
	} else {
		fprintf( stdout, "[INFO]: GLFW Window created\n" );
	}

	glfwMakeContextCurrent(	window );	                            // make the created window the current window
	glfwSwapInterval( 1 );				                    // update our screen after at least 1 screen refresh

	glfwSetKeyCallback( 	    window, key_callback	      );	// set our keyboard callback function
	glfwSetMouseButtonCallback( window, mouse_button_callback );	// set our mouse button callback function
	glfwSetCursorPosCallback(	window, cursor_callback  	  );	// set our cursor position callback function

	return window;										            // return the window that was created
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
		printf( "[ERROR]: Error initializing GLEW\n");
		// Problem: glewInit failed, something is seriously wrong.
  	    fprintf( stderr, "[ERROR]: %s\n", glewGetErrorString(glewResult) );
		exit(EXIT_FAILURE);
	} else {
		 fprintf( stdout, "[INFO]: GLEW initialized\n" );
	}
}

// setupShaders() //////////////////////////////////////////////////////////////
//
//      Create our shaders.  Send GLSL code to GPU to be compiled.  Also get
//  handles to our uniform and attribute locations.
//
////////////////////////////////////////////////////////////////////////////////
void setupShaders() {
    // TODO #09 create the shader program and set the returned handled to our global shaderProgramHandle variable
    //  the shaders currently exist in "shaders/customShader.v.glsl" and "shaders/customShader.f.glsl"
    shaderProgramHandle = createShaderProgram("shaders/customShader.v.glsl", "shaders/customShader.f.glsl");
    // TODO #10A get the location of our ModelViewProjection uniform and set it to the global variable mvp_uniform_location
    mvp_uniform_location = glGetUniformLocation(shaderProgramHandle, "mvpMatrix");
    if (mvp_uniform_location < 0) {
        fprintf( stderr, "[ERROR]: mvp_uniform_location returned a negative number\n" );
        exit(EXIT_FAILURE);
    }
    // TODO #12A get the location of our time uniform and set it to the global variable time_uniform_location
    time_uniform_location = glGetUniformLocation(shaderProgramHandle, "time");
    if (time_uniform_location < 0) {
        fprintf( stderr, "[ERROR]: time_uniform_location returned a negative number\n" );
        exit(EXIT_FAILURE);
    }
    // TODO #10B get the location of our vertex position attribute and set it to the global variable vpos_attrib_location
    vpos_attrib_location = glGetAttribLocation(shaderProgramHandle, "vPosition");
    if (vpos_attrib_location < 0) {
        fprintf( stderr, "[ERROR]: vpos_attrib_location returned a negative number\n" );
        exit(EXIT_FAILURE);
    }

}

// setupBuffers() //////////////////////////////////////////////////////////////
//
//      Create our VAOs & VBOs. Send vertex data to the GPU for future rendering
//
////////////////////////////////////////////////////////////////////////////////
void setupBuffers() {
  model = new CSCI441::ModelLoader();
  model->loadModelFile( "models/suzanne.obj" );
}

// setupScene() ////////////////////////////////////////////////////////////////
//
//      Initialize all of our global variables to set up the starting
//      scene position
//
////////////////////////////////////////////////////////////////////////////////
void setupScene() {
    leftMouseDown = false;
    mousePosition = glm::vec2( -9999.0f, -9999.0f );

    cameraAngles = glm::vec3( 5.52f, 0.9f, 1.0f );
    eyePoint = glm::vec3(   10.0f, 10.0f, 10.0f );
    camDir = glm::vec3(-1.0f, -1.0f, -1.0f);
    upVector = glm::vec3(    0.0f,  1.0f,  0.0f );
    cameraSpeed = glm::vec2(0.25f, 0.02f);
    updateCameraDirection();    // internally calls updateLookAtPoint()

    objectIndex = 2;            // start with a sphere
}

//******************************************************************************
//
// Rendering / Drawing Functions - this is where the magic happens!

// renderScene() ///////////////////////////////////////////////////////////////
//
//		This method will contain all of the objects to be drawn.
//
////////////////////////////////////////////////////////////////////////////////
void renderScene( glm::mat4 viewMtx, glm::mat4 projMtx ) {
    // stores our model matrix
    glm::mat4 modelMtx(1.0f);

    // TODO #11A use our shader program
    glUseProgram(shaderProgramHandle);

    // precompute our MVP CPU side so it only needs to be computed once
    glm::mat4 mvpMtx = projMtx * viewMtx * modelMtx;
    // TODO #11B send the MVP matrix to the GPU
    glUniformMatrix4fv(mvp_uniform_location, 1, GL_FALSE, &mvpMtx[0][0]);

    // TODO #12B send the time to GPU
    glUniform1f(time_uniform_location, glfwGetTime());

    // draw all the cool stuff!
    switch( objectIndex ) {
        case 0: CSCI441::drawSolidTeapot( 2.0f );                                                break;
        case 1: CSCI441::drawSolidCube( 4.0f );                                              break;
        case 2: CSCI441::drawSolidSphere( 3.0f, 16, 16 );                          break;
        case 3: CSCI441::drawSolidTorus( 1.0f, 2.0f, 16, 16 );        break;
        case 4: CSCI441::drawSolidCone( 2.0f, 4.0f, 16, 16 );                break;
        case 5: CSCI441::drawSolidCylinder( 2.0f, 2.0f, 4.0f, 16, 16 ); break;
        case 6: model->draw( vpos_attrib_location );                                                   break;
        default: break;
    }
}

// updateScene() ///////////////////////////////////////////////////////////////
//
//		This method will move our camera
//
////////////////////////////////////////////////////////////////////////////////
void updateScene() {
    // go forward
    if( keys[GLFW_KEY_SPACE] ) {
        eyePoint += camDir * cameraSpeed.x;
        updateLookAtPoint();
    }
    // go backward
    if( keys[GLFW_KEY_X] ) {
        eyePoint -= camDir * cameraSpeed.x;
        updateLookAtPoint();
    }
    // turn right
    if( keys[GLFW_KEY_D] ) {
        cameraAngles.x += cameraSpeed.y;
        updateCameraDirection();
    }
    // turn left
    if( keys[GLFW_KEY_A] ) {
        cameraAngles.x -= cameraSpeed.y;
        updateCameraDirection();
    }
    // pitch up
    if( keys[GLFW_KEY_W] ) {
        cameraAngles.y += cameraSpeed.y;
        updateCameraDirection();
    }
    // pitch down
    if( keys[GLFW_KEY_S] ) {
        cameraAngles.y -= cameraSpeed.y;
        updateCameraDirection();
    }
}

///*****************************************************************************
//
// Our main function

// main() ///////////////////////////////////////////////////////////////
//
//		Really you should know what this is by now.
//
////////////////////////////////////////////////////////////////////////////////
int main() {
    // GLFW sets up our OpenGL context so must be done first
	GLFWwindow *window = setupGLFW();	                    // initialize all of the GLFW specific information related to OpenGL and our window
	setupOpenGL();										    // initialize all of the OpenGL specific information
	setupGLEW();											// initialize all of the GLEW specific information

    CSCI441::OpenGLUtils::printOpenGLInfo();

	setupShaders();                                         // load our shader program into memory
	setupBuffers();										    // load all our VAOs and VBOs into memory
	setupScene();                                           // assign our initial scene information

    // LOOK HERE #2 needed to connect our 3D Object Library to our shader
    CSCI441::setVertexAttributeLocations( vpos_attrib_location );

    updateCameraDirection();		                    // set up our camera position

    //  This is our draw loop - all rendering is done here.  We use a loop to keep the window open
	//	until the user decides to close the window and quit the program.  Without a loop, the
	//	window will display once and then the program exits.
	while( !glfwWindowShouldClose(window) ) {	            // check if the window was instructed to be closed
    glDrawBuffer( GL_BACK );				                // work with our back frame buffer
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );	// clear the current color contents and depth buffer in the window

		// Get the size of our framebuffer.  Ideally this should be the same dimensions as our window, but
		// when using a Retina display the actual window can be larger than the requested window.  Therefore
		// query what the actual size of the window we are rendering to is.
		glfwGetFramebufferSize( window, &windowWidth, &windowHeight );

		// update the viewport - tell OpenGL we want to render to the whole window
		glViewport( 0, 0, windowWidth, windowHeight );

		// set the projection matrix based on the window size
		// use a perspective projection that ranges
		// with a FOV of 45 degrees, for our current aspect ratio, and Z ranges from [0.001, 1000].
		glm::mat4 projectionMatrix = glm::perspective( 45.0f, (GLfloat) windowWidth / (GLfloat) windowHeight, 0.001f, 100.0f );

		// set up our look at matrix to position our camera
		glm::mat4 viewMatrix = glm::lookAt( eyePoint,lookAtPoint, upVector );

		// draw everything to the window
		// pass our view and projection matrices as well as deltaTime between frames
		renderScene( viewMatrix, projectionMatrix );

		glfwSwapBuffers(window);                            // flush the OpenGL commands and make sure they get rendered!
		glfwPollEvents();				                    // check for any events and signal to redraw screen

		updateScene();                                      // perform any animations or updates to our scene
	}

    glfwDestroyWindow( window );                            // clean up and close our window
	glfwTerminate();						                // shut down GLFW to clean up our context

	return EXIT_SUCCESS;
}
