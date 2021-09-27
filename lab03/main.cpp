/*
 *  CSCI 441, Computer Graphics, Fall 2020
 *
 *  Project: lab03
 *  File: main.cpp
 *
 *  Description:
 *      This file contains the basic setup to work with VAOs & VBOs using a
 *	MD5 model.
 *
 *  Author: Dr. Paone, Colorado School of Mines, 2020
 *
 */

//******************************************************************************

#include <GL/glew.h>
#include <GLFW/glfw3.h>			// include GLFW framework header

// include GLM libraries and matrix functions
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>		        // for image loading - necessary for MD5Model

#include <cstdio>				    // for printf functionality
#include <cstdlib>				    // for exit functionality

#include "lab03.h"                  // for Lab03BlackMagic
#include <MD5/md5model.h>	// for our MD5 Model

#include <CSCI441/OpenGLUtils.hpp>
#include <vector>

//******************************************************************************
//
// Global Parameters

int windowWidth, windowHeight;
bool controlDown = false;
bool leftMouseDown = false;
glm::vec2 mousePosition( -9999.0f, -9999.0f );

// TODO #04A create a global VAO descriptor for our platform
GLuint platformVAOd = 0;

md5_model_t md5model;
md5_anim_t md5animation;

md5_joint_t *skeleton = nullptr;
anim_info_t animInfo;

bool animated = false;
bool displaySkeleton = false;
bool displayWireframe = false;
bool displayMesh = true;

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
    if(action == GLFW_PRESS) {
        switch( key ) {
            case GLFW_KEY_Q:
            case GLFW_KEY_ESCAPE:
                glfwSetWindowShouldClose( window, GLFW_TRUE );
                break;

            case GLFW_KEY_S:
                displaySkeleton = !displaySkeleton;
                break;

            case GLFW_KEY_W:
                displayWireframe = !displayWireframe;
                break;

            case GLFW_KEY_M:
                displayMesh = !displayMesh;
                break;

            default: break;
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

	glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );						// request forward compatible OpenGL context
	glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );	        // request OpenGL Core Profile context
	glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );		                // request OpenGL 4.X context
	glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 1 );		                // request OpenGL X.1 context
	glfwWindowHint( GLFW_DOUBLEBUFFER, GLFW_TRUE );

	// create a window for a given size, with a given title
	GLFWwindow *window = glfwCreateWindow(640, 480, "Lab03: VAOs & VBOs", nullptr, nullptr );
	if( !window ) {						                                        // if the window could not be created, NULL is returned
		fprintf( stderr, "[ERROR]: GLFW Window could not be created\n" );
		glfwTerminate();
		exit( EXIT_FAILURE );
	} else {
		fprintf( stdout, "[INFO]: GLFW Window created\n" );
	}

	glfwMakeContextCurrent(	window );	                                        // make the created window the current window
	glfwSwapInterval( 1 );				                                // update our screen after at least 1 screen refresh

	glfwSetKeyCallback( window, key_callback );                             	// set our keyboard callback function

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

	/* check for an error */
	if( glewResult != GLEW_OK ) {
		printf( "[ERROR]: Error initalizing GLEW\n");
  	    fprintf( stderr, "[ERROR]: %s\n", glewGetErrorString(glewResult) );
		exit(EXIT_FAILURE);
	} else {
		 fprintf( stdout, "[INFO]: GLEW initialized\n" );
	}
}

// setupBuffers() //////////////////////////////////////////////////////////////
//
//      Create our VAOs & VBOs. Send vertex data to the GPU for future rendering
//
////////////////////////////////////////////////////////////////////////////////
void setupBuffers() {

    // TODO #01 - create your struct
    struct VertexColored {
        float x, y, z;
        float r, g, b;
    };
    // TODO #02 - create the array of structures
    VertexColored platformVertexArray[4] {
            {-10, 0, -10, 1, 0, 0},
            {10, 0, -10, 0, 1, 0},
            {-10, 0, 10, 0, 0, 1},
            {10, 0, 10, 1, 1, 1},
    };
    // TODO #03 - create index array to form a quad from the four vertices
    unsigned short quadIndexArray[4] = {0, 1, 2, 3};
    // TODO #04B - generate and bind the platform VAO.  also check that this value is nonzero
    glGenVertexArrays(1, &platformVAOd);
    if(platformVAOd != 0)
        glBindVertexArray(platformVAOd);
    // TODO #05A - generate and bind a VBO for the GL_ARRAY_BUFFER, check that the value is nonzero
    GLuint platformVBOd = 0;
    glGenBuffers(1, &platformVBOd);
    if(platformVBOd != 0)
        glBindBuffer(GL_ARRAY_BUFFER, platformVBOd);
    // TODO #05B - send the platform vertex data to the GPU
    glBufferData(GL_ARRAY_BUFFER, sizeof(platformVertexArray), platformVertexArray, GL_STATIC_DRAW);
    // TODO #06A - tell the GPU where vertex positions are located within the data array
    glEnableVertexAttribArray(Lab03BlackMagic::SHADER_ATTRIBUTES.vertexPosition);
    glVertexAttribPointer(Lab03BlackMagic::SHADER_ATTRIBUTES.vertexPosition, 3, GL_FLOAT, GL_FALSE, sizeof(VertexColored), (void*)0);
    // TODO #06B - tell the GPU where vertex colors are located within the data array
    glEnableVertexAttribArray(Lab03BlackMagic::SHADER_ATTRIBUTES.vertexColor);
    glVertexAttribPointer(Lab03BlackMagic::SHADER_ATTRIBUTES.vertexColor, 3, GL_FLOAT, GL_FALSE, sizeof(VertexColored), (void*)(sizeof(float) * 3));
    // TODO #07 - generate and bind an IBO for the GL_ELEMENT_ARRAY_BUFFER, check that the value is nonzero
    //  then send the index array data to the GPU
    GLuint platformIBOd = 0;
    glGenBuffers(1, &platformIBOd);
    if(platformIBOd != 0)
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, platformIBOd);

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndexArray), quadIndexArray, GL_STATIC_DRAW);
}

// loadMD5Model() //////////////////////////////////////////////////////////////
//
//      Load in the MD5 Model
//
////////////////////////////////////////////////////////////////////////////////
void loadMD5Model( const char *md5mesh, const char *md5anim ) {
	/* Load MD5 model file */
	if (!ReadMD5Model (md5mesh, &md5model))
			exit (EXIT_FAILURE);

    // allocate memory for arrays and create VAO for MD5 Model
	AllocVertexArrays (Lab03BlackMagic::SHADER_ATTRIBUTES.vertexPosition,
                       Lab03BlackMagic::SHADER_ATTRIBUTES.vertexColor,
                       Lab03BlackMagic::SHADER_ATTRIBUTES.vertexTextureCoord,
                       md5model.num_joints);

	if( md5anim ) {
		/* Load MD5 animation file */
		if (!ReadMD5Anim (md5anim, &md5animation)) {
				exit (EXIT_FAILURE);
		} else {
				// successful loading...set up animation parameters
				animInfo.curr_frame = 0;
				animInfo.next_frame = 1;

				animInfo.last_time = 0;
				animInfo.max_time = 1.0 / md5animation.frameRate;

				/* Allocate memory for animated skeleton */
				skeleton = (md5_joint_t *)malloc (sizeof (md5_joint_t) * md5animation.num_joints);

                if( CheckAnimValidity (&md5model, &md5animation) ) {
                    animated = true;
                }
		}
	}

	if( !animated ) {
			printf ("[.md5anim]: no animation loaded.\n");
	}

	printf("\n");
}

//******************************************************************************
//
// Rendering / Drawing Functions - this is where the magic happens!

// renderScene() ///////////////////////////////////////////////////////////////
//
//		This method will contain all of the objects to be drawn.
//
////////////////////////////////////////////////////////////////////////////////
void renderScene( glm::mat4 viewMatrix, glm::mat4 projectionMatrix, double dt ) {
    Lab03BlackMagic::setMVPMatrices( projectionMatrix, viewMatrix );

    // DON'"'T CHANGE ANYTHING ABOVE HERE
    // TODO #08 - bind our platform VAO and draw it!
    glBindVertexArray(platformVAOd);
    glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, (void*)0);
    // DON'T CHANGE ANYTHING BELOW HERE

	if( animated ) {
		/* Calculate current and next frames */
		Animate (&md5animation, &animInfo, dt);

		/* Interpolate skeletons between two frames */
		InterpolateSkeletons (md5animation.skelFrames[animInfo.curr_frame],
													md5animation.skelFrames[animInfo.next_frame],
													md5animation.num_joints,
													animInfo.last_time * md5animation.frameRate,
													skeleton);
    } else {
		skeleton = md5model.baseSkel;
	}

	if( displaySkeleton ) {
        Lab03BlackMagic::setSkeletonShader();
		DrawSkeleton( skeleton, md5model.num_joints );
	}

    Lab03BlackMagic::setMD5Shader();

	/* Draw each mesh of the model */
	for( int i = 0; i < md5model.num_meshes; ++i ) {
			md5_mesh_t mesh = md5model.meshes[i];
			PrepareMesh( &mesh, skeleton );

			if( displayWireframe )
				glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);

			if( displayMesh )
				DrawMesh( &mesh );
	}
}

///*****************************************************************************
//
// Our main function

// main() ///////////////////////////////////////////////////////////////
//
int main() {
	// GLFW sets up our OpenGL context so must be done first
	GLFWwindow *window = setupGLFW();	                // initialize all of the GLFW specific information releated to OpenGL and our window
    setupGLEW();										// initialize all of the GLEW specific information
    setupOpenGL();										// initialize all of the OpenGL specific information

    CSCI441::OpenGLUtils::printOpenGLInfo();

	// hidden Lab03 Black Magic to set up additional settings
	// specifically, compiles a shader for our renderer to use
    Lab03BlackMagic::setupShaders();

	setupBuffers();										// load all our VAOs and VBOs into memory

    // load the MD5 Model & animation
	loadMD5Model("models/monsters/hellknight/mesh/hellknight.md5mesh","models/monsters/hellknight/animations/idle2.md5anim" );

	// used to keep track of the time between frames rendered
	double current_time = glfwGetTime(), last_time = 0;

	//  This is our draw loop - all rendering is done here.  We use a loop to keep the window open
	//	until the user decides to close the window and quit the program.  Without a loop, the
	//	window will display once and then the program exits.
	while( !glfwWindowShouldClose(window) ) {	        // check if the window was instructed to be closed
		last_time = current_time;				        // time the last frame was rendered
		current_time = glfwGetTime();		            // time this frame is starting at

		glDrawBuffer( GL_BACK );				        // work with our back frame buffer
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
		glm::mat4 viewMatrix = glm::lookAt( glm::vec3(21.928f, 10.630f, 5.581f),
                                      glm::vec3(0.0f, 0.0f, 0.0f),
                                      glm::vec3(0.0f, 1.0f, 0.0f) );

		// draw everything to the window
		// pass our view and projection matrices as well as deltaTime between frames
		renderScene( viewMatrix, projectionMatrix, current_time - last_time );

		glfwSwapBuffers(window);                        // flush the OpenGL commands and make sure they get rendered!
		glfwPollEvents();				                // check for any events and signal to redraw screen
	}

	glfwDestroyWindow( window );                        // clean up and close our window
	glfwTerminate();						            // shut down GLFW to clean up our context

	// free up memory created by Lab08 Black Magic
    Lab03BlackMagic::deleteShaders();

    // TODO #12B delete the VAO for the platform
    glDeleteBuffers(1, &platformVAOd);
	// free up memory used by the MD5 model
	FreeVertexArrays();
	FreeAnim (&md5animation);
	FreeModel(&md5model);

	return EXIT_SUCCESS;				                // exit our program successfully!
}
