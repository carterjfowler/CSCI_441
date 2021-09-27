/*
 *  CSCI 441, Computer Graphics, Fall 2020
 *
 *  Project: A1
 *  File: main.cpp
 *
 *	Author: Carter Fowler
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
const GLint WINDOW_WIDTH = 700, WINDOW_HEIGHT = 150;

GLuint letterA_VAO = 0, letterE_VAO = 0, letterT_VAO = 0, letterH_VAO = 0,
letterO_VAO = 0, letterN_VAO = 0, letterM_VAO = 0, letterI_VAO = 0, letterS_VAO = 0,
otherLetterT_VAO = 0, letterY_VAO = 0, hammerVAO = 0, swordVAO = 0, swordTipVAO = 0,
handleVAO = 0;
std::vector<glm::vec2> rectangle;
std::vector<glm::vec2> triangle;
std::vector<glm::vec3> rectangleColorGold;
std::vector<glm::vec3> rectangleColorBlue;
std::vector<glm::vec3> rectangleColorSilver;
std::vector<glm::vec3> triangleColorSilver;
std::vector<glm::vec3> rectangleColorBrown;
//used for all rotations, rewritten before each rotation so this initial number doesn't matter
GLfloat angle = 6;
//Used to keep everything in line if the entire image ever needs to shift
GLuint x_Translation = 30;
GLuint y_Translation = 50;

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
    }
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

    //Define a basic rectangle and triangle, to be used after scaling,
    //transforming, and rotating to draw the letters and crest
    //Only one to save space and to make the code easier to understand
    rectangle = {
            glm::vec2(0,0),
            glm::vec2(1,0),
            glm::vec2(1,1),
            glm::vec2(0,1)};
    triangle = {
            glm::vec2(0,0),
            glm::vec2(0.5,0.866),
            glm::vec2(1,0)};
    //define the color arrays for each shape and color combo necessary
    rectangleColorBlue = {
            glm::vec3(0.23, 0.52, 0.76),
            glm::vec3(0.23, 0.52, 0.76),
            glm::vec3(0.23, 0.52, 0.76),
            glm::vec3(0.23, 0.52, 0.76)};
    rectangleColorGold = {
            glm::vec3( 0.9, 0.8, 0.1),
            glm::vec3( 0.9, 0.8, 0.1),
            glm::vec3( 0.9, 0.8, 0.1),
            glm::vec3( 0.9, 0.8, 0.1)};
    rectangleColorSilver = {
            glm::vec3( 0.79, 0.79, 0.79),
            glm::vec3( 0.79, 0.79, 0.79),
            glm::vec3( 0.79, 0.79, 0.79),
            glm::vec3( 0.79, 0.79, 0.79)};
    triangleColorSilver = {
            glm::vec3( 0.79, 0.79, 0.79),
            glm::vec3( 0.79, 0.79, 0.79),
            glm::vec3( 0.79, 0.79, 0.79)};
    rectangleColorBrown = {
            glm::vec3( 0.43, 0.22, 0.06),
            glm::vec3( 0.43, 0.22, 0.06),
            glm::vec3( 0.43, 0.22, 0.06),
            glm::vec3( 0.43, 0.22, 0.06)};

    //used to link the necessary shape and color arrays, these are what will be scaled,
    //translated, and rotated later when drawing
    letterA_VAO = CSCI441::SimpleShader2::registerVertexArray( rectangle, rectangleColorGold);
    letterE_VAO = CSCI441::SimpleShader2::registerVertexArray( rectangle, rectangleColorGold);
    letterT_VAO = CSCI441::SimpleShader2::registerVertexArray( rectangle, rectangleColorGold);
    letterH_VAO = CSCI441::SimpleShader2::registerVertexArray( rectangle, rectangleColorGold);
    letterO_VAO = CSCI441::SimpleShader2::registerVertexArray( rectangle, rectangleColorGold);
    letterN_VAO = CSCI441::SimpleShader2::registerVertexArray( rectangle, rectangleColorGold);

    letterM_VAO = CSCI441::SimpleShader2::registerVertexArray( rectangle, rectangleColorBlue);
    letterI_VAO = CSCI441::SimpleShader2::registerVertexArray( rectangle, rectangleColorBlue);
    letterS_VAO = CSCI441::SimpleShader2::registerVertexArray( rectangle, rectangleColorBlue);
    otherLetterT_VAO = CSCI441::SimpleShader2::registerVertexArray( rectangle, rectangleColorBlue);
    letterY_VAO = CSCI441::SimpleShader2::registerVertexArray( rectangle, rectangleColorBlue);

    hammerVAO = CSCI441::SimpleShader2::registerVertexArray( rectangle, rectangleColorSilver);
    swordVAO = CSCI441::SimpleShader2::registerVertexArray( rectangle, rectangleColorSilver);
    swordTipVAO = CSCI441::SimpleShader2::registerVertexArray( triangle, triangleColorSilver);
    handleVAO = CSCI441::SimpleShader2::registerVertexArray( rectangle, rectangleColorBrown);
}

//void updateTriangleColors() {
//    // TODO #4 send the GPU the new color information to draw our triangle with
//    if (!EVIL_TRIFORCE) {
//        CSCI441::SimpleShader2::updateVertexArray(triangleVAO, trianglePoints, triangleColorsGold);
//    } else {
//        CSCI441::SimpleShader2::updateVertexArray(triangleVAO, trianglePoints, triangleColorsRed);
//    }
//}

//*************************************************************************************
//
// Rendering / Drawing Functions - this is where the magic happens!


void drawLetterA() {
    angle = 6;
    glm::mat4 transMtx = glm::translate( glm::mat4(1.0), glm::vec3( x_Translation, y_Translation, 0 ) );
    CSCI441::SimpleShader2::pushTransformation( transMtx );
        glm::mat4 rotMtx = glm::rotate( glm::mat4(1.0), angle, glm::vec3(0, 0, 1) );
        CSCI441::SimpleShader2::pushTransformation( rotMtx );
            glm::mat4 scaleMtx = glm::scale( glm::mat4(1.0), glm::vec3(10, 70, 1) );
            CSCI441::SimpleShader2::pushTransformation( scaleMtx );
                CSCI441::SimpleShader2::draw(GL_TRIANGLE_FAN, letterA_VAO, rectangle.size());
            CSCI441::SimpleShader2::popTransformation();
        CSCI441::SimpleShader2::popTransformation();
    CSCI441::SimpleShader2::popTransformation();

    angle = 0.29;
    transMtx = glm::translate( glm::mat4(1.0), glm::vec3( x_Translation + 40, y_Translation - 3, 0 ) );
    CSCI441::SimpleShader2::pushTransformation( transMtx );
        rotMtx = glm::rotate( glm::mat4(1.0), angle, glm::vec3(0, 0, 1) );
        CSCI441::SimpleShader2::pushTransformation( rotMtx );
            scaleMtx = glm::scale( glm::mat4(1.0), glm::vec3(10, 70, 1) );
            CSCI441::SimpleShader2::pushTransformation( scaleMtx );
                CSCI441::SimpleShader2::draw(GL_TRIANGLE_FAN, letterA_VAO, rectangle.size());
            CSCI441::SimpleShader2::popTransformation();
        CSCI441::SimpleShader2::popTransformation();
    CSCI441::SimpleShader2::popTransformation();

    transMtx = glm::translate( glm::mat4(1.0), glm::vec3( x_Translation + 10, y_Translation + 20, 0 ) );
    CSCI441::SimpleShader2::pushTransformation( transMtx );
        scaleMtx = glm::scale( glm::mat4(1.0), glm::vec3(25, 10, 1) );
        CSCI441::SimpleShader2::pushTransformation( scaleMtx );
            CSCI441::SimpleShader2::draw(GL_TRIANGLE_FAN, letterA_VAO, rectangle.size());
        CSCI441::SimpleShader2::popTransformation();
    CSCI441::SimpleShader2::popTransformation();
}

void drawLetterE() {
    glm::mat4 transMtx = glm::translate( glm::mat4(1.0), glm::vec3( x_Translation + 60, y_Translation - 3, 0 ) );
    CSCI441::SimpleShader2::pushTransformation( transMtx );
        glm::mat4 scaleMtx = glm::scale( glm::mat4(1.0), glm::vec3(10, 55, 1) );
        CSCI441::SimpleShader2::pushTransformation( scaleMtx );
            CSCI441::SimpleShader2::draw(GL_TRIANGLE_FAN, letterE_VAO, rectangle.size());
        CSCI441::SimpleShader2::popTransformation();
    CSCI441::SimpleShader2::popTransformation();

    transMtx = glm::translate( glm::mat4(1.0), glm::vec3( x_Translation + 60, y_Translation - 3, 0 ) );
    CSCI441::SimpleShader2::pushTransformation( transMtx );
        scaleMtx = glm::scale( glm::mat4(1.0), glm::vec3(30, 10, 1) );
        CSCI441::SimpleShader2::pushTransformation( scaleMtx );
            CSCI441::SimpleShader2::draw(GL_TRIANGLE_FAN, letterE_VAO, rectangle.size());
        CSCI441::SimpleShader2::popTransformation();
    CSCI441::SimpleShader2::popTransformation();

    transMtx = glm::translate( glm::mat4(1.0), glm::vec3( x_Translation + 60, y_Translation + 20, 0 ) );
    CSCI441::SimpleShader2::pushTransformation( transMtx );
        scaleMtx = glm::scale( glm::mat4(1.0), glm::vec3(25, 10, 1) );
        CSCI441::SimpleShader2::pushTransformation( scaleMtx );
            CSCI441::SimpleShader2::draw(GL_TRIANGLE_FAN, letterE_VAO, rectangle.size());
        CSCI441::SimpleShader2::popTransformation();
    CSCI441::SimpleShader2::popTransformation();

    transMtx = glm::translate( glm::mat4(1.0), glm::vec3( x_Translation + 60, y_Translation + 42, 0 ) );
    CSCI441::SimpleShader2::pushTransformation( transMtx );
        scaleMtx = glm::scale( glm::mat4(1.0), glm::vec3(30, 10, 1) );
        CSCI441::SimpleShader2::pushTransformation( scaleMtx );
            CSCI441::SimpleShader2::draw(GL_TRIANGLE_FAN, letterE_VAO, rectangle.size());
        CSCI441::SimpleShader2::popTransformation();
    CSCI441::SimpleShader2::popTransformation();
}

void drawLetterT() {
    glm::mat4 transMtx = glm::translate( glm::mat4(1.0), glm::vec3( x_Translation + 115, y_Translation - 3, 0 ) );
    CSCI441::SimpleShader2::pushTransformation( transMtx );
        glm::mat4 scaleMtx = glm::scale( glm::mat4(1.0), glm::vec3(10, 55, 1) );
        CSCI441::SimpleShader2::pushTransformation( scaleMtx );
            CSCI441::SimpleShader2::draw(GL_TRIANGLE_FAN, letterT_VAO, rectangle.size());
        CSCI441::SimpleShader2::popTransformation();
    CSCI441::SimpleShader2::popTransformation();

    transMtx = glm::translate( glm::mat4(1.0), glm::vec3( x_Translation + 100, y_Translation + 42, 0 ) );
    CSCI441::SimpleShader2::pushTransformation( transMtx );
        scaleMtx = glm::scale( glm::mat4(1.0), glm::vec3(40, 10, 1) );
        CSCI441::SimpleShader2::pushTransformation( scaleMtx );
            CSCI441::SimpleShader2::draw(GL_TRIANGLE_FAN, letterT_VAO, rectangle.size());
        CSCI441::SimpleShader2::popTransformation();
    CSCI441::SimpleShader2::popTransformation();
}

void drawLetterH() {
    glm::mat4 transMtx = glm::translate( glm::mat4(1.0), glm::vec3( x_Translation + 150, y_Translation - 3, 0 ) );
    CSCI441::SimpleShader2::pushTransformation( transMtx );
        glm::mat4 scaleMtx = glm::scale( glm::mat4(1.0), glm::vec3(10, 55, 1) );
        CSCI441::SimpleShader2::pushTransformation( scaleMtx );
            CSCI441::SimpleShader2::draw(GL_TRIANGLE_FAN, letterH_VAO, rectangle.size());
        CSCI441::SimpleShader2::popTransformation();
    CSCI441::SimpleShader2::popTransformation();

    transMtx = glm::translate( glm::mat4(1.0), glm::vec3( x_Translation + 175, y_Translation - 3, 0 ) );
    CSCI441::SimpleShader2::pushTransformation( transMtx );
        scaleMtx = glm::scale( glm::mat4(1.0), glm::vec3(10, 55, 1) );
        CSCI441::SimpleShader2::pushTransformation( scaleMtx );
            CSCI441::SimpleShader2::draw(GL_TRIANGLE_FAN, letterH_VAO, rectangle.size());
        CSCI441::SimpleShader2::popTransformation();
    CSCI441::SimpleShader2::popTransformation();

    transMtx = glm::translate( glm::mat4(1.0), glm::vec3( x_Translation + 150, y_Translation + 20, 0 ) );
    CSCI441::SimpleShader2::pushTransformation( transMtx );
        scaleMtx = glm::scale( glm::mat4(1.0), glm::vec3(25, 10, 1) );
        CSCI441::SimpleShader2::pushTransformation( scaleMtx );
            CSCI441::SimpleShader2::draw(GL_TRIANGLE_FAN, letterH_VAO, rectangle.size());
        CSCI441::SimpleShader2::popTransformation();
    CSCI441::SimpleShader2::popTransformation();
}

void drawLetterO() {
    glm::mat4 transMtx = glm::translate( glm::mat4(1.0), glm::vec3( x_Translation + 195, y_Translation - 3, 0 ) );
    CSCI441::SimpleShader2::pushTransformation( transMtx );
        glm::mat4 scaleMtx = glm::scale( glm::mat4(1.0), glm::vec3(10, 55, 1) );
        CSCI441::SimpleShader2::pushTransformation( scaleMtx );
            CSCI441::SimpleShader2::draw(GL_TRIANGLE_FAN, letterO_VAO, rectangle.size());
        CSCI441::SimpleShader2::popTransformation();
    CSCI441::SimpleShader2::popTransformation();

    transMtx = glm::translate( glm::mat4(1.0), glm::vec3( x_Translation + 220, y_Translation - 3, 0 ) );
    CSCI441::SimpleShader2::pushTransformation( transMtx );
        scaleMtx = glm::scale( glm::mat4(1.0), glm::vec3(10, 55, 1) );
        CSCI441::SimpleShader2::pushTransformation( scaleMtx );
            CSCI441::SimpleShader2::draw(GL_TRIANGLE_FAN, letterO_VAO, rectangle.size());
        CSCI441::SimpleShader2::popTransformation();
    CSCI441::SimpleShader2::popTransformation();

    transMtx = glm::translate( glm::mat4(1.0), glm::vec3( x_Translation + 195, y_Translation + 42, 0 ) );
    CSCI441::SimpleShader2::pushTransformation( transMtx );
        scaleMtx = glm::scale( glm::mat4(1.0), glm::vec3(25, 10, 1) );
        CSCI441::SimpleShader2::pushTransformation( scaleMtx );
            CSCI441::SimpleShader2::draw(GL_TRIANGLE_FAN, letterO_VAO, rectangle.size());
        CSCI441::SimpleShader2::popTransformation();
    CSCI441::SimpleShader2::popTransformation();

    transMtx = glm::translate( glm::mat4(1.0), glm::vec3( x_Translation + 195, y_Translation - 3, 0 ) );
    CSCI441::SimpleShader2::pushTransformation( transMtx );
        scaleMtx = glm::scale( glm::mat4(1.0), glm::vec3(25, 10, 1) );
        CSCI441::SimpleShader2::pushTransformation( scaleMtx );
            CSCI441::SimpleShader2::draw(GL_TRIANGLE_FAN, letterO_VAO, rectangle.size());
        CSCI441::SimpleShader2::popTransformation();
    CSCI441::SimpleShader2::popTransformation();
}

void drawLetterN() {
    glm::mat4 transMtx = glm::translate( glm::mat4(1.0), glm::vec3( x_Translation + 240, y_Translation - 3, 0 ) );
        CSCI441::SimpleShader2::pushTransformation( transMtx );
        glm::mat4 scaleMtx = glm::scale( glm::mat4(1.0), glm::vec3(10, 55, 1) );
        CSCI441::SimpleShader2::pushTransformation( scaleMtx );
            CSCI441::SimpleShader2::draw(GL_TRIANGLE_FAN, letterN_VAO, rectangle.size());
        CSCI441::SimpleShader2::popTransformation();
    CSCI441::SimpleShader2::popTransformation();

    transMtx = glm::translate( glm::mat4(1.0), glm::vec3( x_Translation + 265, y_Translation - 3, 0 ) );
    CSCI441::SimpleShader2::pushTransformation( transMtx );
        scaleMtx = glm::scale( glm::mat4(1.0), glm::vec3(10, 55, 1) );
        CSCI441::SimpleShader2::pushTransformation( scaleMtx );
            CSCI441::SimpleShader2::draw(GL_TRIANGLE_FAN, letterN_VAO, rectangle.size());
        CSCI441::SimpleShader2::popTransformation();
    CSCI441::SimpleShader2::popTransformation();

    angle = 0.44;
    transMtx = glm::translate( glm::mat4(1.0), glm::vec3( x_Translation + 265, y_Translation - 3, 0 ) );
    CSCI441::SimpleShader2::pushTransformation( transMtx );
        glm::mat4 rotMtx = glm::rotate( glm::mat4(1.0), angle, glm::vec3(0, 0, 1) );
        CSCI441::SimpleShader2::pushTransformation( rotMtx );
            scaleMtx = glm::scale( glm::mat4(1.0), glm::vec3(10, 55, 1) );
            CSCI441::SimpleShader2::pushTransformation( scaleMtx );
                CSCI441::SimpleShader2::draw(GL_TRIANGLE_FAN, letterA_VAO, rectangle.size());
            CSCI441::SimpleShader2::popTransformation();
        CSCI441::SimpleShader2::popTransformation();
    CSCI441::SimpleShader2::popTransformation();
}

void drawLetterM() {
    glm::mat4 transMtx = glm::translate( glm::mat4(1.0), glm::vec3( x_Translation + 310, y_Translation - 3, 0 ) );
    CSCI441::SimpleShader2::pushTransformation( transMtx );
        glm::mat4 scaleMtx = glm::scale( glm::mat4(1.0), glm::vec3(10, 67, 1) );
        CSCI441::SimpleShader2::pushTransformation( scaleMtx );
            CSCI441::SimpleShader2::draw(GL_TRIANGLE_FAN, letterM_VAO, rectangle.size());
        CSCI441::SimpleShader2::popTransformation();
    CSCI441::SimpleShader2::popTransformation();

    transMtx = glm::translate( glm::mat4(1.0), glm::vec3( x_Translation + 355, y_Translation - 3, 0 ) );
    CSCI441::SimpleShader2::pushTransformation( transMtx );
        scaleMtx = glm::scale( glm::mat4(1.0), glm::vec3(10, 67, 1) );
        CSCI441::SimpleShader2::pushTransformation( scaleMtx );
            CSCI441::SimpleShader2::draw(GL_TRIANGLE_FAN, letterM_VAO, rectangle.size());
        CSCI441::SimpleShader2::popTransformation();
    CSCI441::SimpleShader2::popTransformation();

    angle = 0.6;
    transMtx = glm::translate( glm::mat4(1.0), glm::vec3( x_Translation + 335, y_Translation + 25, 0 ) );
    CSCI441::SimpleShader2::pushTransformation( transMtx );
        glm::mat4 rotMtx = glm::rotate( glm::mat4(1.0), angle, glm::vec3(0, 0, 1) );
        CSCI441::SimpleShader2::pushTransformation( rotMtx );
            scaleMtx = glm::scale( glm::mat4(1.0), glm::vec3(10, 40, 1) );
            CSCI441::SimpleShader2::pushTransformation( scaleMtx );
                CSCI441::SimpleShader2::draw(GL_TRIANGLE_FAN, letterM_VAO, rectangle.size());
            CSCI441::SimpleShader2::popTransformation();
        CSCI441::SimpleShader2::popTransformation();
    CSCI441::SimpleShader2::popTransformation();

    angle = 5.75;
    transMtx = glm::translate( glm::mat4(1.0), glm::vec3( x_Translation + 335, y_Translation + 30, 0 ) );
    CSCI441::SimpleShader2::pushTransformation( transMtx );
        rotMtx = glm::rotate( glm::mat4(1.0), angle, glm::vec3(0, 0, 1) );
        CSCI441::SimpleShader2::pushTransformation( rotMtx );
            scaleMtx = glm::scale( glm::mat4(1.0), glm::vec3(10, 40, 1) );
            CSCI441::SimpleShader2::pushTransformation( scaleMtx );
                CSCI441::SimpleShader2::draw(GL_TRIANGLE_FAN, letterM_VAO, rectangle.size());
            CSCI441::SimpleShader2::popTransformation();
        CSCI441::SimpleShader2::popTransformation();
    CSCI441::SimpleShader2::popTransformation();
}

void drawLetterI() {
    glm::mat4 transMtx = glm::translate( glm::mat4(1.0), glm::vec3( x_Translation + 375, y_Translation - 3, 0 ) );
    CSCI441::SimpleShader2::pushTransformation( transMtx );
        glm::mat4 scaleMtx = glm::scale( glm::mat4(1.0), glm::vec3(10, 55, 1) );
        CSCI441::SimpleShader2::pushTransformation( scaleMtx );
            CSCI441::SimpleShader2::draw(GL_TRIANGLE_FAN, letterI_VAO, rectangle.size());
        CSCI441::SimpleShader2::popTransformation();
    CSCI441::SimpleShader2::popTransformation();
}

void drawLetterS() {
    glm::mat4 transMtx = glm::translate( glm::mat4(1.0), glm::vec3( x_Translation + 395, y_Translation + 20, 0 ) );
    CSCI441::SimpleShader2::pushTransformation( transMtx );
        glm::mat4 scaleMtx = glm::scale( glm::mat4(1.0), glm::vec3(10, 27, 1) );
        CSCI441::SimpleShader2::pushTransformation( scaleMtx );
            CSCI441::SimpleShader2::draw(GL_TRIANGLE_FAN, letterS_VAO, rectangle.size());
        CSCI441::SimpleShader2::popTransformation();
    CSCI441::SimpleShader2::popTransformation();

    transMtx = glm::translate( glm::mat4(1.0), glm::vec3( x_Translation + 415, y_Translation - 3, 0 ) );
    CSCI441::SimpleShader2::pushTransformation( transMtx );
        scaleMtx = glm::scale( glm::mat4(1.0), glm::vec3(10, 27, 1) );
        CSCI441::SimpleShader2::pushTransformation( scaleMtx );
            CSCI441::SimpleShader2::draw(GL_TRIANGLE_FAN, letterS_VAO, rectangle.size());
        CSCI441::SimpleShader2::popTransformation();
    CSCI441::SimpleShader2::popTransformation();

    transMtx = glm::translate( glm::mat4(1.0), glm::vec3( x_Translation + 395, y_Translation - 3, 0 ) );
    CSCI441::SimpleShader2::pushTransformation( transMtx );
        scaleMtx = glm::scale( glm::mat4(1.0), glm::vec3(30, 10, 1) );
        CSCI441::SimpleShader2::pushTransformation( scaleMtx );
            CSCI441::SimpleShader2::draw(GL_TRIANGLE_FAN, letterS_VAO, rectangle.size());
        CSCI441::SimpleShader2::popTransformation();
    CSCI441::SimpleShader2::popTransformation();

    transMtx = glm::translate( glm::mat4(1.0), glm::vec3( x_Translation + 395, y_Translation + 20, 0 ) );
    CSCI441::SimpleShader2::pushTransformation( transMtx );
        scaleMtx = glm::scale( glm::mat4(1.0), glm::vec3(30, 10, 1) );
        CSCI441::SimpleShader2::pushTransformation( scaleMtx );
            CSCI441::SimpleShader2::draw(GL_TRIANGLE_FAN, letterS_VAO, rectangle.size());
        CSCI441::SimpleShader2::popTransformation();
    CSCI441::SimpleShader2::popTransformation();

    transMtx = glm::translate( glm::mat4(1.0), glm::vec3( x_Translation + 395, y_Translation + 42, 0 ) );
    CSCI441::SimpleShader2::pushTransformation( transMtx );
        scaleMtx = glm::scale( glm::mat4(1.0), glm::vec3(30, 10, 1) );
        CSCI441::SimpleShader2::pushTransformation( scaleMtx );
            CSCI441::SimpleShader2::draw(GL_TRIANGLE_FAN, letterS_VAO, rectangle.size());
        CSCI441::SimpleShader2::popTransformation();
    CSCI441::SimpleShader2::popTransformation();
}

void drawLetterT2() {
    glm::mat4 transMtx = glm::translate( glm::mat4(1.0), glm::vec3( x_Translation + 450, y_Translation - 3, 0 ) );
    CSCI441::SimpleShader2::pushTransformation( transMtx );
        glm::mat4 scaleMtx = glm::scale( glm::mat4(1.0), glm::vec3(10, 55, 1) );
        CSCI441::SimpleShader2::pushTransformation( scaleMtx );
            CSCI441::SimpleShader2::draw(GL_TRIANGLE_FAN, otherLetterT_VAO, rectangle.size());
        CSCI441::SimpleShader2::popTransformation();
    CSCI441::SimpleShader2::popTransformation();

    transMtx = glm::translate( glm::mat4(1.0), glm::vec3( x_Translation + 435, y_Translation + 42, 0 ) );
    CSCI441::SimpleShader2::pushTransformation( transMtx );
        scaleMtx = glm::scale( glm::mat4(1.0), glm::vec3(40, 10, 1) );
        CSCI441::SimpleShader2::pushTransformation( scaleMtx );
            CSCI441::SimpleShader2::draw(GL_TRIANGLE_FAN, otherLetterT_VAO, rectangle.size());
        CSCI441::SimpleShader2::popTransformation();
    CSCI441::SimpleShader2::popTransformation();
}

void drawLetterY() {
    glm::mat4 transMtx = glm::translate( glm::mat4(1.0), glm::vec3( x_Translation + 500, y_Translation - 3, 0 ) );
    CSCI441::SimpleShader2::pushTransformation( transMtx );
        glm::mat4 scaleMtx = glm::scale( glm::mat4(1.0), glm::vec3(10, 30, 1) );
        CSCI441::SimpleShader2::pushTransformation( scaleMtx );
            CSCI441::SimpleShader2::draw(GL_TRIANGLE_FAN, letterY_VAO, rectangle.size());
        CSCI441::SimpleShader2::popTransformation();
    CSCI441::SimpleShader2::popTransformation();

    angle = 0.5;
    transMtx = glm::translate( glm::mat4(1.0), glm::vec3( x_Translation + 500, y_Translation + 22, 0 ) );
    CSCI441::SimpleShader2::pushTransformation( transMtx );
        glm::mat4 rotMtx = glm::rotate( glm::mat4(1.0), angle, glm::vec3(0, 0, 1) );
        CSCI441::SimpleShader2::pushTransformation( rotMtx );
            scaleMtx = glm::scale( glm::mat4(1.0), glm::vec3(10, 30, 1) );
            CSCI441::SimpleShader2::pushTransformation( scaleMtx );
                CSCI441::SimpleShader2::draw(GL_TRIANGLE_FAN, letterY_VAO, rectangle.size());
            CSCI441::SimpleShader2::popTransformation();
        CSCI441::SimpleShader2::popTransformation();
    CSCI441::SimpleShader2::popTransformation();

    angle = 5.8;
    transMtx = glm::translate( glm::mat4(1.0), glm::vec3( x_Translation + 500, y_Translation + 22, 0 ) );
    CSCI441::SimpleShader2::pushTransformation( transMtx );
        rotMtx = glm::rotate( glm::mat4(1.0), angle, glm::vec3(0, 0, 1) );
        CSCI441::SimpleShader2::pushTransformation( rotMtx );
            scaleMtx = glm::scale( glm::mat4(1.0), glm::vec3(10, 35, 1) );
            CSCI441::SimpleShader2::pushTransformation( scaleMtx );
                CSCI441::SimpleShader2::draw(GL_TRIANGLE_FAN, letterY_VAO, rectangle.size());
            CSCI441::SimpleShader2::popTransformation();
        CSCI441::SimpleShader2::popTransformation();
    CSCI441::SimpleShader2::popTransformation();
}

void drawHammer() {
    angle = 5.45;
    glm::mat4 transMtx = glm::translate( glm::mat4(1.0), glm::vec3( x_Translation + 575, y_Translation + 5, 0 ) );
    CSCI441::SimpleShader2::pushTransformation( transMtx );
        glm::mat4 rotMtx = glm::rotate( glm::mat4(1.0), angle, glm::vec3(0, 0, 1) );
        CSCI441::SimpleShader2::pushTransformation( rotMtx );
            glm::mat4 scaleMtx = glm::scale( glm::mat4(1.0), glm::vec3(9, 45, 1) );
            CSCI441::SimpleShader2::pushTransformation( scaleMtx );
                CSCI441::SimpleShader2::draw(GL_TRIANGLE_FAN, handleVAO, rectangle.size());
            CSCI441::SimpleShader2::popTransformation();
        CSCI441::SimpleShader2::popTransformation();
    CSCI441::SimpleShader2::popTransformation();

    transMtx = glm::translate( glm::mat4(1.0), glm::vec3( x_Translation + 600, y_Translation + 40, 0 ) );
    CSCI441::SimpleShader2::pushTransformation( transMtx );
        rotMtx = glm::rotate( glm::mat4(1.0), angle, glm::vec3(0, 0, 1) );
        CSCI441::SimpleShader2::pushTransformation( rotMtx );
            scaleMtx = glm::scale( glm::mat4(1.0), glm::vec3(25, 20, 1) );
            CSCI441::SimpleShader2::pushTransformation( scaleMtx );
                CSCI441::SimpleShader2::draw(GL_TRIANGLE_FAN, hammerVAO, rectangle.size());
            CSCI441::SimpleShader2::popTransformation();
        CSCI441::SimpleShader2::popTransformation();
    CSCI441::SimpleShader2::popTransformation();
}

void drawSword() {
    angle = 0.7;
    glm::mat4 transMtx = glm::translate( glm::mat4(1.0), glm::vec3( x_Translation + 590, y_Translation + 13, 0 ) );
    CSCI441::SimpleShader2::pushTransformation( transMtx );
        glm::mat4 rotMtx = glm::rotate( glm::mat4(1.0), angle, glm::vec3(0, 0, 1) );
        CSCI441::SimpleShader2::pushTransformation( rotMtx );
            glm::mat4 scaleMtx = glm::scale( glm::mat4(1.0), glm::vec3(8, 40, 1) );
            CSCI441::SimpleShader2::pushTransformation( scaleMtx );
                CSCI441::SimpleShader2::draw(GL_TRIANGLE_FAN, swordVAO, rectangle.size());
            CSCI441::SimpleShader2::popTransformation();
        CSCI441::SimpleShader2::popTransformation();
    CSCI441::SimpleShader2::popTransformation();

    transMtx = glm::translate( glm::mat4(1.0), glm::vec3( x_Translation + 565, y_Translation + 42, 0 ) );
    CSCI441::SimpleShader2::pushTransformation( transMtx );
        rotMtx = glm::rotate( glm::mat4(1.0), angle, glm::vec3(0, 0, 1) );
        CSCI441::SimpleShader2::pushTransformation( rotMtx );
            scaleMtx = glm::scale( glm::mat4(1.0), glm::vec3(9, 9, 1) );
            CSCI441::SimpleShader2::pushTransformation( scaleMtx );
                CSCI441::SimpleShader2::draw(GL_TRIANGLES, swordTipVAO, triangle.size());
            CSCI441::SimpleShader2::popTransformation();
        CSCI441::SimpleShader2::popTransformation();
    CSCI441::SimpleShader2::popTransformation();

    transMtx = glm::translate( glm::mat4(1.0), glm::vec3( x_Translation + 604, y_Translation - 3, 0 ) );
    CSCI441::SimpleShader2::pushTransformation( transMtx );
        rotMtx = glm::rotate( glm::mat4(1.0), angle, glm::vec3(0, 0, 1) );
        CSCI441::SimpleShader2::pushTransformation( rotMtx );
            scaleMtx = glm::scale( glm::mat4(1.0), glm::vec3(7, 20, 1) );
            CSCI441::SimpleShader2::pushTransformation( scaleMtx );
                CSCI441::SimpleShader2::draw(GL_TRIANGLE_FAN, handleVAO, rectangle.size());
            CSCI441::SimpleShader2::popTransformation();
        CSCI441::SimpleShader2::popTransformation();
    CSCI441::SimpleShader2::popTransformation();

    transMtx = glm::translate( glm::mat4(1.0), glm::vec3( x_Translation + 587, y_Translation + 7, 0 ) );
    CSCI441::SimpleShader2::pushTransformation( transMtx );
        rotMtx = glm::rotate( glm::mat4(1.0), angle, glm::vec3(0, 0, 1) );
        CSCI441::SimpleShader2::pushTransformation( rotMtx );
            scaleMtx = glm::scale( glm::mat4(1.0), glm::vec3(20, 4, 1) );
            CSCI441::SimpleShader2::pushTransformation( scaleMtx );
                CSCI441::SimpleShader2::draw(GL_TRIANGLE_FAN, swordVAO, rectangle.size());
            CSCI441::SimpleShader2::popTransformation();
        CSCI441::SimpleShader2::popTransformation();
    CSCI441::SimpleShader2::popTransformation();
}

//
//  void renderScene()
//
//      We will register this function as GLUT's display callback.
//  This is where the magic happens - all rendering is done here.
//
void renderScene() {
    drawLetterA();
    drawLetterE();
    drawLetterT();
    drawLetterH();
    drawLetterO();
    drawLetterN();
    drawLetterM();
    drawLetterI();
    drawLetterS();
    drawLetterT2();
    drawLetterY();
    drawHammer();
    drawSword();
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
