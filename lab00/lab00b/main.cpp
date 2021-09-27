/*
 *  CSCI 441, Computer Graphics, Fall 2020
 *
 *  Project: lab00b
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

GLuint mountains = 0, mountainPoints = 0, river = 0, riverPoints = 0,
ground = 0, groundPoints = 0, sky = 0, skyPoints = 0;

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
	GLFWwindow *window = glfwCreateWindow( windowWidth, windowHeight, "Lab00B", nullptr, nullptr );
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
    // TODO 1 specify data on the CPU
    std::vector<glm::vec2> mountainVertexes;  // will hold the vertex location data
    std::vector<glm::vec3> mountainColors;  // will hold the vertex color data
    std::vector<glm::vec2> riverVertexes;  // will hold the vertex location data
    std::vector<glm::vec3> riverColors;  // will hold the vertex color data
    std::vector<glm::vec2> groundVertexes;  // will hold the vertex location data
    std::vector<glm::vec3> groundColors;  // will hold the vertex color data
    std::vector<glm::vec2> skyVertexes;  // will hold the vertex location data
    std::vector<glm::vec3> skyColors;  // will hold the vertex color data

    mountainVertexes = {
            glm::vec2(0,0),
            glm::vec2(100,0),
            glm::vec2(50,80)};
    mountainColors = {
            glm::vec3(0.69, 0.69, 0.69),
            glm::vec3(0.69, 0.69, 0.69),
            glm::vec3(1, 1, 1)};
    riverVertexes = {
            glm::vec2(0,220),
            glm::vec2(512,220),
            glm::vec2(512,260),
            glm::vec2(0,260)};
    riverColors = {
            glm::vec3(0.19, 0.69, 0.75),
            glm::vec3(0.19, 0.69, 0.75),
            glm::vec3(0.19, 0.69, 0.75),
            glm::vec3(0.19, 0.69, 0.75)};
    groundVertexes = {
            glm::vec2(0,0),
            glm::vec2(512,0),
            glm::vec2(512,300),
            glm::vec2(0,300)};
    groundColors = {
            glm::vec3(0.0, 0.78, 0.01),
            glm::vec3(0.0, 0.78, 0.01),
            glm::vec3(0.0, 0.53, 0.01),
            glm::vec3(0.0, 0.53, 0.01)};
    skyVertexes = {
            glm::vec2(0,300),
            glm::vec2(512,300),
            glm::vec2(512,512),
            glm::vec2(0,512)};
    skyColors = {
            glm::vec3(0.28, 0.65, 0.96),
            glm::vec3(0.28, 0.65, 0.96),
            glm::vec3(0.23, 0.52, 0.76),
            glm::vec3(0.23, 0.52, 0.76),};
    // TODO 2 transfer data to the GPU
    mountains = CSCI441::SimpleShader2::registerVertexArray(mountainVertexes, mountainColors);
    mountainPoints = 3;
    river = CSCI441::SimpleShader2::registerVertexArray(riverVertexes, riverColors);
    riverPoints = 4;
    ground = CSCI441::SimpleShader2::registerVertexArray(groundVertexes, groundColors);
    groundPoints = 4;
    sky = CSCI441::SimpleShader2::registerVertexArray(skyVertexes, skyColors);
    skyPoints = 4;
}

//*************************************************************************************
//
// Rendering / Drawing Functions - this is where the magic happens!

void drawMountains() {
    glm::mat4 scaleMtx = glm::scale(glm::mat4(1.0f), glm::vec3(2.5f, 2.5f, 1.0f));
    glm::mat4 transMtx = glm::translate(glm::mat4(1.0f),
                              glm::vec3(50.0f, 120.0f, 0.0f));
    CSCI441::SimpleShader2::pushTransformation(scaleMtx);
    CSCI441::SimpleShader2::pushTransformation( transMtx );
    CSCI441::SimpleShader2::draw(GL_TRIANGLES, mountains, mountainPoints);
    CSCI441::SimpleShader2::popTransformation();
    CSCI441::SimpleShader2::popTransformation();

    scaleMtx = glm::scale(glm::mat4(1.0f), glm::vec3(2.0f, 2.0f, 1.0f));
    transMtx = glm::translate(glm::mat4(1.0f),
                                        glm::vec3(0.0f, 150.0f, 0.0f));
    CSCI441::SimpleShader2::pushTransformation(scaleMtx);
    CSCI441::SimpleShader2::pushTransformation( transMtx );
    CSCI441::SimpleShader2::draw(GL_TRIANGLES, mountains, mountainPoints);
    CSCI441::SimpleShader2::popTransformation();
    CSCI441::SimpleShader2::popTransformation();

    scaleMtx = glm::scale(glm::mat4(1.0f), glm::vec3(2.0f, 2.0f, 1.0f));
    transMtx = glm::translate(glm::mat4(1.0f),
                                        glm::vec3(156.0f, 150.0f, 0.0f));
    CSCI441::SimpleShader2::pushTransformation(scaleMtx);
    CSCI441::SimpleShader2::pushTransformation( transMtx );
    CSCI441::SimpleShader2::draw(GL_TRIANGLES, mountains, mountainPoints);
    CSCI441::SimpleShader2::popTransformation();
    CSCI441::SimpleShader2::popTransformation();
}
//
//	void renderScene()
//
//		This method will contain all of the objects to be drawn.
//
void renderScene() {

    // TODO 3 tell GPU to draw data
    CSCI441::SimpleShader2::draw(GL_TRIANGLE_FAN, ground, groundPoints);
    CSCI441::SimpleShader2::draw(GL_TRIANGLE_FAN, sky, skyPoints);
    drawMountains();
    CSCI441::SimpleShader2::draw(GL_TRIANGLE_FAN, river, riverPoints);

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
