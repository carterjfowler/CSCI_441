/*
 *  CSCI 441, Computer Graphics, Fall 2020
 *
 *  Project: lab05
 *  File: main.cpp
 *
 *  Description:
 *      This file contains the basic setup to work with GLSL shaders.
 *
 *  Author: Dr. Paone, Colorado School of Mines, 2020
 *
 */

//*************************************************************************************

// include OpenGL and GLFW libraries
#include <GL/glew.h>                    // include GLEW to get our OpenGL 3.0+ bindings
#include <GLFW/glfw3.h>			        // include GLFW framework header

// include GLM libraries and matrix functions
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// include C/C++ libraries
#include <cstdio>				        // for printf functionality
#include <cstdlib>				        // for exit functionality
#include <vector>                       // for vector functionality

// include our class CSCI441 libraries
#include <CSCI441/OpenGLUtils.hpp>      // to print info about OpenGL
#include <CSCI441/objects.hpp>          // to render our 3D primitives
#include <CSCI441/ShaderProgram.hpp>    // a wrapper class for Shader Programs

//*************************************************************************************
//
// Global Parameters

// global variables to keep track of window width and height.
// set to initial values for convenience
const GLint WINDOW_WIDTH = 640, WINDOW_HEIGHT = 480;

GLboolean leftMouseDown;    	 		// status ff the left mouse button is pressed
glm::vec2 mousePosition;				// last known X and Y of the mouse

glm::vec3 camPos;            			// camera position in cartesian coordinates
glm::vec3 camAngles;               		// camera DIRECTION in spherical coordinates stored as (theta, phi, radius)
glm::vec3 camDir; 			            // camera DIRECTION in cartesian coordinates
glm::vec2 cameraSpeed;                  // cameraSpeed --> x = forward/backward delta, y = rotational delta

GLboolean keys[256] = {0};              // keep track of our key states

GLfloat propAngle;                      // angle of rotation for our plane propeller

struct BuildingData {                   // stores the information unique to a single building
    glm::mat4 modelMatrix;                  // the translation/scale of each building
    glm::vec3 color;                        // the color of each building
};
std::vector<BuildingData> buildings;    // information for all of our buildings

GLuint groundVAO;                       // the VAO descriptor for our ground plane

// Shader Program information
CSCI441::ShaderProgram *lightingShader = nullptr;   // the wrapper for our shader program
struct LightingShaderUniforms {         // stores the locations of all of our shader uniforms
    // TODO #1 add variables to store the new uniforms that were created
    GLint lightDir_uniform_location, diffLightColor_uniform_location,
        specLightColor_uniform_location, ambLightColor_uniform_location;
    GLint spotLightDir_uniform_location, spotLightPos_uniform_loc, diffSpotLightColor_uniform_location,
        specSpotLightColor_uniform_location, ambSpotLightColor_uniform_location, spotLightTheta;
    GLint normalMtx;
    GLint mvpMatrix;
    GLint modelMatrix;
    GLint materialColor;
    GLint eyePos;
} lightingShaderUniforms;
struct LightingShaderAttributes {       // stores the locations of all of our shader attributes
    // TODO #2 add variables to store the new attributes that were created
    GLint vertexNormal_attr_location;
    GLint vPos;
} lightingShaderAttributes;

//*************************************************************************************
//
// Helper Functions

// getRand() ///////////////////////////////////////////////////////////////////
//
//  Simple helper function to return a random number between 0.0f and 1.0f.
//
////////////////////////////////////////////////////////////////////////////////
GLfloat getRand() {
    return (GLfloat)rand() / (GLfloat)RAND_MAX;
}

// updateCameraDirection() /////////////////////////////////////////////////////
//
// This function updates the camera's position in cartesian coordinates based
//  on its position in spherical coordinates. Should be called every time
//  cameraTheta, cameraPhi, or cameraRadius is updated.
//
////////////////////////////////////////////////////////////////////////////////
void updateCameraDirection() {
    // ensure phi doesn't flip our camera
    if( camAngles.y <= 0 ) camAngles.y = 0.0f + 0.001f;
    if( camAngles.y >= M_PI ) camAngles.y = M_PI - 0.001f;

    // convert from spherical to cartesian in our RH coord. sys.
    camDir.x = camAngles.z * sinf( camAngles.x ) * sinf( camAngles.y );
    camDir.y = camAngles.z * -cosf( camAngles.y );
    camDir.z = camAngles.z * -cosf( camAngles.x ) * sinf( camAngles.y );

    // normalize the direction for a free cam
    camDir = glm::normalize(camDir);
}

// computeAndSendMatrixUniforms() ///////////////////////////////////////////////
//
// This function precomputes the matrix uniforms CPU-side and then sends them
// to the GPU to be used in the shader for each vertex.  It is more efficient
// to calculate these once and then use the resultant product in the shader.
//
////////////////////////////////////////////////////////////////////////////////
void computeAndSendMatrixUniforms(glm::mat4 modelMtx, glm::mat4 viewMtx, glm::mat4 projMtx) {
    // precompute the Model-View-Projection matrix on the CPU
    glm::mat4 mvpMtx = projMtx * viewMtx * modelMtx;
    // then send it to the shader on the GPU to apply to every vertex
    glUniformMatrix4fv( lightingShaderUniforms.mvpMatrix, 1, GL_FALSE, &mvpMtx[0][0] );

    glUniformMatrix4fv( lightingShaderUniforms.modelMatrix, 1, GL_FALSE, &modelMtx[0][0] );

    // TODO #6 compute and send the normal matrix to the GPU
    glm::mat3 normalMtx = glm::mat3( glm::transpose( glm::inverse( modelMtx ) ) );
    glUniformMatrix3fv(lightingShaderUniforms.normalMtx, 1, GL_FALSE, &normalMtx[0][0]);
}

//*************************************************************************************
//
// Event Callbacks

// error_callback() /////////////////////////////////////////////////////////////
//
//		We will register this function as GLFW's error callback.
//	When an error within GLFW occurs, GLFW will tell us by calling
//	this function.  We can then print this info to the terminal to
//	alert the user.
//
////////////////////////////////////////////////////////////////////////////////
static void error_callback( int error, const char* description ) {
	fprintf( stderr, "[ERROR]: (%d) %s\n", error, description );
}

// keyboard_callback() /////////////////////////////////////////////////////////////
//
//		We will register this function as GLFW's keyboard callback.
//	We only log if a key was pressed/released and will handle the actual event
//  updates later in updateScene().
//      Pressing Q or ESC does close the window and quit the program.
//
////////////////////////////////////////////////////////////////////////////////
static void keyboard_callback( GLFWwindow *window, int key, int scancode, int action, int mods ) {
	if( action == GLFW_PRESS ) {
	    // store that a key had been pressed
	    keys[key] = GL_TRUE;

		switch( key ) {
		    // close our window and quit our program
			case GLFW_KEY_ESCAPE:
			case GLFW_KEY_Q:
				glfwSetWindowShouldClose(window, GLFW_TRUE);
				break;

            default:
		        break;
		}
	} else if( action == GLFW_RELEASE ) {
	    // store that a key is no longer being pressed
	    keys[key] = GL_FALSE;
	}
}

// cursor_callback() ///////////////////////////////////////////////////////////
//
//		We will register this function as GLFW's cursor movement callback.
//	Responds to mouse movement.
//
////////////////////////////////////////////////////////////////////////////////
static void cursor_callback( GLFWwindow* window, double xPos, double yPos) {
    // make sure movement is in bounds of the window
    // glfw captures mouse movement on entire screen
    if( xPos > 0 && xPos < WINDOW_WIDTH ) {
        if( yPos > 0 && yPos < WINDOW_HEIGHT ) {
            // active motion
            if( leftMouseDown ) {
                // ensure we've moved at least one pixel to register a movement from the click
                if( !((mousePosition.x - -9999.0f) < 0.001f) ) {
                    camAngles.x += (xPos - mousePosition.x) * 0.005f;
                    camAngles.y += (mousePosition.y - yPos) * 0.005f;

                    updateCameraDirection();
                }
                mousePosition = glm::vec2( xPos, yPos);
            }
            // passive motion
            else {

            }
        }
    }
}

// mouse_button_callback() /////////////////////////////////////////////////////
//
//		We will register this function as GLFW's mouse button callback.
//	Responds to mouse button presses and mouse button releases.
//
////////////////////////////////////////////////////////////////////////////////
static void mouse_button_callback( GLFWwindow *window, int button, int action, int mods ) {
	if( button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        leftMouseDown = GL_TRUE;
  } else {
        leftMouseDown = GL_FALSE;
	    mousePosition = glm::vec2(-9999.0f, -9999.0f);
	}
}

//*************************************************************************************
//
// Drawing Funcs

void drawPlaneBody( glm::mat4 modelMtx, glm::mat4 viewMtx, glm::mat4 projMtx ) {
    modelMtx = glm::scale( modelMtx, glm::vec3( 2.0f, 0.5f, 1.0f ) );

    computeAndSendMatrixUniforms(modelMtx, viewMtx, projMtx);

    glm::vec3 bodyColor(0.0f, 0.0f, 1.0f);
    glUniform3fv(lightingShaderUniforms.materialColor, 1, &bodyColor[0]);

    CSCI441::drawSolidCube( 0.1 );
}

void drawPlaneWing( bool leftWing, glm::mat4 modelMtx, glm::mat4 viewMtx, glm::mat4 projMtx ) {
    modelMtx = glm::scale( modelMtx, glm::vec3( 1.5f, 0.5f, 1.0f ) );

    glm::mat4 rsMtx;
    if( leftWing )
        modelMtx = glm::rotate( modelMtx, -1.57f, CSCI441::X_AXIS );
    else
        modelMtx = glm::rotate( modelMtx, 1.57f, CSCI441::X_AXIS );

    computeAndSendMatrixUniforms(modelMtx, viewMtx, projMtx);

    glm::vec3 bodyColor(1.0f, 0.0f, 0.0f);
    glUniform3fv(lightingShaderUniforms.materialColor, 1, &bodyColor[0]);
    CSCI441::drawSolidCone( 0.05, 0.2, 16, 16 );
}

void drawPlaneNose( glm::mat4 modelMtx, glm::mat4 viewMtx, glm::mat4 projMtx ) {
    modelMtx = glm::rotate( modelMtx, 1.57f, CSCI441::Z_AXIS );

    computeAndSendMatrixUniforms(modelMtx, viewMtx, projMtx);

    glm::vec3 bodyColor(0.0f, 1.0f, 0.0f);
    glUniform3fv(lightingShaderUniforms.materialColor, 1, &bodyColor[0]);
    CSCI441::drawSolidCone( 0.025, 0.3, 16, 16 );
}

void drawPlanePropeller( glm::mat4 modelMtx, glm::mat4 viewMtx, glm::mat4 projMtx ) {
    glm::mat4 modelMtx1 = glm::translate( modelMtx, glm::vec3( 0.1f, 0.0f, 0.0f ) );
    modelMtx1 = glm::rotate( modelMtx1, propAngle, CSCI441::X_AXIS );
    modelMtx1 = glm::scale( modelMtx1, glm::vec3( 1.1, 1, 0.025 ) );

    computeAndSendMatrixUniforms(modelMtx1, viewMtx, projMtx);

    glm::vec3 bodyColor(1.0f, 1.0f, 1.0f);
    glUniform3fv(lightingShaderUniforms.materialColor, 1, &bodyColor[0]);
    CSCI441::drawSolidCube( 0.1 );

    glm::mat4 modelMtx2 = glm::translate( modelMtx, glm::vec3( 0.1f, 0.0f, 0.0f ) );
    modelMtx2 = glm::rotate( modelMtx2, propAngle+1.57f, CSCI441::X_AXIS );
    modelMtx2 = glm::scale( modelMtx2, glm::vec3( 1.1, 1, 0.025 ) );

    computeAndSendMatrixUniforms(modelMtx2, viewMtx, projMtx);

    CSCI441::drawSolidCube( 0.1 );
}

void drawPlaneTail( glm::mat4 modelMtx, glm::mat4 viewMtx, glm::mat4 projMtx ) {
    computeAndSendMatrixUniforms(modelMtx, viewMtx, projMtx);

    glm::vec3 bodyColor(1.0f, 1.0f, 0.0f);
    glUniform3fv(lightingShaderUniforms.materialColor, 1, &bodyColor[0]);
    CSCI441::drawSolidCone( 0.02, 0.1, 16, 16 );
}

// drawPlane() //////////////////////////////////////////////////////////////////
//
//  A very CRUDE plane
//
////////////////////////////////////////////////////////////////////////////////
void drawPlane( glm::mat4 modelMtx, glm::mat4 viewMtx, glm::mat4 projMtx ) {
    modelMtx = glm::rotate( modelMtx, -1.57f, CSCI441::Y_AXIS );
    modelMtx = glm::rotate( modelMtx, 1.57f, CSCI441::Z_AXIS );
    drawPlaneBody( modelMtx, viewMtx, projMtx );        // the body of our plane
    drawPlaneWing( true, modelMtx, viewMtx, projMtx );  // the left wing
    drawPlaneWing( false, modelMtx, viewMtx, projMtx ); // the right wing
    drawPlaneNose( modelMtx, viewMtx, projMtx );        // the nose
    drawPlanePropeller( modelMtx, viewMtx, projMtx );   // the propeller
    drawPlaneTail( modelMtx, viewMtx, projMtx );        // the tail
}

// renderScene() ///////////////////////////////////////////////////////////////
//
//  Responsible for drawing all of our objects that make up our world.  Must
//      use the corresponding Shaders, VAOs, and set uniforms as appropriate.
//
////////////////////////////////////////////////////////////////////////////////
void renderScene( glm::mat4 viewMtx, glm::mat4 projMtx )  {
    // use our lighting shader program
    lightingShader->useProgram();

    //// BEGIN DRAWING THE GROUND PLANE ////
    // draw the ground plane
    glm::mat4 groundModelMtx = glm::scale( glm::mat4(1.0f), glm::vec3(55.0f, 1.0f, 55.0f));
    computeAndSendMatrixUniforms(groundModelMtx, viewMtx, projMtx);

    glm::vec3 groundColor(0.3f, 0.8f, 0.2f);
    glUniform3fv(lightingShaderUniforms.materialColor, 1, &groundColor[0]);

    glBindVertexArray(groundVAO);
    glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, (void*)0);
    //// END DRAWING THE GROUND PLANE ////

    //// BEGIN DRAWING THE BUILDINGS ////
    for( const BuildingData& currentBuilding : buildings ) {
        computeAndSendMatrixUniforms(currentBuilding.modelMatrix, viewMtx, projMtx);

        glUniform3fv(lightingShaderUniforms.materialColor, 1, &currentBuilding.color[0]);

        CSCI441::drawSolidCube(1.0);
    }
    //// END DRAWING THE BUILDINGS ////

    //// BEGIN DRAWING THE PLANE ////
    glm::mat4 modelMtx(1.0f);
    // we are going to cheat and use our look at point to place our plane so it is always in view
    modelMtx = glm::translate( modelMtx, camPos+camDir );
    // rotate the plane with our camera theta direction (we need to rotate the opposite direction so we always look at the back)
    modelMtx = glm::rotate( modelMtx, -camAngles.x, CSCI441::Y_AXIS );
    // rotate the plane with our camera phi direction
    modelMtx = glm::rotate( modelMtx,  camAngles.y, CSCI441::X_AXIS );
    // draw our plane now
    drawPlane( modelMtx, viewMtx, projMtx );
    //// END DRAWING THE PLANE ////
}

// updateScene() ////////////////////////////////////////////////////////////////
//
//  Responsible for updating any of the attributes/parameters for any of our
//      world objects.  This includes any objects that should be animated AND
//      updating our camera.
//
////////////////////////////////////////////////////////////////////////////////
void updateScene() {
    // go forward
    if( keys[GLFW_KEY_SPACE] ) {
        camPos += camDir * cameraSpeed.x;

        // rotate the propeller to make the plane fly!
        propAngle += M_PI/16.0f;
        if( propAngle > 2*M_PI ) propAngle -= 2*M_PI;
    }
    // go backward
    if( keys[GLFW_KEY_X] ) {
        camPos -= camDir * cameraSpeed.x;

        // rotate the propeller to make the plane fly!
        propAngle -= M_PI/16.0f;
        if( propAngle < 0 ) propAngle += 2*M_PI;
    }
    // turn right
    if( keys[GLFW_KEY_D] ) {
        camAngles.x += cameraSpeed.y;
        updateCameraDirection();
    }
    // turn left
    if( keys[GLFW_KEY_A] ) {
        camAngles.x -= cameraSpeed.y;
        updateCameraDirection();
    }
    // pitch up
    if( keys[GLFW_KEY_W] ) {
        camAngles.y += cameraSpeed.y;
        updateCameraDirection();
    }
    // pitch down
    if( keys[GLFW_KEY_S] ) {
        camAngles.y -= cameraSpeed.y;
        updateCameraDirection();
    }
}

//*************************************************************************************
//
// Setup Functions

// generateEnvironmentDL() /////////////////////////////////////////////////////
//
//  This function creates all the static scenery in our world.
//
////////////////////////////////////////////////////////////////////////////////
void generateEnvironmentDL() {
    const GLint GRID_WIDTH = 100;
    const GLint GRID_LENGTH = 100;
    const GLfloat GRID_SPACING = 1.1f;

    //everything's on a grid.
    for( int i = 0; i < GRID_WIDTH - 1; i++) {
        for( int j = 0; j < GRID_LENGTH - 1; j++) {
            //don't just draw a building ANYWHERE.
            if( i % 2 && j % 2 && getRand() < 0.4f ) {
                // translate to spot
                glm::mat4 modelMtx = glm::translate( glm::mat4(1.0), glm::vec3(((GLfloat)i - GRID_WIDTH / 2.0f) * GRID_SPACING, 0.0f, ((GLfloat)j - GRID_LENGTH / 2.0f) * GRID_SPACING) );
                // compute random height
                GLdouble height = powf(getRand(), 2.5)*10 + 1;  //center buildings are bigger!
                // translate up to grid plus make them float slightly to avoid depth fighting/aliasing
                modelMtx = glm::translate( modelMtx, glm::vec3(0, height/2.0f + 0.1, 0) );
                // scale to building size
                modelMtx = glm::scale( modelMtx, glm::vec3(1, height, 1) );

                // compute random color
                glm::vec3 color( getRand(), getRand(), getRand() );

                // store building properties
                BuildingData currentBuilding;
                currentBuilding.modelMatrix = modelMtx;
                currentBuilding.color = color;
                buildings.emplace_back( currentBuilding );
            }
        }
    }
}

// setupGLFW() //////////////////////////////////////////////////////////////////
//
//      Used to setup everything GLFW related.  This includes the OpenGL context
//	and our window.
//
////////////////////////////////////////////////////////////////////////////////
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

    glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );					// request forward compatible OpenGL context
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );	    // request OpenGL Core Profile context
	glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );	                // request OpenGL v4.X
	glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 1 );	                // request OpenGL vX.1
	glfwWindowHint( GLFW_RESIZABLE, GLFW_FALSE );		                    // do not allow our window to be able to be resized
	glfwWindowHint( GLFW_DOUBLEBUFFER, GLFW_TRUE );                         // request double buffering

	// create a window for a given size, with a given title
	GLFWwindow *window = glfwCreateWindow( WINDOW_WIDTH, WINDOW_HEIGHT, "Lab05 - Flight Simulator v0.41 alpha", nullptr, nullptr );
	if( !window ) {						// if the window could not be created, NULL is returned
		fprintf( stderr, "[ERROR]: GLFW Window could not be created\n" );
		glfwTerminate();
		exit( EXIT_FAILURE );
	} else {
		fprintf( stdout, "[INFO]: GLFW Window created\n" );
	}

	glfwMakeContextCurrent(window);		                                    // make the created window the current window
	glfwSwapInterval(1);				     	                    // update our screen after at least 1 screen refresh

	glfwSetKeyCallback( window, keyboard_callback );						// set our keyboard callback function
	glfwSetCursorPosCallback( window, cursor_callback );					// set our cursor position callback function
	glfwSetMouseButtonCallback( window, mouse_button_callback );	        // set our mouse button callback function

	return window;						                                    // return the window that was created
}

// setupOpenGL() ///////////////////////////////////////////////////////////////
//
//      Used to setup everything OpenGL related.
//
////////////////////////////////////////////////////////////////////////////////
void setupOpenGL() {
    glEnable( GL_DEPTH_TEST );					                            // enable depth testing
    glDepthFunc( GL_LESS );							                        // use less than depth test

    glEnable(GL_BLEND);									                    // enable blending
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	                    // use one minus blending equation

    glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );	        // clear the frame buffer to black
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
    lightingShader = new CSCI441::ShaderProgram( "shaders/lab05.v.glsl", "shaders/lab05.f.glsl" );
    lightingShaderUniforms.mvpMatrix      = lightingShader->getUniformLocation("mvpMatrix");
    lightingShaderUniforms.modelMatrix      = lightingShader->getUniformLocation("modelMatrix");
    // TODO #3 assign the uniform and attribute locations
    lightingShaderUniforms.materialColor  = lightingShader->getUniformLocation("materialColor");
    lightingShaderUniforms.lightDir_uniform_location = lightingShader->getUniformLocation("lightDirection");
    lightingShaderUniforms.diffLightColor_uniform_location = lightingShader->getUniformLocation("diffuseLightColor");
    lightingShaderUniforms.specLightColor_uniform_location = lightingShader->getUniformLocation("specularLightColor");
    lightingShaderUniforms.ambLightColor_uniform_location = lightingShader->getUniformLocation("ambientLightColor");

    lightingShaderUniforms.spotLightDir_uniform_location = lightingShader->getUniformLocation("spotLightDirection");
    lightingShaderUniforms.spotLightPos_uniform_loc = lightingShader->getUniformLocation("spotLightPosition");
    lightingShaderUniforms.diffSpotLightColor_uniform_location = lightingShader->getUniformLocation("diffuseSpotLightColor");
    lightingShaderUniforms.specSpotLightColor_uniform_location = lightingShader->getUniformLocation("specularSpotLightColor");
    lightingShaderUniforms.ambSpotLightColor_uniform_location = lightingShader->getUniformLocation("ambientSpotLightColor");

    lightingShaderUniforms.normalMtx = lightingShader->getUniformLocation("normalMatrix");
    lightingShaderUniforms.eyePos        = lightingShader->getUniformLocation("eyePos");
    lightingShaderUniforms.spotLightTheta        = lightingShader->getUniformLocation("spotLightTheta");

    lightingShaderAttributes.vPos         = lightingShader->getAttributeLocation("vPos");
    lightingShaderAttributes.vertexNormal_attr_location = lightingShader->getAttributeLocation("vertexNormal");
    // TODO #10A get the location of our ModelViewProjection uniform and set it to the global variable mvp_uniform_location
}

// setupBuffers() //////////////////////////////////////////////////////////////
//
//      Create our VAOs.  Send vertex data to the GPU to be stored.
//
////////////////////////////////////////////////////////////////////////////////
void setupBuffers() {
    // TODO #7 expand our struct to store vertex normals
    struct VertexNormal {
        GLfloat x, y, z;
        GLfloat xNorm, yNorm, zNorm;
    };

    // TODO #8 specify the specific vertex normal values
    VertexNormal groundQuad[4] = {
            {-1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f},
            { 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f},
            {-1.0f, 0.0f,  1.0f, 0.0f, 1.0f, 0.0f},
            { 1.0f, 0.0f,  1.0f, 0.0f, 1.0f, 0.0f}
    };

    GLushort indices[4] = {0,1,2,3};

    glGenVertexArrays(1, &groundVAO);
    glBindVertexArray(groundVAO);

    GLuint vbods[2];       // 0 - VBO, 1 - IBO
    glGenBuffers(2, vbods);
    glBindBuffer(GL_ARRAY_BUFFER, vbods[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(groundQuad), groundQuad, GL_STATIC_DRAW);

    glEnableVertexAttribArray(lightingShaderAttributes.vPos);
    glVertexAttribPointer(lightingShaderAttributes.vPos, 3, GL_FLOAT, GL_FALSE, sizeof(VertexNormal), (void*)0);

    // TODO #9 enable the vertex normal attribute and specify the pointer to the data
    glEnableVertexAttribArray(lightingShaderAttributes.vertexNormal_attr_location);
    glVertexAttribPointer(lightingShaderAttributes.vertexNormal_attr_location, 3, GL_FLOAT, GL_FALSE, sizeof(VertexNormal), (void*)(3*sizeof(GLfloat)));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbods[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
}

// setupScene() /////////////////////////////////////////////////////////////////
//
//	Setup everything specific to our scene.  in this case,
//	position our camera
//
////////////////////////////////////////////////////////////////////////////////
void setupScene() {
    leftMouseDown = false;
    mousePosition = glm::vec2(-9999.0f, -9999.0f);

	// give the camera a scenic starting point.
	camPos = glm::vec3(60.0f, 40.0f, 30.0f);
	camAngles = glm::vec3( -M_PI / 3.0f, M_PI / 2.8f, 1.0f);
	cameraSpeed = glm::vec2(0.25f, 0.02f);
	updateCameraDirection();

	propAngle = 0.0f;

    srand( time(nullptr) );                 // seed our random number generator
    generateEnvironmentDL();                // create our environment display list

    lightingShader->useProgram();           // use our lighting shader program so when
                                            // assign uniforms, they get sent to this shader

    // TODO #4 set the light direction and color
    glm::vec3 lightDir = {-1, -1, -1};
    glm::vec3 diffLightColor = {0.5, 0.5, 0.5};
    glm::vec3 specLightColor = {0.2, 0.2, 0.2};
    glm::vec3 ambLightColor = {0.5, 0.5, 0.5};
    glUniform3fv(lightingShaderUniforms.lightDir_uniform_location, 1, &lightDir[0]);
    glUniform3fv(lightingShaderUniforms.diffLightColor_uniform_location, 1, &diffLightColor[0]);
    glUniform3fv(lightingShaderUniforms.specLightColor_uniform_location, 1, &specLightColor[0]);
    glUniform3fv(lightingShaderUniforms.ambLightColor_uniform_location, 1, &ambLightColor[0]);

    glm::vec3 spotLightDir = {-1, -1, -1};
    glm::vec3 lightPos = {0, 0, 0};
    glm::vec3 diffSpotLightColor = {1, 0, 0};
    glm::vec3 specSpotLightColor = {1, 0, 0};
    glm::vec3 ambSpotLightColor = {0.5, 0, 0};
    glUniform1f(lightingShaderUniforms.spotLightTheta, 0.1f);
    glUniform3fv(lightingShaderUniforms.spotLightDir_uniform_location, 1, &spotLightDir[0]);
    glUniform3fv(lightingShaderUniforms.spotLightPos_uniform_loc, 1, &lightPos[0]);
    glUniform3fv(lightingShaderUniforms.diffSpotLightColor_uniform_location, 1, &diffSpotLightColor[0]);
    glUniform3fv(lightingShaderUniforms.specSpotLightColor_uniform_location, 1, &specSpotLightColor[0]);
    glUniform3fv(lightingShaderUniforms.ambSpotLightColor_uniform_location, 1, &ambSpotLightColor[0]);
}

///*************************************************************************************
//
// Our main function

// main() //////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
int main() {
    // GLFW sets up our OpenGL context so must be done first
    GLFWwindow *window = setupGLFW();	                    // initialize all of the GLFW specific information related to OpenGL and our window
    setupOpenGL();										    // initialize all of the OpenGL specific information
    setupGLEW();											// initialize all of the GLEW specific information

    CSCI441::OpenGLUtils::printOpenGLInfo();

    setupShaders();                                         // load our shader program into memory
    setupBuffers();
    setupScene();

    // TODO #5 connect the CSCI441 objects library to our shader attribute inputs
    // needed to connect our 3D Object Library to our shader
    CSCI441::setVertexAttributeLocations( lightingShaderAttributes.vPos, lightingShaderAttributes.vertexNormal_attr_location);


	//  This is our draw loop - all rendering is done here.  We use a loop to keep the window open
	//	until the user decides to close the window and quit the program.  Without a loop, the
	//	window will display once and then the program exits.
	while( !glfwWindowShouldClose(window) ) {	// check if the window was instructed to be closed
        glDrawBuffer( GL_BACK );				// work with our back frame buffer
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );	// clear the current color contents and depth buffer in the window

		// update the projection matrix based on the window size
		// the GL_PROJECTION matrix governs properties of the view coordinates;
		// i.e. what gets seen - use a perspective projection that ranges
		// with a FOV of 45 degrees, for our current aspect ratio, and Z ranges from [0.001, 1000].
		glm::mat4 projMtx = glm::perspective( 45.0f, (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.001f, 10000.0f );

		// Get the size of our framebuffer.  Ideally this should be the same dimensions as our window, but
		// when using a Retina display the actual window can be larger than the requested window.  Therefore
		// query what the actual size of the window we are rendering to is.
		GLint framebufferWidth, framebufferHeight;
		glfwGetFramebufferSize( window, &framebufferWidth, &framebufferHeight );

		// update the viewport - tell OpenGL we want to render to the whole window
		glViewport( 0, 0, framebufferWidth, framebufferHeight );

		// set up our look at matrix to position our camera
		glm::mat4 viewMtx = glm::lookAt( camPos, camPos + camDir, glm::vec3(  0,  1,  0 ) );

        glUniform3fv(lightingShaderUniforms.eyePos, 1, &camPos[0]);

		renderScene( viewMtx, projMtx );					// draw everything to the window

		glfwSwapBuffers(window);                            // flush the OpenGL commands and make sure they get rendered!
		glfwPollEvents();				                    // check for any events and signal to redraw screen

        updateScene();
	}

	fprintf( stdout, "[INFO]: Shutting down.......\n" );
	fprintf( stdout, "[INFO]: ...freeing memory...\n" );

    delete lightingShader;                                  // delete our shader program
    glDeleteBuffers(1, &groundVAO);                         // delete our ground VAO
    CSCI441::deleteObjectVBOs();                            // delete our library VBOs
    CSCI441::deleteObjectVAOs();                            // delete our library VAOs

    fprintf( stdout, "[INFO]: ...closing GLFW.....\n" );

    glfwDestroyWindow( window );                            // clean up and close our window
    glfwTerminate();						                // shut down GLFW to clean up our context

    fprintf( stdout, "[INFO]: ............complete\n" );
    fprintf( stdout, "[INFO]: Goodbye\n" );

	return EXIT_SUCCESS;				                    // exit our program successfully!
}
