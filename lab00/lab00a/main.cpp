/*
 *  CSCI 441, Computer Graphics, Fall 2020
 *
 *  Project: lab00a
 *  File: main.cpp
 *
 *	Author: Dr. Jeffrey Paone - Fall 2020
 *
 *  Description:
 *      Contains the code for a simple 2D OpenGL / GLFW example.
 *
 */

// include the OpenGL library header
#include <GL/glew.h>
#include <GLFW/glfw3.h>			// include GLFW framework header


// include GLM libraries and matrix functions
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <cstdio>				// for printf functionality

// include our class libraries
#include <CSCI441/OpenGLUtils.hpp>
#include <CSCI441/SimpleShader.hpp>

//*************************************************************************************
//
// Global Parameters

// global variables to keep track of window width and height.
// set to initial values for convenience, but we need variables
// for later on in case the window gets resized.
int windowWidth = 512, windowHeight = 512;

// global variables to keep track of our Triforce Vertex Data
GLuint triforceVAO = 0, numTriforcePoints = 0;
GLuint triforceVAO2 = 0, triforceVAO3 = 0;
// TODO 2B: Create the global variables to track our single triangle
GLuint transformTriforce = 0, transformPointNumber = 0;
//  VAO handle and number of vertices

bool mackHack = false;

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

	// create a window for a given size, with a given title
	GLFWwindow *window = glfwCreateWindow( windowWidth, windowHeight, "Lab00A", nullptr, nullptr );
	if( !window ) {						// if the window could not be created, NULL is returned
		fprintf( stderr, "[ERROR]: GLFW Window could not be created\n" );
		glfwTerminate();
		exit( EXIT_FAILURE );
	} else {
		fprintf( stdout, "[INFO]: GLFW Window created\n" );
	}

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
    std::vector<glm::vec2> triforcePoints;  // will hold the vertex location data
    std::vector<glm::vec3> triforceColors;  // will hold the vertex color data
    std::vector<glm::vec2> triforcePoints2;  // will hold the vertex location data
    std::vector<glm::vec3> triforceColors2;  // will hold the vertex color data
    std::vector<glm::vec2> triforcePoints3;  // will hold the vertex location data
    std::vector<glm::vec3> triforceColors3;  // will hold the vertex color data

    // TODO 1A: Create the vertex data vector for points and colors
    triforcePoints = {
            glm::vec2(100,100),
            glm::vec2(200,100),
            glm::vec2(150,180)};
    triforceColors = {
            glm::vec3(0.9, 0.8, 0.1),
            glm::vec3(0.9, 0.8, 0.1),
            glm::vec3(0.9, 0.8, 0.1)};
    triforcePoints2 = {
            glm::vec2(200,100),
            glm::vec2(300,100),
            glm::vec2(250,180)};
    triforceColors2 = {
            glm::vec3(0.9, 0.8, 0.1),
            glm::vec3(0.9, 0.8, 0.1),
            glm::vec3(0.9, 0.8, 0.1)};
    triforcePoints3 = {
            glm::vec2(150,180),
            glm::vec2(250,180),
            glm::vec2(200,260)};
    triforceColors3 = {
            glm::vec3(0.9, 0.8, 0.1),
            glm::vec3(0.9, 0.8, 0.1),
            glm::vec3(0.9, 0.8, 0.1)};

    // TODO 1B: Register the vertex data storing the VAO Handle returned
    //  Also be sure to set the number of vertices that were registered
    triforceVAO = CSCI441::SimpleShader2::registerVertexArray(triforcePoints, triforceColors);
    triforceVAO2 = CSCI441::SimpleShader2::registerVertexArray(triforcePoints2, triforceColors2);
    triforceVAO3 = CSCI441::SimpleShader2::registerVertexArray(triforcePoints3, triforceColors3);
    numTriforcePoints = 3;

    // TODO 2A: Create the vertex data array for a single triangle
    glm::vec2 transformPoints[3] = {
            glm::vec2(-50, -50),
            glm::vec2(50, -50),
            glm::vec2(0, 30)};
    glm::vec3 transformColors[3] =  {
            glm::vec3(0.9, 0.8, 0.1),
            glm::vec3(0.9, 0.8, 0.1),
            glm::vec3(0.9, 0.8, 0.1)};

    // TODO 2C: Register the vertex data array storing the VAO Handle returned
    //  Also be sure to set the number of vertices that were registered
    transformPointNumber = 3;
    transformTriforce = CSCI441::SimpleShader2::registerVertexArray(transformPointNumber, transformPoints, transformColors);
}

//*************************************************************************************
//
// Rendering / Drawing Functions - this is where the magic happens!

// TODO 2D: Create a function to handle drawing a single triangle.  We can use this
//  function to draw a standard triangle and by callling it multiple times have a
//  new triangle appear in different places.
void drawMyTriangle() {
    CSCI441::SimpleShader2::draw(GL_TRIANGLES, transformTriforce, transformPointNumber);
}

// TODO 3: Create a function to draw a stacked pyramid that will consist of three triangles.
void drawMyPyramid() {
    glm::mat4 transMtx = glm::translate(glm::mat4(1.0f),
                                        glm::vec3(-25.0f, -25.0f, 0.0f));
    CSCI441::SimpleShader2::pushTransformation( transMtx );
    drawMyTriangle();
    CSCI441::SimpleShader2::popTransformation();

    transMtx = glm::translate(glm::mat4(1.0f),
                              glm::vec3(75.0f, -25.0f, 0.0f));
    CSCI441::SimpleShader2::pushTransformation( transMtx );
    drawMyTriangle();
    CSCI441::SimpleShader2::popTransformation();

    transMtx = glm::translate(glm::mat4(1.0f),
                              glm::vec3(25.0f, 55.0f, 0.0f));
    CSCI441::SimpleShader2::pushTransformation( transMtx );
    drawMyTriangle();
    CSCI441::SimpleShader2::popTransformation();
}
//
//	void renderScene()
//
//		This method will contain all of the objects to be drawn.
//
void renderScene() {
    // TODO 1C: Draw the triangles to the window
    CSCI441::SimpleShader2::draw(GL_TRIANGLES, triforceVAO, numTriforcePoints);
    CSCI441::SimpleShader2::draw(GL_TRIANGLES, triforceVAO2, numTriforcePoints);
    CSCI441::SimpleShader2::draw(GL_TRIANGLES, triforceVAO3, numTriforcePoints);

    // TODO 2F: Now translate the triangle!
    glm::mat4 transMtx = glm::translate(glm::mat4(1.0f),
                                        glm::vec3(300.0f, 300.0f, 0.0f));
    CSCI441::SimpleShader2::pushTransformation( transMtx );
    // TODO 2E: Draw the single triangle to the window
    drawMyPyramid();
    CSCI441::SimpleShader2::popTransformation();
}

//*************************************************************************************
//
// Our main function

//
//	int main( int argc, char *argv[] )
//
//		Really you should know what this is by now.  We will make use of the parameters later
//
int main() {
	GLFWwindow *window = setupGLFW();	            // initialize all of the GLFW specific information related to OpenGL and our window
										            // GLFW sets up our OpenGL context so must be done first
	setupOpenGL();						            // initialize all of the OpenGL specific information
	CSCI441::OpenGLUtils::printOpenGLInfo();        // print out diagnostic information about the OpenGL version
    CSCI441::SimpleShader2::setupSimpleShader();    // setup our shaders and enable vertex array attributes
    setupBuffers();                                 // register vertex data with the GPU

	//  This is our draw loop - all rendering is done here.  We use a loop to keep the window open
	//	until the user decides to close the window and quit the program.  Without a loop, the
	//	window will display once and then the program exits.
	while( !glfwWindowShouldClose(window) ) {	// check if the window was instructed to be closed
		glClear( GL_COLOR_BUFFER_BIT );	// clear the current color contents in the window

		// update the projection matrix based on the window size
		// the projection matrix governs properties of the view coordinates;
		// i.e. what gets seen - use an Orthographic projection that ranges
		// from [0, windowWidth] in X and [0, windowHeight] in Y. (0,0) is the lower left.
        glm::mat4 projMtx = glm::ortho( 0.0f, (GLfloat)windowWidth, 0.0f, (GLfloat)windowHeight );
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
            glfwSetWindowPos( window, xPos+10, yPos + 10);
            glfwSetWindowPos( window, xPos, yPos);
            mackHack = true;
        }
	}

	glfwDestroyWindow( window );		// clean up and close our window
	glfwTerminate();					// shut down GLFW to clean up our context

	return EXIT_SUCCESS;				// exit our program successfully!
}
