/*
 *  CSCI 441 Computer Graphics, Fall 2020
 *
 *  Project: a2
 *  File: main.cpp
 *
 *  Author: Dr. Jeffrey Paone - Fall 2020
 *
 *  Description:
 *		This is the shell code for Assignment 2.  It contains predefined
 *		methods that simply need to be called properly to have your Hero
 *		move throughout the map.
 */

// include everything necessary to make our world map work
#include "WorldMap.h"

// include our class libraries
#include <CSCI441/OpenGLUtils.hpp>
#include <CSCI441/SimpleShader.hpp>
#include "math.h"

//*************************************************************************************
//
// Global Parameters

// global variables to keep track of window width and height.
// set to initial values for convenience, but we need variables
// for later on in case the window gets resized.
const GLint WINDOW_WIDTH = 512, WINDOW_HEIGHT = 512;
GLuint headVAO = 0, eyeVAO = 0, pupilVAO = 0, armVAO = 0, handVAO = 0, torsoVAO = 0, legVAO = 0, shoeVAO = 0,
mouthVAO = 0, smileVAO = 0;
std::vector<glm::vec2> headPoints, eyePoints, pupilPoints, armPoints, handPoints, torsoPoints, legPoints, shoePoints, mouthPoints, smilePoints;
std::vector<glm::vec3> headColors, eyeColors, pupilColors, armColors, handColors, torsoColors, legColors, shoeColors, mouthColors, smileColors;
int x_origin = 0, y_origin = 0, x_translation = 154, y_translation = 154;
bool mouseClicked = false, ctrlClicked = false, cursorInTorso = false;
int characterX = 0, characterY = 0, characterSpeed = 10;
GLfloat headAngle = 0, characterScale = 0.6, headRate = 0.01, leftArmAngle = -0.2, rightArmAngle = 0.2, armRate = 0.01;
GLfloat pupilTrans_X = 0.0, pupilTrans_Y = 0.0, eyeRadius = 18.0;

bool mackHack = false;

//*************************************************************************************
//
// Event Callbacks

static void error_callback(int error, const char* description) {
	fprintf(stderr, "[ERROR]: (%d) %s\n", error, description);
}

//////////////////////////////////////////////////////////
///// TODO		Add Your Callbacks Here   		 	 /////
//////////////////////////////////////////////////////////

//Note for head rotation, right now it is rotating around the origin, so look at Lab01 to see how to have the object rotate around its own center of mass. although that may have to do with how I am drawing the object initally
//may need to make x_origin and y_origin the 0,0 point instead of 256, 256 and fic the translations that might be main issue
//also still have no idea how to bound the pupils in the irises, but maybe I'll think of something tomorrow. Okay I should go to bed

void keyboard_callback( GLFWwindow *win, int key, int scancode, int action, int mods ) {
    if (key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(win, GLFW_TRUE);
    }
    //Used to set the boolean which helps determine if the arms are animated
    else if ((key == GLFW_KEY_LEFT_CONTROL || key == GLFW_KEY_RIGHT_CONTROL) && action == GLFW_PRESS) {
        ctrlClicked = true;
    } else if ((key == GLFW_KEY_LEFT_CONTROL || key == GLFW_KEY_RIGHT_CONTROL) && action == GLFW_RELEASE) {
        ctrlClicked = false;
    } else if (key == GLFW_KEY_A || key == GLFW_KEY_LEFT) {
        //shift character left
        x_translation -= characterSpeed;
        //update our variable that keeps track of character location
        characterX -= characterSpeed;

        //check if beyond left side of screen
        if (characterX <= 0) {
            //map function
            moveLeft();
            //reset the character location tracker to opposite side of screen
            characterX += 512;
            //reset the character draw location to opposite side of screen
            x_translation += 512;
        }
    } else if (key == GLFW_KEY_D || key == GLFW_KEY_RIGHT) {
        //shift character right
        x_translation += characterSpeed;
        //update our variable that keeps track of character location
        characterX += characterSpeed;

        //check if beyond right side of screen
        if (characterX >= WINDOW_WIDTH) {
            //map function
            moveRight();
            //reset the character location tracker to opposite side of screen
            characterX -= 512;
            //reset the character draw location to opposite side of screen
            x_translation -= 512;
        }
    } else if (key == GLFW_KEY_S || key == GLFW_KEY_DOWN) {
        //shift character down
        y_translation -= characterSpeed;
        //update our variable that keeps track of character location
        characterY -= characterSpeed;

        //check if beyond the bottom of the screen
        if (characterY <= 0) {
            //map function
            moveDown();
            //reset the character location tracker to opposite side of screen
            characterY += 512;
            //reset the character draw location to opposite side of screen
            y_translation += 512;
        }
    } else if (key == GLFW_KEY_W || key == GLFW_KEY_UP) {
        //shift character up
        y_translation += characterSpeed;
        //update our variable that keeps track of character location
        characterY += characterSpeed;

        //check if beyond the top of the screen
        if (characterY >= WINDOW_HEIGHT) {
            //map function
            moveUp();
            //reset the character location tracker to opposite side of screen
            characterY -= 512;
            //reset the character draw location to opposite side of screen
            y_translation -= 512;
        }
    }
}

void mouse_button_callback( GLFWwindow *window, int button, int action, int mods ) {
    //Used for changing the character's expression and animating their arms (if control is also pressed)
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            mouseClicked = true;
        } else if (action == GLFW_RELEASE) {
            mouseClicked = false;
        }
    }
}

void cursor_callback( GLFWwindow *window, double x, double y ) {
    //check if mouse is within the torso
    if (x <= characterX + (80*characterScale) && x >= characterX - (80*characterScale) &&
    (WINDOW_HEIGHT - y) <= characterY + (100*characterScale) && (WINDOW_HEIGHT - y) >= characterY - (100*characterScale))
        cursorInTorso = true;
    else
        cursorInTorso = false;

    //animating the pupils to follow the cursor
    pupilTrans_X = 0;
    pupilTrans_Y = 0;
    GLfloat distX = x - characterX;
    GLfloat distY = (WINDOW_HEIGHT - y) - characterY;
    GLfloat distRight = 0, distUp = 0;
    if (distX > 0)
        distRight = WINDOW_WIDTH - characterX;
    else
        distRight = characterX;
    if (distY > 0)
        distUp = WINDOW_HEIGHT - characterY;
    else
        distUp = characterY;
    GLfloat ratioX = distX / distRight, ratioY = distY / distUp;
    //Prevent the pupils going beyond the iris
    if (ratioX > 1.0)
        ratioX = 1.0;
    if (ratioY > 1.0)
        ratioY = 1.0;
    if (ratioX < -1.0)
        ratioX = -1.0;
    if (ratioY < -1.0)
        ratioY = -1.0;

    pupilTrans_X = ratioX * (eyeRadius * characterScale);
    pupilTrans_Y = ratioY * (eyeRadius * characterScale);

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
	if (!glfwInit()) {
		fprintf( stderr, "[ERROR]: Could not initialize GLFW\n" );
		exit(EXIT_FAILURE);
	} else {
		fprintf( stdout, "[INFO]: GLFW initialized\n" );
	}

    glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );              // request forward compatible OpenGL context
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );    // request OpenGL Core Profile context
	glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );	            // request OpenGL v4.X
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 1 );	            // request OpenGL vX.1
    glfwWindowHint( GLFW_RESIZABLE, GLFW_FALSE );		                // do not allow our window to be able to be resized
    glfwWindowHint( GLFW_DOUBLEBUFFER, GLFW_TRUE  );	                // use double buffering

	// create a window for a given size, with a given title
	GLFWwindow *window = glfwCreateWindow( WINDOW_WIDTH, WINDOW_HEIGHT, "Assignment 2", nullptr, nullptr );
	if( !window ) {
        fprintf( stderr, "[ERROR]: GLFW Window could not be created\n" );
        glfwTerminate();
		exit(EXIT_FAILURE);
	} else {
		fprintf( stdout, "[INFO]: GLFW Window created\n" );
	}

	glfwMakeContextCurrent(window);		                                // make the created window the current window
	glfwSwapInterval(1);				                        // update our window after at least 1 screen refresh

    //////////////////////////////////////////////////////////
    /////	TODO	Add Your Callback Registration Here	 /////
    //////////////////////////////////////////////////////////
    glfwSetKeyCallback( window, keyboard_callback);
    glfwSetMouseButtonCallback( window, mouse_button_callback );
    glfwSetCursorPosCallback( window, cursor_callback );

	return window;						                                // return the window that was created
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
        fprintf( stderr, "[ERROR]: Error initializing GLEW\n" );
        exit(EXIT_FAILURE);
    }

	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );	// set the clear color to black
}

//
// void setupBuffers()
//
//      Register all of your vertex attribute arrays inside this function
//
void setupBuffers() {

    //////////////////////////////////////////////////////////
    ///// TODO		Add Your Vertex Registration Here    /////
    //////////////////////////////////////////////////////////

    //creating points for character's head, eyes, pupils, hand, centered at the 0,0 origin
    for (int i = 0; i < 127; ++i) {
        int head_x = x_origin + 77*cos(0.05*i);
        int head_y = y_origin + 70*sin(0.05*i);
        headPoints.emplace_back(glm::vec2(head_x, head_y));
        headColors.emplace_back(glm::vec3(0.82, 0.63, 0.55));

        int eye_x = x_origin + eyeRadius*cos(0.05*i);
        int eye_y = y_origin + eyeRadius*sin(0.05*i);
        eyePoints.emplace_back(glm::vec2(eye_x, eye_y));
        eyeColors.emplace_back(glm::vec3(0.2, 0.48, 0.17));

        int pupil_x = x_origin + 5*cos(0.05*i);
        int pupil_y = y_origin + 5*sin(0.05*i);
        pupilPoints.emplace_back(glm::vec2(pupil_x, pupil_y));
        pupilColors.emplace_back(glm::vec3(0, 0, 0));

        int hand_x = x_origin + 20*cos(0.05*i);
        int hand_y = y_origin + 20*sin(0.05*i);
        handPoints.emplace_back(glm::vec2(hand_x, hand_y));
        handColors.emplace_back(glm::vec3(0.82, 0.63, 0.55));
    }

    legPoints = {
            glm::vec2(x_origin - 25,y_origin + 60),
            glm::vec2(x_origin - 25,y_origin - 60),
            glm::vec2(x_origin + 25,y_origin - 60),
            glm::vec2(x_origin + 25,y_origin + 60),
    };
    legColors = {glm::vec3(0.19,0.25,0.59),glm::vec3(0.19,0.25,0.59),glm::vec3(0.19,0.25,0.59),glm::vec3(0.19,0.25,0.59)};

    shoePoints = {
            glm::vec2(x_origin - 40, y_origin + 20),
            glm::vec2(x_origin - 40, y_origin - 20),
            glm::vec2(x_origin + 40, y_origin - 20),
            glm::vec2(x_origin + 40, y_origin + 20),

    };
    shoeColors = {glm::vec3(0.43, 0.35, 0.25),glm::vec3(0.43, 0.35, 0.25),glm::vec3(0.43, 0.35, 0.25),glm::vec3(0.43, 0.35, 0.25),};

    torsoPoints = {
            glm::vec2(x_origin - 80, y_origin + 100),
            glm::vec2(x_origin - 65, y_origin - 100),
            glm::vec2(x_origin + 65, y_origin - 100),
            glm::vec2(x_origin + 80, y_origin + 100),
    };
    torsoColors = {glm::vec3(0.79, 0.75, 0.55), glm::vec3(0.79, 0.75, 0.55), glm::vec3(0.79, 0.75, 0.55), glm::vec3(0.79, 0.75, 0.55)};

    armPoints = {
            glm::vec2(x_origin - 20, y_origin),
            glm::vec2(x_origin - 20, y_origin - 160),
            glm::vec2(x_origin + 20, y_origin - 160),
            glm::vec2(x_origin + 20, y_origin),
    };
    armColors = {glm::vec3(0.79, 0.75, 0.55), glm::vec3(0.79, 0.75, 0.55), glm::vec3(0.79, 0.75, 0.55), glm::vec3(0.79, 0.75, 0.55)};

    mouthPoints = {
            glm::vec2(x_origin - 40, y_origin + 8),
            glm::vec2(x_origin - 20, y_origin - 5),
            glm::vec2(x_origin + 20, y_origin - 5),
            glm::vec2(x_origin + 40, y_origin + 8)
    };
    mouthColors = {glm::vec3(0.84, 0.23, 0.23),glm::vec3(0.84, 0.23, 0.23),glm::vec3(0.84, 0.23, 0.23),glm::vec3(0.84, 0.23, 0.23)};

    smilePoints = {
            glm::vec2(x_origin - 40, y_origin + 8),
            glm::vec2(x_origin - 20, y_origin - 5),
            glm::vec2(x_origin + 20, y_origin - 5),
            glm::vec2(x_origin + 40, y_origin + 8)
    };
    smileColors = {glm::vec3(0.84, 0.23, 0.23),glm::vec3(0.84, 0.23, 0.23),glm::vec3(0.84, 0.23, 0.23),glm::vec3(0.84, 0.23, 0.23)};

    headVAO = CSCI441::SimpleShader2::registerVertexArray(headPoints, headColors);
    eyeVAO = CSCI441::SimpleShader2::registerVertexArray(eyePoints, eyeColors);
    pupilVAO = CSCI441::SimpleShader2::registerVertexArray(pupilPoints, pupilColors);
    handVAO = CSCI441::SimpleShader2::registerVertexArray(handPoints, handColors);
    legVAO = CSCI441::SimpleShader2::registerVertexArray(legPoints, legColors);
    torsoVAO = CSCI441::SimpleShader2::registerVertexArray(torsoPoints, torsoColors);
    armVAO = CSCI441::SimpleShader2::registerVertexArray(armPoints, armColors);
    shoeVAO = CSCI441::SimpleShader2::registerVertexArray(shoePoints, shoeColors);
    mouthVAO = CSCI441::SimpleShader2::registerVertexArray(mouthPoints, mouthColors);
    smileVAO = CSCI441::SimpleShader2::registerVertexArray(smilePoints, smileColors);

    //used to keep track of the character's center of mass
    characterX = x_origin * characterScale + x_translation;
    characterY = y_origin * characterScale + y_translation;
}

//*************************************************************************************
//
// Rendering / Drawing Functions - this is where the magic happens!

void drawEyes() {
    CSCI441::SimpleShader2::draw(GL_TRIANGLE_FAN, eyeVAO, eyePoints.size());
    glm::mat4 transMtx = glm::translate( glm::mat4(1.0), glm::vec3( pupilTrans_X, pupilTrans_Y, 0 ) );
    CSCI441::SimpleShader2::pushTransformation( transMtx );
        CSCI441::SimpleShader2::draw(GL_TRIANGLE_FAN, pupilVAO, pupilPoints.size());
    CSCI441::SimpleShader2::popTransformation();
}

void drawHead() {
    CSCI441::SimpleShader2::draw(GL_TRIANGLE_FAN, headVAO, headPoints.size());

    glm::mat4 transMtx = glm::translate( glm::mat4(1.0), glm::vec3( -30, 10, 0 ) );
    CSCI441::SimpleShader2::pushTransformation( transMtx );
        drawEyes();
    CSCI441::SimpleShader2::popTransformation();


    transMtx = glm::translate( glm::mat4(1.0), glm::vec3( 30, 10, 0 ) );
    CSCI441::SimpleShader2::pushTransformation( transMtx );
        drawEyes();
    CSCI441::SimpleShader2::popTransformation();

    transMtx = glm::translate( glm::mat4(1.0), glm::vec3( 0, -35, 0 ) );
    CSCI441::SimpleShader2::pushTransformation( transMtx );
    //if mouse clicked and was clicked within the bounds of the torso, draw the smile
    if (mouseClicked && cursorInTorso)
        CSCI441::SimpleShader2::draw(GL_TRIANGLE_FAN, smileVAO, smilePoints.size());
    //otherwise draw just the mouth
    else
        CSCI441::SimpleShader2::draw(GL_LINE_STRIP, mouthVAO, mouthPoints.size());
    CSCI441::SimpleShader2::popTransformation();
}

void drawTorso() {
    CSCI441::SimpleShader2::draw(GL_TRIANGLE_FAN, torsoVAO, torsoPoints.size());
}

void drawArm() {
    CSCI441::SimpleShader2::draw(GL_TRIANGLE_FAN, armVAO, armPoints.size());

    glm::mat4 transMtx = glm::translate( glm::mat4(1.0), glm::vec3( 0, -180, 0 ) );
    CSCI441::SimpleShader2::pushTransformation( transMtx );
        CSCI441::SimpleShader2::draw(GL_TRIANGLE_FAN, handVAO, handPoints.size());
    CSCI441::SimpleShader2::popTransformation();
}

void drawLegs() {
    glm::mat4 transMtx = glm::translate( glm::mat4(1.0), glm::vec3( -40, 0, 0 ) );
    CSCI441::SimpleShader2::pushTransformation( transMtx );
        CSCI441::SimpleShader2::draw(GL_TRIANGLE_FAN, legVAO, legPoints.size());

        transMtx = glm::translate( glm::mat4(1.0), glm::vec3( -15, -80, 0 ) );
        CSCI441::SimpleShader2::pushTransformation( transMtx );
            CSCI441::SimpleShader2::draw(GL_TRIANGLE_FAN, shoeVAO, shoePoints.size());
        CSCI441::SimpleShader2::popTransformation();
    CSCI441::SimpleShader2::popTransformation();

    transMtx = glm::translate( glm::mat4(1.0), glm::vec3( 40, 0, 0 ) );
    CSCI441::SimpleShader2::pushTransformation( transMtx );
        CSCI441::SimpleShader2::draw(GL_TRIANGLE_FAN, legVAO, legPoints.size());

        transMtx = glm::translate( glm::mat4(1.0), glm::vec3( 15, -80, 0 ) );
        CSCI441::SimpleShader2::pushTransformation( transMtx );
            CSCI441::SimpleShader2::draw(GL_TRIANGLE_FAN, shoeVAO, shoePoints.size());
        CSCI441::SimpleShader2::popTransformation();
    CSCI441::SimpleShader2::popTransformation();
}

void drawCharacter() {
    glm::mat4 transMtx = glm::translate( glm::mat4(1.0), glm::vec3( 0, 165, 0 ) );
    CSCI441::SimpleShader2::pushTransformation( transMtx );
        glm::mat4 rotMtx = glm::rotate( glm::mat4(1.0), headAngle, glm::vec3(0, 0, 1) );
        CSCI441::SimpleShader2::pushTransformation( rotMtx );
            drawHead();
        CSCI441::SimpleShader2::popTransformation();
    CSCI441::SimpleShader2::popTransformation();

    transMtx = glm::translate( glm::mat4(1.0), glm::vec3( 0, 0, 0 ) );
    CSCI441::SimpleShader2::pushTransformation( transMtx );
        drawTorso();
    CSCI441::SimpleShader2::popTransformation();

    transMtx = glm::translate( glm::mat4(1.0), glm::vec3( -90, 100, 0 ) );
    CSCI441::SimpleShader2::pushTransformation( transMtx );
        rotMtx = glm::rotate( glm::mat4(1.0), leftArmAngle, glm::vec3(0, 0, 1) );
        CSCI441::SimpleShader2::pushTransformation( rotMtx );
            drawArm();
        CSCI441::SimpleShader2::popTransformation();
    CSCI441::SimpleShader2::popTransformation();

    transMtx = glm::translate( glm::mat4(1.0), glm::vec3( 90, 100, 0 ) );
    CSCI441::SimpleShader2::pushTransformation( transMtx );
        rotMtx = glm::rotate( glm::mat4(1.0), rightArmAngle, glm::vec3(0, 0, 1) );
        CSCI441::SimpleShader2::pushTransformation( rotMtx );
            drawArm();
        CSCI441::SimpleShader2::popTransformation();
    CSCI441::SimpleShader2::popTransformation();

    transMtx = glm::translate( glm::mat4(1.0), glm::vec3( 0, -160, 0 ) );
    CSCI441::SimpleShader2::pushTransformation( transMtx );
        drawLegs();
    CSCI441::SimpleShader2::popTransformation();
}


//
//	void renderScene()
//
//		This method will contain all of the objects to be drawn.
//
void renderScene() {
	  // draw our World Map to the screen.  this MUST be your first drawing call
    drawMap();	// DO NOT REMOVE THIS LINE

    //////////////////////////////////////////////////////////
    ///// TODO		Add Your Drawing Below Here		 	 /////
    //////////////////////////////////////////////////////////
    glm::mat4 transMtx = glm::translate( glm::mat4(1.0), glm::vec3( x_translation, y_translation, 0 ) );
    CSCI441::SimpleShader2::pushTransformation( transMtx );
        glm::mat4 scaleMtx = glm::scale( glm::mat4(1.0), glm::vec3(characterScale, characterScale, 1) );
        CSCI441::SimpleShader2::pushTransformation( scaleMtx );
            drawCharacter();
        CSCI441::SimpleShader2::popTransformation();
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
int main( ) {
    GLFWwindow *window = setupGLFW();	            // initialize all of the GLFW specific information related to OpenGL and our window
                                                    // GLFW sets up our OpenGL context so must be done first
    setupOpenGL();						            // initialize all of the OpenGL specific information
    CSCI441::OpenGLUtils::printOpenGLInfo();        // print out diagnostic information about the OpenGL version
    CSCI441::SimpleShader2::setupSimpleShader();    // setup our shaders and enable vertex array attributes
    setupBuffers();                                 // register vertex data with the GPU

	initMap(WINDOW_WIDTH, WINDOW_HEIGHT);			// initialize our map

	//  This is our draw loop - all rendering is done here.  We use a loop to keep the window open
	//	until the user decides to close the window and quit the program.  Without a loop, the
	//	window will display once and then the program exits.
	while( !glfwWindowShouldClose(window) ) {
	    //idle head animation
	    headAngle += headRate;
	    if(headAngle <= -0.4 || headAngle >= 0.4) {
            headRate = headRate * -1;
	    }
	    //arm animation if mouse and control key clicked at same time, does not reset angle when let go
	    if (mouseClicked && ctrlClicked) {
            leftArmAngle -= armRate;
            rightArmAngle += armRate;
            if ((leftArmAngle <= -0.5 || leftArmAngle >= -0.05) && (rightArmAngle >= 0.5 || rightArmAngle <= 0.05)) {
                armRate = armRate * -1;
            }
        }
		glDrawBuffer( GL_BACK );		// ensure we are drawing to the back buffer
		glClear( GL_COLOR_BUFFER_BIT );	// clear the current color contents in the buffer

        // update the projection matrix based on the window size
        // the projection matrix governs properties of the view coordinates;
        // i.e. what gets seen - use an Orthographic projection that ranges
        // from [0, windowWidth] in X and [0, windowHeight] in Y. (0,0) is the lower left.
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

        //////////////////////////////////////////////////////////
        ///// TODO		Add Your Idle Animation Steps Here	 /////
        //////////////////////////////////////////////////////////

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

	return 0;
}
