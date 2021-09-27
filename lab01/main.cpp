/*
 *  CSCI 441, Computer Graphics, Fall 2020
 *
 *  Project: lab01
 *  File: main.cpp
 *
 *	Author: Dr. Jeffrey Paone - Fall 2020
 *
 *  Description:
 *      Contains the code for a simple interactive and animated example
 *
 */

 // include the OpenGL library header
#include <GL/glew.h>
#include <GLFW/glfw3.h>			// include GLFW framework header

// include GLM libraries and matrix functions
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cstdio>				// for printf functionality
#include <cstdlib>			    // for exit functionality

#include <CSCI441/OpenGLUtils.hpp>
#include <CSCI441/SimpleShader.hpp>

 //*************************************************************************************
 //
 // Global Parameters

 // global variables to keep track of window width and height.
 // set to initial values for convenience, but we need variables
 // for later on in case the window gets resized.
const GLint WINDOW_WIDTH = 512, WINDOW_HEIGHT = 512;

GLuint triangleVAO = 0;
std::vector<glm::vec2> trianglePoints;
std::vector<glm::vec3> triangleColorsGold;
std::vector<glm::vec3> triangleColorsRed;
GLfloat triforceAngle = 0.05;
GLboolean EVIL_TRIFORCE = false;
GLuint triforceXPosition = 100;
GLuint triforceYPosition = 100;

bool mackHack = false;

//*************************************************************************************
//
// Function Prototypes

// function to change the colors of the triangle
void updateTriangleColors();

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

// TODO #1  create the keyboard callback function

void keyboard_callback( GLFWwindow *win, int key, int scancode, int action, int mods ) {
    if (key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(win, GLFW_TRUE);
    } else if (key == GLFW_KEY_C && action == GLFW_PRESS) {
        EVIL_TRIFORCE = !EVIL_TRIFORCE;
        updateTriangleColors();
    }
}

void mouse_button_callback( GLFWwindow *window, int button, int action, int mods ) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            EVIL_TRIFORCE = true;
        } else if (action == GLFW_RELEASE) {
            EVIL_TRIFORCE = false;
        }
    }
    updateTriangleColors();
}

void cursor_callback( GLFWwindow *window, double x, double y ) {
    triforceXPosition = x;
    triforceYPosition = WINDOW_HEIGHT - y;
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

    glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );              // request forward compatible OpenGL context
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );    // request OpenGL Core Profile context
    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );	            // request OpenGL v4.X
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 1 );	            // request OpenGL vX.1
    glfwWindowHint( GLFW_RESIZABLE, GLFW_FALSE );		                // do not allow our window to be able to be resized
    glfwWindowHint( GLFW_DOUBLEBUFFER, GLFW_TRUE );


    // create a window for a given size, with a given title
	GLFWwindow *window = glfwCreateWindow( WINDOW_WIDTH, WINDOW_HEIGHT, "Lab01", nullptr, nullptr );
	if( !window ) {						// if the window could not be created, NULL is returned
		fprintf( stderr, "[ERROR]: GLFW Window could not be created\n" );
		glfwTerminate();
		exit( EXIT_FAILURE );
	} else {
		fprintf( stdout, "[INFO]: GLFW Window created\n" );
	}

    // TODO #2 register our callback functions here
    glfwSetKeyCallback( window, keyboard_callback);
    glfwSetMouseButtonCallback( window, mouse_button_callback );
    glfwSetCursorPosCallback( window, cursor_callback );

	glfwMakeContextCurrent(window);		// make the created window the current window
	glfwSwapInterval(1);				// update our screen after at least 1 screen refresh

	return window;						// return the window that was created
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

	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );	// set the clear color to black
}

void setupBuffers() {
    // create a single triangle
    trianglePoints.emplace_back( glm::vec2( -2.5, -2.0) );
    triangleColorsGold.emplace_back( glm::vec3( 0.9, 0.8, 0.1) );
    triangleColorsRed.emplace_back( glm::vec3(1.0, 0.0, 0.0) );

    trianglePoints.emplace_back( glm::vec2(  2.5, -2.0) );
    triangleColorsGold.emplace_back( glm::vec3( 0.9, 0.8, 0.1) );
    triangleColorsRed.emplace_back( glm::vec3(1.0, 0.0, 0.0) );

    trianglePoints.emplace_back( glm::vec2(  0.0,  2.0) );
    triangleColorsGold.emplace_back( glm::vec3( 0.9, 0.8, 0.1) );
    triangleColorsRed.emplace_back( glm::vec3(1.0, 0.0, 0.0) );

    triangleVAO = CSCI441::SimpleShader2::registerVertexArray( trianglePoints, triangleColorsGold);
}

void updateTriangleColors() {
    // TODO #4 send the GPU the new color information to draw our triangle with
    if (!EVIL_TRIFORCE) {
        CSCI441::SimpleShader2::updateVertexArray(triangleVAO, trianglePoints, triangleColorsGold);
    } else {
        CSCI441::SimpleShader2::updateVertexArray(triangleVAO, trianglePoints, triangleColorsRed);
    }
}

//*************************************************************************************
//
// Rendering / Drawing Functions - this is where the magic happens!

//
//  void drawTriangle()
//
//		Issues a series of OpenGL commands to draw a triangle
//
void drawTriangle() {
    CSCI441::SimpleShader2::draw(GL_TRIANGLES, triangleVAO, trianglePoints.size());
}

//
//  void drawTriforce()
//
//      Issues a series of OpenGL commands to draw three triangles in a
//  triangle shape that all fit inside a larger triangle.
//
void drawTriforce() {
    glm::mat4 t1 = glm::translate( glm::mat4(1.0), glm::vec3( -2.5, -2, 0 ) );
    CSCI441::SimpleShader2::pushTransformation( t1 );
        drawTriangle();
    CSCI441::SimpleShader2::popTransformation();

    glm::mat4 t2 = glm::translate( glm::mat4(1.0), glm::vec3( 2.5, -2, 0 ) );
    CSCI441::SimpleShader2::pushTransformation( t2 );
        drawTriangle();
    CSCI441::SimpleShader2::popTransformation();

    glm::mat4 t3 = glm::translate( glm::mat4(1.0), glm::vec3( 0, 2, 0 ) );
    CSCI441::SimpleShader2::pushTransformation( t3 );
        drawTriangle();
    CSCI441::SimpleShader2::popTransformation();
}

//
//  void renderScene()
//
//      We will register this function as GLUT's display callback.
//  This is where the magic happens - all rendering is done here.
//
void renderScene() {
//    glm::mat4 transMtx = glm::translate( glm::mat4(1.0), glm::vec3( triforceXPosition, triforceYPosition, 0 ) );
    glm::mat4 transMtx = glm::translate( glm::mat4(1.0), glm::vec3( 200, 200, 0 ) );
    CSCI441::SimpleShader2::pushTransformation( transMtx );
        glm::mat4 rotMtx = glm::rotate( glm::mat4(1.0), triforceAngle, glm::vec3(0, 0, 1) );
        CSCI441::SimpleShader2::pushTransformation( rotMtx );
            glm::mat4 scaleMtx = glm::scale( glm::mat4(1.0), glm::vec3(10, 10, 1) );
            CSCI441::SimpleShader2::pushTransformation( scaleMtx );
                drawTriforce();
            CSCI441::SimpleShader2::popTransformation();
        CSCI441::SimpleShader2::popTransformation();
    CSCI441::SimpleShader2::popTransformation();
}

///*************************************************************************************
//
// Our main function

//
//	int main( int argc, char *argv[] )
//
//		Really you should know what this is by now.  We will make use of the parameters later
//
int main() {
	GLFWwindow *window = setupGLFW();	// initialize all of the GLFW specific information related to OpenGL and our window
										// GLFW sets up our OpenGL context so must be done first
	setupOpenGL();						// initialize all of the OpenGL specific information
    CSCI441::OpenGLUtils::printOpenGLInfo();
    CSCI441::SimpleShader2::setupSimpleShader();
    setupBuffers();

	//  This is our draw loop - all rendering is done here.  We use a loop to keep the window open
	//	until the user decides to close the window and quit the program.  Without a loop, the
	//	window will display once and then the program exits.
	while( !glfwWindowShouldClose(window) ) {	// check if the window was instructed to be closed
        // TODO #3 draw to the back buffer and clear it!
        triforceAngle += 0.03;
        glDrawBuffer(GL_BACK);
        glClear(GL_COLOR_BUFFER_BIT);
		// update the projection matrix based on the window size
		// the GL_PROJECTION matrix governs properties of the view coordinates;
		// i.e. what gets seen - use an Orthographic projection that ranges
		// from [0, WINDOW_WIDTH] in X and [0, WINDOW_HEIGHT] in Y. (0,0) is the lower left.
		glm::mat4 projMtx = glm::ortho( 0.0f, (GLfloat)WINDOW_WIDTH, 0.0f, (GLfloat)WINDOW_HEIGHT );
        CSCI441::SimpleShader2::setProjectionMatrix(projMtx);

		// Get the size of our framebuffer.  Ideally this should be the same dimensions as our window, but
		// when using a Retina display the actual window can be larger than the requested window.  Therefore
		// query what the actual size of the window we are rendering to is.
		GLint framebufferWidth, framebufferHeight;
		glfwGetFramebufferSize( window, &framebufferWidth, &framebufferHeight );

		// update the viewport - tell OpenGL we want to render to the whole window
		glViewport( 0, 0, framebufferWidth, framebufferHeight );

		renderScene();					// draw everything to the window

		glfwSwapBuffers(window);		// flush the OpenGL commands and make sure they get rendered!
		glfwPollEvents();				// check for any events and signal to redraw screen

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

	glfwDestroyWindow( window );		// clean up and close our window
	glfwTerminate();					// shut down GLFW to clean up our context

	return EXIT_SUCCESS;				// exit our program successfully!
}
