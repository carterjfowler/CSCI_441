/*
 *  CSCI 441, Computer Graphics, Fall 2020
 *
 *  Project: lab02
 *  File: main.cpp
 *
 *	Author: Jeffrey Paone - Fall 2020
 *
 *  Description:
 *      Contains the base code for a basic flight simulator.
 *
 */

// include the OpenGL library headers
#include <GL/glew.h>
#include <GLFW/glfw3.h>			// include GLFW framework header

// include GLM libraries and matrix functions
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// include C and C++ libraries
#include <cmath>				// for cos(), sin() functionality
#include <cstdio>				// for printf functionality
#include <cstdlib>			    // for exit functionality
#include <ctime>			    // for time() functionality
#include <vector>

// include our class libraries
#include <CSCI441/OpenGLUtils.hpp>
#include <CSCI441/objects.hpp>  // for our 3D objects
#include <CSCI441/SimpleShader.hpp>

//*************************************************************************************
//
// Global Parameters

// global variables to keep track of window width and height.
// set to initial values for convenience, but we need variables
// for later on in case the window gets resized.
const GLint WINDOW_WIDTH = 640, WINDOW_HEIGHT = 480;

int leftMouseButton;    	 				// status of the mouse button
double mouseX = -99999, mouseY = -99999;    // last known X and Y of the mouse

glm::vec3 camPos;            				// camera POSITION in cartesian coordinates
GLdouble cameraTheta, cameraPhi;            // camera DIRECTION in spherical coordinates
glm::vec3 camDir; 			                // camera DIRECTION in cartesian coordinates

struct BuildingData {                       // keeps track of an individual building's attributes
    glm::mat4 modelMatrix;                      // its position and size
    glm::vec3 color;                            // its color
};
std::vector<BuildingData> buildings;        // stores all of our building information

// values to track our grid properties
const glm::vec3 WHITE_COLOR( 1.0f, 1.0f, 1.0f );
GLuint gridVAO = 0;
GLuint numGridPoints = 0;

//because the direction vector is unit length, and we probably don't want
//to move one full unit every time a button is pressed, just create a constant
//to keep track of how far we want to move at each step. you could make
//this change w.r.t. the amount of time the button's held down for
//simple scale-sensitive movement!
const GLfloat MOVEMENT_CONSTANT = 1.5f;

bool mackHack = false;

// END GLOBAL VARIABLES
//********************************************************************************

//********************************************************************************
//
// Helper Functions

// getRand() ///////////////////////////////////////////////////////////////////
//
//  Simple helper function to return a random number between 0.0f and 1.0f.
//
////////////////////////////////////////////////////////////////////////////////
GLdouble getRand() { return rand() / (GLdouble)RAND_MAX; }

// recomputeOrientation() //////////////////////////////////////////////////////
//
// This function updates the camera's position in cartesian coordinates based
//  on its position in spherical coordinates. Should be called every time
//  cameraTheta, cameraPhi, or cameraRadius is updated.
//
////////////////////////////////////////////////////////////////////////////////
void recomputeOrientation() {
    // TODO #2 convert our theta and phi spherical angles to a cartesian vector
    GLfloat magnitude = sqrt(pow(sin(cameraTheta)*sin(cameraPhi), 2.0) + pow(-1*cos(cameraPhi), 2.0) + pow(-1*cos(cameraTheta)*sin(cameraPhi), 2.0));
    camDir = glm::vec3((sin(cameraTheta)*sin(cameraPhi))/magnitude, (-1*cos(cameraPhi))/magnitude, (-1*cos(cameraTheta)*sin(cameraPhi))/magnitude);
}

//*************************************************************************************
//
// Event Callbacks

//
//	void error_callback( int error, const char* description )
//
//		We will register this function as GLFW's error callback.
//	When an error within GLFW occurs, GLFW will tell us by calling
//	this function.  We can then print this info to the terminal to
//	alert the user.
//
static void error_callback( int error, const char* description ) {
	fprintf( stderr, "[ERROR]: %d\n\t%s\n", error, description );
}

static void keyboard_callback( GLFWwindow *window, int key, int scancode, int action, int mods ) {
	if( action == GLFW_PRESS ) {
		switch( key ) {
			case GLFW_KEY_ESCAPE:
			case GLFW_KEY_Q:
				glfwSetWindowShouldClose(window, GLFW_TRUE);
				break;
		    default: break; // to remove CLion warning
		}
	} else if (action == GLFW_REPEAT) {
	    switch(key) {
            case GLFW_KEY_W:
                camPos += camDir;
                break;
            case GLFW_KEY_S:
                camPos -= camDir;
                break;
            default: break; // to remove CLion warning
	    }
	}
}

static void cursor_callback( GLFWwindow *window, double x, double y ) {
    // if mouse hasn't moved in the window, prevent camera from flipping out
    if(mouseX == -99999) {
        mouseX = x;
        mouseY = y;
    }
    if(leftMouseButton == GLFW_PRESS) {
        // TODO #4 update camera theta and phi
        cameraTheta += (x - mouseX) * 0.005;
        if (cameraPhi + (mouseY - y) * 0.005 < M_PI && cameraPhi + (mouseY - y) * 0.005 > 0) {
            cameraPhi += (mouseY - y) * 0.005;
        }

        recomputeOrientation();     //update camera (x,y,z) based on (radius,theta,phi)
    }


    mouseX = x;
    mouseY = y;
}

static void mouse_button_callback( GLFWwindow *window, int button, int action, int mods ) {
	if( button == GLFW_MOUSE_BUTTON_LEFT ) {
		leftMouseButton = action;
	}
}

// generateEnvironment() ///////////////////////////////////////////////////////
//
//  This function creates our world which will consist of a grid in the XZ-Plane
//      and randomly placed and sized buildings of varying colors.
//
////////////////////////////////////////////////////////////////////////////////
void generateEnvironment() {
    // parameters to size our world
    const GLint GRID_WIDTH = 100;
    const GLint GRID_LENGTH = 100;
    const GLfloat GRID_SPACING_WIDTH = 1.0f;
    const GLfloat GRID_SPACING_LENGTH = 1.0f;
    const GLfloat LEFT_END_POINT = -GRID_WIDTH / 2.0f - 5;
    const GLfloat RIGHT_END_POINT = GRID_WIDTH / 2.0f + 5;
    const GLfloat BOTTOM_END_POINT = -GRID_LENGTH / 2.0f - 5;
    const GLfloat TOP_END_POINT = GRID_LENGTH / 2.0f + 5;

    // TODO #1 create the city
    for (int i = LEFT_END_POINT; i <= RIGHT_END_POINT; ++i) {
        for (int j = BOTTOM_END_POINT; j <= TOP_END_POINT; ++j) {
            if (i % 2 == 0 && j % 2 == 0 && getRand() < 0.4) {
                //make a building translation
                glm::mat4 transMatx = glm::translate(glm::mat4(1.0), glm::vec3(i, 0, j));
                float sizeScale = rand() % 10 + 1.0;
                glm::mat4 scaleMatx = glm::scale(glm::mat4(1.0), glm::vec3(1, sizeScale, 1));
                glm::mat4 heightTrans = glm::translate(glm::mat4(1.0), glm::vec3(0, 0.5 * sizeScale, 0));
                glm::vec3 colorMatx = glm::vec3(getRand(), getRand(), getRand());
                BuildingData temp;
                temp.modelMatrix = heightTrans * scaleMatx * transMatx;
                temp.color = colorMatx;
                buildings.emplace_back(temp);
            }
        }
    }

    // create the grid - do not edit this code
    std::vector< glm::vec3 > points;
    for(GLfloat i = LEFT_END_POINT; i <= RIGHT_END_POINT; i += GRID_SPACING_WIDTH) {
        points.emplace_back( glm::vec3(i, 0, BOTTOM_END_POINT) );
        points.emplace_back( glm::vec3(i, 0, TOP_END_POINT) );
    }
    for(GLfloat j = BOTTOM_END_POINT; j <= TOP_END_POINT; j += GRID_SPACING_LENGTH) {
        points.emplace_back( glm::vec3(LEFT_END_POINT, 0, j) );
        points.emplace_back( glm::vec3(RIGHT_END_POINT, 0, j) );
    }
    numGridPoints = points.size();
    gridVAO = CSCI441::SimpleShader3::registerVertexArray(points, std::vector<glm::vec3>(numGridPoints));
}

// renderScene() ///////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
void renderScene() {
    // LOOK HERE #1 draw all the buildings
    for( BuildingData currentBuilding : buildings ) {
        CSCI441::SimpleShader3::pushTransformation( currentBuilding.modelMatrix );
            CSCI441::SimpleShader3::setMaterialColor( currentBuilding.color );
            CSCI441::drawSolidCube(1.0);
        CSCI441::SimpleShader3::popTransformation();
    }


    // draw our grid
    CSCI441::SimpleShader3::disableLighting();
        CSCI441::SimpleShader3::setMaterialColor( WHITE_COLOR );
        CSCI441::SimpleShader3::draw(GL_LINES, gridVAO, numGridPoints);
    CSCI441::SimpleShader3::enableLighting();
}


//*************************************************************************************
//
// Setup Functions

//
//  void setupGLFW()
//
//      Used to setup everything GLFW related.  This includes the OpenGL context
//	and our window.
//
GLFWwindow* setupGLFW() {
	// set what function to use when registering errors
	// this is the ONLY GLFW function that can be called BEFORE GLFW is initialized
	// all other GLFW calls must be performed after GLFW has been initialized
	glfwSetErrorCallback( error_callback );

	// initialize GLFW
	if( !glfwInit() ) {
		fprintf( stderr, "[ERROR]: Could not initialize GLFW\n" );
		exit( EXIT_FAILURE );
	} else {
		fprintf( stdout, "[INFO]: GLFW initialized\n" );
	}

    glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );  // request forward compatible OpenGL context
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );        // request OpenGL Core Profile context
    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );	// request OpenGL v4.X
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 1 );	// request OpenGL vX.1
    glfwWindowHint( GLFW_RESIZABLE, GLFW_FALSE );		    // do not allow our window to be able to be resized
    glfwWindowHint( GLFW_DOUBLEBUFFER, GLFW_TRUE );         // request double buffering

	// create a window for a given size, with a given title
	GLFWwindow *window = glfwCreateWindow( WINDOW_WIDTH, WINDOW_HEIGHT, "Lab02: Flight Simulator v0.41", nullptr, nullptr );
	if( !window ) {						// if the window could not be created, NULL is returned
		fprintf( stderr, "[ERROR]: GLFW Window could not be created\n" );
		glfwTerminate();
		exit( EXIT_FAILURE );
	} else {
		fprintf( stdout, "[INFO]: GLFW Window created\n" );
	}

	glfwMakeContextCurrent(window);		                    // make the created window the current window
	glfwSwapInterval(1);				     	    // update our screen after at least 1 screen refresh

	glfwSetKeyCallback( window, keyboard_callback );		// set our keyboard callback function
	glfwSetCursorPosCallback( window, cursor_callback );	// set our cursor position callback function
	glfwSetMouseButtonCallback( window, mouse_button_callback );	// set our mouse button callback function

	return window;						                     // return the window that was created
}

//
//  void setupOpenGL()
//
//      Used to setup everything OpenGL related.  For now, the only setting
//	we need is what color to make the background of our window when we clear
//	the window.  In the future we will be adding many more settings to this
//	function.
//
void setupOpenGL() {
    // initialize GLEW
    glewExperimental = GL_TRUE;
    GLenum glewResult = glewInit();

    // check for an error
    if( glewResult != GLEW_OK ) {
        printf( "[ERROR]: Error initializing GLEW\n");
        exit(EXIT_FAILURE);
    }

    // tell OpenGL to perform depth testing with the Z-Buffer to perform hidden
    //		surface removal.  We will discuss this more very soon.
    glEnable( GL_DEPTH_TEST );

    glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );	// set the clear color to black
}

void setupScene() {
	// give the camera a scenic starting point.
	camPos.x = 60;
	camPos.y = 40;
	camPos.z = 30;
	cameraTheta = -M_PI / 3.0f;
	cameraPhi = M_PI / 2.8f;
	recomputeOrientation();

	srand( time(nullptr) );	// seed our random number generator
	generateEnvironment();

    //******************************************************************
    // this is some code to enable a default light for the scene;
    // feel free to play around with this, but we won't talk about
    // lighting in OpenGL for another couple of weeks yet.
    glm::vec3 lightPosition(10.0f, 10.0f, 10.0f);
    CSCI441::SimpleShader3::setLightPosition( lightPosition );

    glm::vec3 lightColor(1.0, 1.0, 1.0);
    CSCI441::SimpleShader3::setLightColor( lightColor );
    //******************************************************************
}

///*************************************************************************************
//
// Our main function

//
//	int main()
//
int main() {
	// GLFW sets up our OpenGL context so must be done first
	GLFWwindow *window = setupGLFW();	                // initialize all of the GLFW specific information related to OpenGL and our window
	setupOpenGL();										// initialize all of the OpenGL specific information
    CSCI441::OpenGLUtils::printOpenGLInfo();
    CSCI441::SimpleShader3::enableSmoothShading();
    CSCI441::SimpleShader3::setupSimpleShader();
    setupScene();

    printf("Controls:\n");
    printf("\tW / S - Move forwards / backwards\n");
    printf("\tMouse Drag - Pan camera\n");
    printf("\tQ / ESC - Quit program\n");

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

		// update the projection matrix based on the window size
		// the GL_PROJECTION matrix governs properties of the view coordinates;
		// i.e. what gets seen - use a perspective projection that ranges
		// with a FOV of 45 degrees, for our current aspect ratio, and Z ranges from [0.001, 1000].
		glm::mat4 projMtx = glm::perspective( 45.0f, (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.001f, 1000.0f );
        CSCI441::SimpleShader3::setProjectionMatrix(projMtx);

        // TODO #3 set up our look at matrix to position our camera
		glm::mat4 viewMtx = glm::lookAt( camPos, camDir + camPos,glm::vec3(  0,  1,  0 ) );
		// multiply by the look at matrix - this is the same as our view matrix
        CSCI441::SimpleShader3::setViewMatrix(viewMtx);

		renderScene();					                // draw everything to the window

		glfwSwapBuffers(window);                        // flush the OpenGL commands and make sure they get rendered!
		glfwPollEvents();				                // check for any events and signal to redraw screen

        // the following code is a hack for OSX Mojave
        // the window is initially black until it is moved
        // so instead of having the user manually move the window,
        // we'll automatically move it and then move it back
        if( !mackHack ) {
            GLint xPos, yPos;
            glfwGetWindowPos(window, &xPos, &yPos);
            glfwSetWindowPos(window, xPos+10, yPos+10);
            glfwSetWindowPos(window, xPos, yPos);
            mackHack = true;
        }
	}

	glfwDestroyWindow( window );// clean up and close our window
	glfwTerminate();						// shut down GLFW to clean up our context

	return EXIT_SUCCESS;				// exit our program successfully!
}
