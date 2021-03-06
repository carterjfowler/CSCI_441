/*
 *  CSCI 441, Computer Graphics, Fall 2020
 *
 *  Project: lab10
 *  File: main.cpp
 *
 *  Description:
 *      This file contains the start to render billboarded sprites.
 *
 *  Author: Dr. Paone, Colorado School of Mines, 2020
 *
 */

//***********************************************************************************************************************************************************
//
// Library includes

#include <GL/glew.h>                    // define our OpenGL extensions
#include <GLFW/glfw3.h>			        // include GLFW framework header

#include <glm/glm.hpp>                  // include GLM libraries
#include <glm/gtc/matrix_transform.hpp> // and matrix functions

#include <cstdio>				        // for printf functionality
#include <cstdlib>				        // for exit functionality

#include <CSCI441/materials.hpp>        // our pre-defined material properties
#include <CSCI441/OpenGLUtils.hpp>      // prints OpenGL information
#include <CSCI441/objects.hpp>          // draws 3D objects
#include <CSCI441/ShaderProgram.hpp>    // wrapper class for GLSL shader programs
#include <CSCI441/TextureUtils.hpp>     // convenience for loading textures

//***********************************************************************************************************************************************************
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
    glm::vec3 eyePos;                   // camera position
    glm::vec3 lookAtPoint;              // location of our object of interest to view
    glm::vec3 upVector;                 // the upVector of our camera
} arcballCam;

// all drawing information
const struct VAO_IDS {
    GLuint PARTICLE_SYSTEM = 0;
} VAOS;
const GLuint NUM_VAOS = 1;
GLuint vaos[NUM_VAOS];                  // an array of our VAO descriptors
GLuint vbos[NUM_VAOS];                  // an array of our VBO descriptors
GLuint ibos[NUM_VAOS];                  // an array of our IBO descriptors

// point sprite information
const GLuint NUM_SPRITES = 2;          // the number of sprites to draw
const GLfloat MAX_BOX_SIZE = 10;        // our sprites exist within a box of this size
glm::vec3* spriteLocations = nullptr;   // the (x,y,z) location of each sprite
GLushort* spriteIndices = nullptr;      // the order to draw the sprites in
GLfloat* distances = nullptr;           // will be used to store the distance to the camera
GLuint spriteTextureHandle;             // the texture to apply to the sprite
GLfloat snowglobeAngle;                 // rotates all of our snowflakes

// Billboard shader program
CSCI441::ShaderProgram *billboardShaderProgram = nullptr;
struct BillboardShaderProgramUniforms {
    GLint mvMatrix;                     // the ModelView Matrix to apply
    GLint projMatrix;                   // the Projection Matrix to apply
    GLint image;                        // the texture to bind
} billboardShaderProgramUniforms;
struct BillboardShaderProgramAttributes {
    GLint vPos;                         // the vertex position
} billboardShaderProgramAttributes;

//***********************************************************************************************************************************************************
//
// Helper Functions

// updateCameraDirection() /////////////////////////////////////////////////////////////////////////////
/// \desc
/// This function updates the camera's position in cartesian coordinates based
///  on its position in spherical coordinates. Should be called every time
///  cameraAngles is updated.
///
// /////////////////////////////////////////////////////////////////////////////
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

// computeAndSendTransformationMatrices() //////////////////////////////////////////////////////////////////////////////
/// \desc
/// This function sends the matrix uniforms to a given shader location.  Precomputes the ModelView Matrix
/// \param modelMatrix - current Model Matrix
/// \param viewMatrix - current View Matrix
/// \param projectionMatrix - current Projection Matrix
/// \param mvMtxLocation - location within currently bound shader to send the ModelView Matrix
/// \param projMtxLocation - location within currently bound shader to send the Projection Matrix
// //////////////////////////////////////////////////////////////////////////////
void computeAndSendTransformationMatrices(glm::mat4 modelMatrix, glm::mat4 viewMatrix, glm::mat4 projectionMatrix,
                                          GLint mvMtxLocation, GLint projMtxLocation) {
    glm::mat4 mvMatrix = viewMatrix * modelMatrix;

    glUniformMatrix4fv(mvMtxLocation, 1, GL_FALSE, &mvMatrix[0][0]);
    glUniformMatrix4fv(projMtxLocation, 1, GL_FALSE, &projectionMatrix[0][0]);
}

// randNumber() /////////////////////////////////////////////////////////////////////////////
/// \dexc generates a random float between [-max, max]
/// \param max - lower & upper bound to generate value between
/// \return float within range [-max, max]
// //////////////////////////////////////////////////////////////////////////////
GLfloat randNumber( GLfloat max ) {
    return rand() / (GLfloat)RAND_MAX * max * 2.0 - max;
}

//***********************************************************************************************************************************************************
//
// Event Callbacks

// error_callback() /////////////////////////////////////////////////////////////////////////////
/// \desc
///		We will register this function as GLFW's error callback.
///	When an error within GLFW occurs, GLFW will tell us by calling
///	this function.  We can then print this info to the terminal to
///	alert the user.
///
// /////////////////////////////////////////////////////////////////////////////
static void error_callback(int error, const char* description) {
    fprintf(stderr, "[ERROR]: (%d) %s\n", error, description);
}

// key_callback() /////////////////////////////////////////////////////////////////////////////
/// \desc
///		We will register this function as GLFW's keypress callback.
///	Responds to key presses and key releases
///
// /////////////////////////////////////////////////////////////////////////////
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if(action == GLFW_PRESS) {
        switch( key ) {
            case GLFW_KEY_Q:
            case GLFW_KEY_ESCAPE:
                glfwSetWindowShouldClose( window, GLFW_TRUE );
                break;

            default: break;
        }
    }
}

// mouse_button_callback() /////////////////////////////////////////////////////////////////////////////
/// \desc
///		We will register this function as GLFW's mouse button callback.
///	Responds to mouse button presses and mouse button releases.  Keeps track if
///	the control key was pressed when a left mouse click occurs to allow
///	zooming of our arcball camera.
///
// /////////////////////////////////////////////////////////////////////////////
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

// cursor_callback() /////////////////////////////////////////////////////////////////////////////
/// \desc
///		We will register this function as GLFW's cursor movement callback.
///	Responds to mouse movement.  When active motion is used with the left
///	mouse button an arcball camera model is followed.
///
// /////////////////////////////////////////////////////////////////////////////
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

// scroll_callback() /////////////////////////////////////////////////////////////////////////////
/// \desc
///		We will register this function as GLFW's scroll wheel callback.
///	Responds to movement of the scroll where.  Allows zooming of the arcball
///	camera.
///
// /////////////////////////////////////////////////////////////////////////////
static void scroll_callback(GLFWwindow* window, double xOffset, double yOffset ) {
    double totChgSq = yOffset;
    arcballCam.cameraAngles.z += totChgSq*0.2f;
    updateCameraDirection();
}

//***********************************************************************************************************************************************************
//
// Setup Functions

// setupGLFW() /////////////////////////////////////////////////////////////////////////////
/// \desc
///		Used to setup everything GLFW related.  This includes the OpenGL context
///	and our window.
/// \return window - the window associated with the new context
// /////////////////////////////////////////////////////////////////////////////
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
    GLFWwindow *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Lab10: Geometry Shaders", nullptr, nullptr );
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

// setupOpenGL() /////////////////////////////////////////////////////////////////////////////
/// \desc
///      Used to setup everything OpenGL related.
///
// /////////////////////////////////////////////////////////////////////////////
void setupOpenGL() {
    glEnable( GL_DEPTH_TEST );					                                // enable depth testing
    glDepthFunc( GL_LESS );							                            // use less than depth test

    glEnable(GL_BLEND);									                        // enable blending
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	                        // use one minus blending equation

    glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );	            // clear the frame buffer to black

    glPointSize( 4.0f );                                                    // make our points bigger (if supported)
}

// setupGLEW() /////////////////////////////////////////////////////////////////////////////
/// \desc
///      Used to initialize GLEW
///
// /////////////////////////////////////////////////////////////////////////////
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

// setupShaders() /////////////////////////////////////////////////////////////////////////////
/// \desc
///      Registers our Shader Programs and query locations
///          of uniform/attribute inputs
///
// /////////////////////////////////////////////////////////////////////////////
void setupShaders() {
    // LOOKHERE #1
    billboardShaderProgram = new CSCI441::ShaderProgram( "shaders/billboardQuadShader.v.glsl",
                                                         "shaders/billboardQuadShader.g.glsl",
                                                         "shaders/billboardQuadShader.f.glsl" );
    billboardShaderProgramUniforms.mvMatrix            = billboardShaderProgram->getUniformLocation( "mvMatrix");
    billboardShaderProgramUniforms.projMatrix          = billboardShaderProgram->getUniformLocation( "projMatrix");
    billboardShaderProgramUniforms.image               = billboardShaderProgram->getUniformLocation( "image");
    billboardShaderProgramAttributes.vPos              = billboardShaderProgram->getAttributeLocation( "vPos");

    billboardShaderProgram->useProgram();
    glUniform1i(billboardShaderProgramUniforms.image, 0);
}

// setupBuffers() /////////////////////////////////////////////////////////////////////////////
/// \desc
///      Create our VAOs & VBOs. Send vertex assets to the GPU for future rendering
///
// /////////////////////////////////////////////////////////////////////////////
void setupBuffers() {

    // generate ALL VAOs, VBOs, IBOs at once
    glGenVertexArrays( NUM_VAOS, vaos );
    glGenBuffers( NUM_VAOS, vbos );
    glGenBuffers( NUM_VAOS, ibos );

    // --------------------------------------------------------------------------------------------------
    // LOOKHERE #2 - generate sprites

    spriteLocations = (glm::vec3*)malloc(sizeof(glm::vec3) * NUM_SPRITES);
    spriteIndices = (GLushort*)malloc(sizeof(GLushort) * NUM_SPRITES);
    distances = (GLfloat*)malloc(sizeof(GLfloat) * NUM_SPRITES);
    for( int i = 0; i < NUM_SPRITES; i++ ) {
        glm::vec3 pos( randNumber(MAX_BOX_SIZE), randNumber(MAX_BOX_SIZE), randNumber(MAX_BOX_SIZE) );
        spriteLocations[i] = pos;
        spriteIndices[i] = i;
    }

    glBindVertexArray( vaos[VAOS.PARTICLE_SYSTEM] );

    glBindBuffer( GL_ARRAY_BUFFER, vbos[VAOS.PARTICLE_SYSTEM] );
    glBufferData( GL_ARRAY_BUFFER, NUM_SPRITES * sizeof(glm::vec3), spriteLocations, GL_STATIC_DRAW );

    glEnableVertexAttribArray( billboardShaderProgramAttributes.vPos );
    glVertexAttribPointer( billboardShaderProgramAttributes.vPos, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0 );

    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ibos[VAOS.PARTICLE_SYSTEM] );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, NUM_SPRITES * sizeof(GLushort), spriteIndices, GL_STATIC_DRAW );

    fprintf( stdout, "[INFO]: point sprites read in with VAO/VBO/IBO %d/%d/%d\n", vaos[VAOS.PARTICLE_SYSTEM], vbos[VAOS.PARTICLE_SYSTEM], ibos[VAOS.PARTICLE_SYSTEM] );
}

// setupTextures() /////////////////////////////////////////////////////////////////////////////
/// \desc
///      Register all of our textures with the GPU
///
// /////////////////////////////////////////////////////////////////////////////
void setupTextures() {
    // LOOKHERE #4
    spriteTextureHandle = CSCI441::TextureUtils::loadAndRegisterTexture("assets/textures/spark.png");
}

// setupScene() /////////////////////////////////////////////////////////////////////////////
/// \desc
///      Initialize all of our scene information here
///
// /////////////////////////////////////////////////////////////////////////////
void setupScene() {
    // set up mouse info
    leftMouseDown = GL_FALSE;
    controlDown = GL_FALSE;
    mousePosition = glm::vec2( -9999.0f, -9999.0f );

    // set up camera info
    arcballCam.cameraAngles   = glm::vec3( 3.52f, 1.9f, 25.0f );
    arcballCam.camDir         = glm::vec3(-1.0f, -1.0f, -1.0f);
    arcballCam.lookAtPoint    = glm::vec3(0.0f, 0.0f, 0.0f);
    arcballCam.upVector       = glm::vec3(    0.0f,  1.0f,  0.0f );
    updateCameraDirection();

    // setup snowglobe
    snowglobeAngle = 0.0f;
}

// initialize() /////////////////////////////////////////////////////////////////////////////
/// \desc
///      Create our OpenGL context,
///          load all information to the GPU,
///          initialize scene information
/// \return window - the window that was created when the OpenGL context was created
// /////////////////////////////////////////////////////////////////////////////
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

//***********************************************************************************************************************************************************
//
// Cleanup Functions

// cleanupShader() /////////////////////////////////////////////////////////////////////////////
/// \desc
///      Delete shaders off of the GPU
///
// /////////////////////////////////////////////////////////////////////////////
void cleanupShaders() {
    fprintf( stdout, "[INFO]: ...deleting shaders.\n" );

    delete billboardShaderProgram;
}

// cleanupBuffers() /////////////////////////////////////////////////////////////////////////////
/// \desc
///      Delete VAOs and VBOs off of the GPU.
///      Remove any buffers from CPU RAM as well.
///
// /////////////////////////////////////////////////////////////////////////////
void cleanupBuffers() {
    fprintf( stdout, "[INFO]: ...deleting IBOs....\n" );

    glDeleteBuffers( NUM_VAOS, ibos );

    fprintf( stdout, "[INFO]: ...deleting VBOs....\n" );

    glDeleteBuffers( NUM_VAOS, vbos );
    CSCI441::deleteObjectVBOs();

    fprintf( stdout, "[INFO]: ...deleting VAOs....\n" );

    glDeleteVertexArrays( NUM_VAOS, vaos );
    CSCI441::deleteObjectVAOs();

    free(spriteLocations);
    free(spriteIndices);
    free(distances);
}

// cleanupTextures() /////////////////////////////////////////////////////////////////////////////
/// \desc
///      Delete textures off of the GPU
///
// /////////////////////////////////////////////////////////////////////////////
void cleanupTextures() {
    fprintf( stdout, "[INFO]: ...deleting textures\n" );

    glDeleteTextures(1, &spriteTextureHandle);
}

// shutdown() /////////////////////////////////////////////////////////////////////////////
/// \desc
///      Free all memory on the CPU/GPU and close our OpenGL context
///
// /////////////////////////////////////////////////////////////////////////////
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

//***********************************************************************************************************************************************************
//
// Rendering / Drawing Functions - this is where the magic happens!

// renderScene() /////////////////////////////////////////////////////////////////////////////
/// \desc
///		This method will contain all of the objects to be drawn.
/// \param viewMatrix - View Matrix for the Camera this scene should be rendered to
/// \param projectionMatrix - Projection Matrix for the Camera this scene should be rendered to
// /////////////////////////////////////////////////////////////////////////////
void renderScene( glm::mat4 viewMatrix, glm::mat4 projectionMatrix ) {
    glm::mat4 modelMatrix = glm::mat4( 1.0f );

    // LOOKHERE #3
    billboardShaderProgram->useProgram();
    modelMatrix = glm::rotate(glm::mat4(1.0f), snowglobeAngle, CSCI441::Y_AXIS);
    computeAndSendTransformationMatrices( modelMatrix, viewMatrix, projectionMatrix,
                                          billboardShaderProgramUniforms.mvMatrix, billboardShaderProgramUniforms.projMatrix);
    glBindVertexArray( vaos[VAOS.PARTICLE_SYSTEM] );
    glBindTexture(GL_TEXTURE_2D, spriteTextureHandle);

    // TODO #1
    glm::vec3 normViewVector = glm::normalize(arcballCam.lookAtPoint - arcballCam.eyePos);

    for (int i = 0; i < NUM_SPRITES; ++i) {
        glm::vec3 currentSprite = spriteLocations[ spriteIndices[i] ];
        glm::vec4 worldSprite = modelMatrix * glm::vec4(currentSprite, 1);
        glm::vec4 ep = worldSprite - glm::vec4(arcballCam.eyePos,1);
        distances[i] = glm::dot(ep, glm::vec4(normViewVector,0));
    }

    // TODO #2
    //bubble sort the distances
    for (int i = 0; i < NUM_SPRITES; ++i) {
        for (int j = i; j < NUM_SPRITES; ++j) {
            if (distances[i] < distances[j]) {
                GLfloat distI = distances[i];
                GLushort indexI = spriteIndices[i];

                distances[i] = distances[j];
                distances[j] = distI;
                spriteIndices[i] = spriteIndices[j];
                spriteIndices[j] = indexI;
            }
        }
    }

    // TODO #3
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ibos[VAOS.PARTICLE_SYSTEM] );
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(GLushort) * NUM_SPRITES, spriteIndices);
    glDrawElements( GL_POINTS, NUM_SPRITES, GL_UNSIGNED_SHORT, (void*)0 );
}

// updateScene() /////////////////////////////////////////////////////////////////////////////
/// \desc
///      Update all of our scene objects - perform animation here
///
// /////////////////////////////////////////////////////////////////////////////
void updateScene() {
    snowglobeAngle += 0.01f;
    if(snowglobeAngle >= 6.28f) {
        snowglobeAngle -= 6.28f;
    }
}

// run() /////////////////////////////////////////////////////////////////////////////
/// \desc
///      Runs our draw loop and renders/updates our scene
/// \param window - window to render the scene to
// /////////////////////////////////////////////////////////////////////////////
void run(GLFWwindow* window) {
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
        arcballCam.eyePos = arcballCam.lookAtPoint + arcballCam.camDir * arcballCam.cameraAngles.z;
        glm::mat4 viewMatrix = glm::lookAt( arcballCam.eyePos,
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

//**********************************************************************************************************************************************************
//
// Our main function

// main() /////////////////////////////////////////////////////////////////////////////
///
// /////////////////////////////////////////////////////////////////////////////
int main() {
    GLFWwindow *window = initialize();                  // create OpenGL context and setup EVERYTHING for our program
    run(window);                                        // enter our draw loop and run our program
    shutdown(window);                                   // free up all the memory used and close OpenGL context
    return EXIT_SUCCESS;				                // exit our program successfully!
}
