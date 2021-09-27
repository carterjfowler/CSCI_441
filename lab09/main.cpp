/*
 *  CSCI 441, Computer Graphics, Fall 2020
 *
 *  Project: lab09
 *  File: main.cpp
 *
 *  Description:
 *      This file contains the start to render a Bezier curve.
 *
 *  Author: Dr. Paone, Colorado School of Mines, 2020
 *
 */

///***********************************************************************************************************************************************************
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

///***********************************************************************************************************************************************************
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

GLuint lightType;                       // type of the light - 0 point 1 directional 2 spot

// all drawing information
const struct VAO_IDS {
    GLuint CAGE = 0, PATCH = 1;         // unique idenfitiers for each VAO
} VAOS;
const GLuint NUM_VAOS = 2;
GLuint vaos[NUM_VAOS];                  // an array of our VAO descriptors
GLuint vbos[NUM_VAOS];                  // an array of our VBO descriptors
GLuint ibos[NUM_VAOS];                  // an array of our IBO descriptors

// Bezier Patch Information
const GLuint POINTS_PER_PATCH = 16;     // the number of control points per patch
GLuint numControlPoints;                // number of control points in the patch system
glm::vec3 *controlPoints = nullptr;     // control points array
GLuint numSurfaces;                     // the total number of surfaces to draw
GLboolean drawPoints, drawCage;         // flags to draw the control points and/or cage

// gourad with phong illumination shader program
CSCI441::ShaderProgram *gouradShaderProgram = nullptr;
struct GouradShaderProgramUniforms {
    GLint mvpMatrix;                    // the MVP Matrix to apply
    GLint modelMatrix;                  // model matrix
    GLint normalMtx;                    // normal matrix
    GLint eyePos;                       // camera position
    GLint lightPos;                     // light position - used for point/spot
    GLint lightDir;                     // light direction - used for directional/spot
    GLint lightCutoff;                  // light cone angle - used for spot
    GLint lightColor;                   // color of the light
    GLint lightType;                    // type of the light - 0 point 1 directional 2 spot
    GLint materialDiffColor;            // material diffuse color
    GLint materialSpecColor;            // material specular color
    GLint materialShininess;            // material shininess factor
    GLint materialAmbColor;             // material ambient color
} gouradShaderProgramUniforms;
struct GouradShaderProgramAttributes {
    GLint vPos;                         // position of our vertex
    GLint vNormal;                      // normal for the vertex
} gouradShaderProgramAttributes;

// flat shader program
CSCI441::ShaderProgram *flatShaderProgram = nullptr;
struct FlatShaderProgramUniforms {
    GLint mvpMatrix;                    // the MVP Matrix to apply
    GLint color;                        // the color to apply
} flatShaderProgramUniforms;
struct FlatShaderProgramAttributes {
    GLint vPos;                         // the vertex position
} flatShaderProgramAttributes;

// Bezier Patch shader program
CSCI441::ShaderProgram *bezierShaderProgram = nullptr;
struct BezierShaderProgramUniforms {
    GLint mvpMatrix;                    // the MVP Matrix to apply
} bezierShaderProgramUniforms;
struct BezierShaderProgramAttributes {
    GLint vPos;                         // the vertex position
} bezierShaderProgramAttributes;

///***********************************************************************************************************************************************************
//
// Helper Functions

/// updateCameraDirection() /////////////////////////////////////////////////////
///
/// This function updates the camera's position in cartesian coordinates based
///  on its position in spherical coordinates. Should be called every time
///  cameraAngles is updated.
///
////////////////////////////////////////////////////////////////////////////////
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

/// computeAndSendTransformationMatrices() //////////////////////////////////////
///
/// This function sends the matrix uniforms to a given shader location.
///
////////////////////////////////////////////////////////////////////////////////
void computeAndSendTransformationMatrices(glm::mat4 modelMatrix, glm::mat4 viewMatrix, glm::mat4 projectionMatrix,
                                          GLint mvpMtxLocation, GLint modelMtxLocation = -1, GLint normalMtxLocation = -1) {
    glm::mat4 mvpMatrix = projectionMatrix * viewMatrix * modelMatrix;
    glm::mat3 normalMatrix = glm::mat3( glm::transpose( glm::inverse(modelMatrix) ) );

    glUniformMatrix4fv(mvpMtxLocation, 1, GL_FALSE, &mvpMatrix[0][0]);
    glUniformMatrix4fv(modelMtxLocation, 1, GL_FALSE, &modelMatrix[0][0]);
    glUniformMatrix3fv(normalMtxLocation, 1, GL_FALSE, &normalMatrix[0][0]);
}

/// sendMaterialProperties() ////////////////////////////////////////////////////
///
/// This function sends the material uniforms to a given shader location.
///
////////////////////////////////////////////////////////////////////////////////
void sendMaterialProperties(CSCI441::Materials::Material material, GLint diffuseLocation, GLint specularLocation, GLint shininessLocation, GLint ambientLocation) {
    glUniform3fv(diffuseLocation, 1, &(material.diffuse[0]));
    glUniform3fv(specularLocation, 1, &(material.specular[0]));
    glUniform1f(shininessLocation, material.shininess);
    glUniform3fv(ambientLocation, 1, &(material.ambient[0]));
}

/// loadControlPointsFromFile() /////////////////////////////////////////////////
///
/// This function loads the Bezier control points from a given file.  Upon
/// completion, the parameters will store the number of points read in, the
/// number of surfaces they represent, the array of actual points, and the
/// indices to connect them in order.
///
////////////////////////////////////////////////////////////////////////////////
void loadControlPointsFromFile(const char* FILENAME, GLuint* numBezierPoints, GLuint* numBezierSurfaces, glm::vec3* &bezierPoints, GLushort* &bezierIndices) {
    FILE *file = fopen(FILENAME, "r");

    if(!file) {
        fprintf( stderr, "[ERROR]: Could not open \"%s\"\n", FILENAME );
    } else {
        fscanf( file, "%u\n", numBezierSurfaces );

        fprintf( stdout, "[INFO]: Reading in %u surfaces\n", *numBezierSurfaces );

        bezierIndices = (GLushort*)malloc( sizeof( GLushort ) * *numBezierSurfaces * POINTS_PER_PATCH );
        if(!bezierIndices) {
            fprintf( stderr, "[ERROR]: Could not allocate space for surface indices\n" );
        } else {
            for( int i = 0; i < *numBezierSurfaces; i++ ) {
                // read in the first 15 points that have a comma following
                for( int j = 0; j < POINTS_PER_PATCH-1; j++) {
                    fscanf( file, "%hu,", &bezierIndices[i*POINTS_PER_PATCH + j] );
                    bezierIndices[i*POINTS_PER_PATCH + j]--;
                }
                // read in the 16th point that has a new line following
                fscanf( file, "%hu\n", &bezierIndices[i*POINTS_PER_PATCH + POINTS_PER_PATCH-1] );
                bezierIndices[i*POINTS_PER_PATCH + POINTS_PER_PATCH-1]--;
            }
        }

        fscanf( file, "%u\n", numBezierPoints );

        fprintf( stdout, "[INFO]: Reading in %u control points\n", *numBezierPoints );

        bezierPoints = (glm::vec3*)malloc( sizeof( glm::vec3 ) * *numBezierPoints );
        if(!bezierPoints) {
            fprintf( stderr, "[ERROR]: Could not allocate space for control points\n" );
        } else {
            for( int i = 0; i < *numBezierPoints; i++ ) {
                fscanf( file, "%f,%f,%f\n", &(bezierPoints[i].x), &(bezierPoints[i].y), &(bezierPoints[i].z));
            }
        }
    }
    fclose(file);
}

///***********************************************************************************************************************************************************
//
// Event Callbacks

/// error_callback() ////////////////////////////////////////////////////////////
///
///		We will register this function as GLFW's error callback.
///	When an error within GLFW occurs, GLFW will tell us by calling
///	this function.  We can then print this info to the terminal to
///	alert the user.
///
////////////////////////////////////////////////////////////////////////////////
static void error_callback(int error, const char* description) {
    fprintf(stderr, "[ERROR]: (%d) %s\n", error, description);
}

/// key_callback() //////////////////////////////////////////////////////////////
///
///		We will register this function as GLFW's keypress callback.
///	Responds to key presses and key releases
///
////////////////////////////////////////////////////////////////////////////////
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if(action == GLFW_PRESS) {
        switch( key ) {
            case GLFW_KEY_Q:
            case GLFW_KEY_ESCAPE:
                glfwSetWindowShouldClose( window, GLFW_TRUE );
                break;

            case GLFW_KEY_C:
                drawCage = !drawCage;
                break;

            case GLFW_KEY_P:
                drawPoints = !drawPoints;
                break;

                // toggle between light types
            case GLFW_KEY_1:    // point light
            case GLFW_KEY_2:    // directional light
            case GLFW_KEY_3:    // spot light
                lightType = key - GLFW_KEY_1;
                // send the light type to the shader
                gouradShaderProgram->useProgram();
                glUniform1i(gouradShaderProgramUniforms.lightType, lightType);
                break;

            default: break;
        }
    }
}

/// mouse_button_callback() /////////////////////////////////////////////////////
///
///		We will register this function as GLFW's mouse button callback.
///	Responds to mouse button presses and mouse button releases.  Keeps track if
///	the control key was pressed when a left mouse click occurs to allow
///	zooming of our arcball camera.
///
////////////////////////////////////////////////////////////////////////////////
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

/// cursor_callback() ///////////////////////////////////////////////////////////
///
///		We will register this function as GLFW's cursor movement callback.
///	Responds to mouse movement.  When active motion is used with the left
///	mouse button an arcball camera model is followed.
///
////////////////////////////////////////////////////////////////////////////////
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

/// scroll_callback() ///////////////////////////////////////////////////////////
///
///		We will register this function as GLFW's scroll wheel callback.
///	Responds to movement of the scroll where.  Allows zooming of the arcball
///	camera.
///
////////////////////////////////////////////////////////////////////////////////
static void scroll_callback(GLFWwindow* window, double xOffset, double yOffset ) {
    double totChgSq = yOffset;
    arcballCam.cameraAngles.z += totChgSq*0.2f;
    updateCameraDirection();
}

///***********************************************************************************************************************************************************
//
// Setup Functions

/// setupGLFW() /////////////////////////////////////////////////////////////////
///
///		Used to setup everything GLFW related.  This includes the OpenGL context
///	and our window.
///
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
    glfwWindowHint( GLFW_DOUBLEBUFFER, GLFW_TRUE );                             // request double buffering

    // create a window for a given size, with a given title
    GLFWwindow *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Lab09: Bezier Patches", nullptr, nullptr );
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

/// setupOpenGL() ///////////////////////////////////////////////////////////////
///
///      Used to setup everything OpenGL related.
///
////////////////////////////////////////////////////////////////////////////////
void setupOpenGL() {
    glEnable( GL_DEPTH_TEST );					                    // enable depth testing
    glDepthFunc( GL_LESS );							                // use less than depth test

    glEnable(GL_BLEND);									            // enable blending
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	            // use one minus blending equation

    glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );	// clear the frame buffer to black

    // set the number of control points per patch
    glPatchParameteri( GL_PATCH_VERTICES, POINTS_PER_PATCH);
}

/// setupGLEW() /////////////////////////////////////////////////////////////////
///
///      Used to initialize GLEW
///
////////////////////////////////////////////////////////////////////////////////
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

/// setupShaders() //////////////////////////////////////////////////////////////
///
///      Registers our Shader Programs and query locations
///          of uniform/attribute inputs
///
////////////////////////////////////////////////////////////////////////////////
void setupShaders() {
    gouradShaderProgram = new CSCI441::ShaderProgram( "shaders/gouradShader.v.glsl", "shaders/gouradShader.f.glsl" );
    gouradShaderProgramUniforms.mvpMatrix           = gouradShaderProgram->getUniformLocation( "mvpMatrix");
    gouradShaderProgramUniforms.modelMatrix         = gouradShaderProgram->getUniformLocation("modelMatrix");
    gouradShaderProgramUniforms.normalMtx           = gouradShaderProgram->getUniformLocation("normalMtx");
    gouradShaderProgramUniforms.eyePos              = gouradShaderProgram->getUniformLocation("eyePos");
    gouradShaderProgramUniforms.lightPos            = gouradShaderProgram->getUniformLocation("lightPos");
    gouradShaderProgramUniforms.lightDir            = gouradShaderProgram->getUniformLocation("lightDir");
    gouradShaderProgramUniforms.lightCutoff         = gouradShaderProgram->getUniformLocation("lightCutoff");
    gouradShaderProgramUniforms.lightColor          = gouradShaderProgram->getUniformLocation("lightColor");
    gouradShaderProgramUniforms.lightType           = gouradShaderProgram->getUniformLocation("lightType");
    gouradShaderProgramUniforms.materialDiffColor   = gouradShaderProgram->getUniformLocation("materialDiffColor");
    gouradShaderProgramUniforms.materialSpecColor   = gouradShaderProgram->getUniformLocation("materialSpecColor");
    gouradShaderProgramUniforms.materialShininess   = gouradShaderProgram->getUniformLocation("materialShininess");
    gouradShaderProgramUniforms.materialAmbColor    = gouradShaderProgram->getUniformLocation("materialAmbColor");
    gouradShaderProgramAttributes.vPos              = gouradShaderProgram->getAttributeLocation("vPos");
    gouradShaderProgramAttributes.vNormal           = gouradShaderProgram->getAttributeLocation("vNormal");

    flatShaderProgram = new CSCI441::ShaderProgram( "shaders/flatShader.v.glsl", "shaders/flatShader.f.glsl" );
    flatShaderProgramUniforms.mvpMatrix             = flatShaderProgram->getUniformLocation("mvpMatrix");
    flatShaderProgramUniforms.color                 = flatShaderProgram->getUniformLocation("color");
    flatShaderProgramAttributes.vPos                = flatShaderProgram->getAttributeLocation("vPos");

    bezierShaderProgram = new CSCI441::ShaderProgram( "shaders/bezierPatch.v.glsl", "shaders/bezierPatch.tc.glsl",
        "shaders/bezierPatch.te.glsl","shaders/bezierPatch.f.glsl" );
    bezierShaderProgramUniforms.mvpMatrix           = bezierShaderProgram->getUniformLocation("mvpMatrix");
    bezierShaderProgramAttributes.vPos              = bezierShaderProgram->getAttributeLocation("vPos");
}

/// setupBuffers() //////////////////////////////////////////////////////////////
///
///      Create our VAOs & VBOs. Send vertex assets to the GPU for future rendering
///
////////////////////////////////////////////////////////////////////////////////
void setupBuffers() {

    // generate ALL VAOs, VBOs, IBOs at once
    glGenVertexArrays( NUM_VAOS, vaos );
    glGenBuffers( NUM_VAOS, vbos );
    glGenBuffers( NUM_VAOS, ibos );

    // ------------------------------------------------------------------------------------------------------
    // read in the control points

    GLushort* patchIndices = nullptr;
    loadControlPointsFromFile("assets/models/surface.txt", &numControlPoints, &numSurfaces, controlPoints, patchIndices);
    if(!controlPoints) {
        fprintf( stderr, "[ERROR]: Error loading control points from file\n" );
    } else {
        fprintf( stdout, "[INFO]: Read in %u points comprising %u surfaces\n", numControlPoints, numSurfaces );

        // --------------------------------------------------------------------------------------------------
        // generate cage

        glBindVertexArray( vaos[VAOS.CAGE] );

        glBindBuffer( GL_ARRAY_BUFFER, vbos[VAOS.CAGE] );
        glBufferData( GL_ARRAY_BUFFER, numControlPoints * sizeof(glm::vec3), controlPoints, GL_STATIC_DRAW );

        glEnableVertexAttribArray( flatShaderProgramAttributes.vPos );
        glVertexAttribPointer( flatShaderProgramAttributes.vPos, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0 );

        GLushort* cageIndices = (GLushort*)malloc(sizeof(GLushort)*numSurfaces*POINTS_PER_PATCH*2);
        for(int k = 0; k < numSurfaces; k++) {
            for( int i = 0; i < 4; i++ ) {
                for( int j = 0; j < 4; j++ ) {
                    cageIndices[k*(POINTS_PER_PATCH*2) + POINTS_PER_PATCH + j * 4 + i] = cageIndices[k*(POINTS_PER_PATCH*2) + i * 4 + j] = patchIndices[k*POINTS_PER_PATCH + i*4 + j];
                }
            }
        }
        glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ibos[VAOS.CAGE] );
        glBufferData( GL_ELEMENT_ARRAY_BUFFER, numSurfaces * POINTS_PER_PATCH * 2 * sizeof(GLushort), cageIndices, GL_STATIC_DRAW );

        free(cageIndices);

        fprintf( stdout, "[INFO]: surface cage read in with VAO %d\n", vaos[VAOS.CAGE] );

        // --------------------------------------------------------------------------------------------------
        // generate patch

        glBindVertexArray( vaos[VAOS.PATCH] );

        glBindBuffer( GL_ARRAY_BUFFER, vbos[VAOS.PATCH] );
        glBufferData( GL_ARRAY_BUFFER, numControlPoints * sizeof(glm::vec3), controlPoints, GL_STATIC_DRAW );

        glEnableVertexAttribArray( bezierShaderProgramAttributes.vPos );
        glVertexAttribPointer( bezierShaderProgramAttributes.vPos, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0 );

        glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ibos[VAOS.PATCH] );
        glBufferData( GL_ELEMENT_ARRAY_BUFFER, numSurfaces * POINTS_PER_PATCH * sizeof(GLushort), patchIndices, GL_STATIC_DRAW );

        free(patchIndices);

        fprintf( stdout, "[INFO]: surface control points read in with VAO %d\n", vaos[VAOS.PATCH] );
    }
}

/// setupTextures() /////////////////////////////////////////////////////////////
///
///      Register all of our textures with the GPU
///
////////////////////////////////////////////////////////////////////////////////
void setupTextures() {

}

/// setupScene() ////////////////////////////////////////////////////////////////
///
///      Initialize all of our scene information here
///
////////////////////////////////////////////////////////////////////////////////
void setupScene() {
    // set up mouse info
    leftMouseDown = GL_FALSE;
    controlDown = GL_FALSE;
    mousePosition = glm::vec2( -9999.0f, -9999.0f );

    // set up camera info
    arcballCam.cameraAngles   = glm::vec3( 3.52f, 1.9f, 15.0f );
    arcballCam.camDir         = glm::vec3(-1.0f, -1.0f, -1.0f);
    arcballCam.lookAtPoint    = glm::vec3(0.0f, 0.0f, 0.0f);
    arcballCam.upVector       = glm::vec3(    0.0f,  1.0f,  0.0f );
    updateCameraDirection();

    // set up light info
    glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
    glm::vec3 lightPos(5.0f, 15.0f, 5.0f);
    glm::vec3 lightDir(-1.0f, -3.0f, -1.0f);
    float lightCutoff = glm::cos( glm::radians(7.5f) );
    lightType = 0;
    gouradShaderProgram->useProgram();
    glUniform3fv(gouradShaderProgramUniforms.lightColor, 1, &lightColor[0]);
    glUniform3fv(gouradShaderProgramUniforms.lightPos, 1, &lightPos[0]);
    glUniform3fv(gouradShaderProgramUniforms.lightDir, 1, &lightDir[0]);
    glUniform1f(gouradShaderProgramUniforms.lightCutoff, lightCutoff);
    glUniform1i(gouradShaderProgramUniforms.lightType, lightType);

    glm::vec3 flatColor(1.0f, 1.0f, 1.0f);
    flatShaderProgram->useProgram();
    glUniform3fv(flatShaderProgramUniforms.color, 1, &flatColor[0]);

    // setup drawing info
    drawCage = GL_TRUE;
    drawPoints = GL_TRUE;
}

/// initialize() /////////////////////////////////////////////////////////////////
///
///      Create our OpenGL context,
///          load all information to the GPU,
///          initialize scene information
///
////////////////////////////////////////////////////////////////////////////////
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

///***********************************************************************************************************************************************************
//
// Cleanup Functions

/// cleanupShaders() ////////////////////////////////////////////////////////////
///
///      Delete shaders off of the GPU
///
////////////////////////////////////////////////////////////////////////////////
void cleanupShaders() {
    fprintf( stdout, "[INFO]: ...deleting shaders.\n" );

    delete gouradShaderProgram;
    delete flatShaderProgram;
    delete bezierShaderProgram;
}

/// cleanupBuffers() ////////////////////////////////////////////////////////////
///
///      Delete VAOs and VBOs off of the GPU
///
////////////////////////////////////////////////////////////////////////////////
void cleanupBuffers() {
    fprintf( stdout, "[INFO]: ...deleting IBOs....\n" );

    glDeleteBuffers( NUM_VAOS, ibos );

    fprintf( stdout, "[INFO]: ...deleting VBOs....\n" );

    glDeleteBuffers( NUM_VAOS, vbos );
    CSCI441::deleteObjectVBOs();

    fprintf( stdout, "[INFO]: ...deleting VAOs....\n" );

    glDeleteVertexArrays( NUM_VAOS, vaos );
    CSCI441::deleteObjectVAOs();

    free(controlPoints);
}

/// cleanupTextures() ///////////////////////////////////////////////////////////
///
///      Delete textures off of the GPU
///
////////////////////////////////////////////////////////////////////////////////
void cleanupTextures() {
    fprintf( stdout, "[INFO]: ...deleting textures\n" );
}

/// shutdown() ///////////////////////////////////////////////////////////////////
///
///      Free all memory on the CPU/GPU and close our OpenGL context
///
////////////////////////////////////////////////////////////////////////////////
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

///***********************************************************************************************************************************************************
//
// Rendering / Drawing Functions - this is where the magic happens!

/// renderScene() ///////////////////////////////////////////////////////////////
///
///		This method will contain all of the objects to be drawn.
///
////////////////////////////////////////////////////////////////////////////////
void renderScene( glm::mat4 viewMatrix, glm::mat4 projectionMatrix ) {
    glm::mat4 modelMatrix = glm::mat4( 1.0f );

    if(drawPoints) {
        // use the gourad shader
        gouradShaderProgram->useProgram();
        // set the eye position - needed for specular reflection
        glUniform3fv( gouradShaderProgramUniforms.eyePos, 1, &(arcballCam.eyePos[0]));

        // hook up the CSCI441 object library to our shader program - MUST be done after the shader is used and before the objects are drawn
        // if we have multiple shaders the flow would be:
        //      1) shader->useProgram()
        //      2) CSCI441::setVertexAttributeLocations()
        //      3) CSCI441::draw*()
        // but this lab only has one shader program ever in use so we are safe to assign these values just once
        //
        // OR the alternative is to ensure that all of your shader programs use the
        // same attribute locations for the vertex position, normal, and texture coordinate
        CSCI441::setVertexAttributeLocations( gouradShaderProgramAttributes.vPos,     // vertex position location
                                              gouradShaderProgramAttributes.vNormal ); // vertex normal location

        // use the ruby material
        sendMaterialProperties( CSCI441::Materials::RUBY,
                                gouradShaderProgramUniforms.materialDiffColor,
                                gouradShaderProgramUniforms.materialSpecColor,
                                gouradShaderProgramUniforms.materialShininess,
                                gouradShaderProgramUniforms.materialAmbColor );

        // draw each of the control points represented by a sphere
        for( int i = 0; i < numControlPoints; i++ ) {
            modelMatrix = glm::translate( glm::mat4( 1.0f ), controlPoints[i] );
            computeAndSendTransformationMatrices( modelMatrix, viewMatrix, projectionMatrix,
                                                  gouradShaderProgramUniforms.mvpMatrix,
                                                  gouradShaderProgramUniforms.modelMatrix,
                                                  gouradShaderProgramUniforms.normalMtx );
            CSCI441::drawSolidSphere( 0.1f, 16, 16 );
        }
    }

    if(drawCage) {
        // use the flat shader to draw lines
        flatShaderProgram->useProgram();
        modelMatrix = glm::mat4( 1.0f );
        computeAndSendTransformationMatrices( modelMatrix, viewMatrix, projectionMatrix,
                                              flatShaderProgramUniforms.mvpMatrix );

        // draw the curve control cage
        glBindVertexArray( vaos[VAOS.CAGE] );
        for( int i = 0; i < numSurfaces * 8; i++ ) {
            glDrawElements( GL_LINE_STRIP, 4, GL_UNSIGNED_SHORT, (void *)(sizeof( GLushort ) * i * 4));
        }
    }

    // use the bezier shader to draw surface
    bezierShaderProgram->useProgram();
    modelMatrix = glm::mat4(1.0f);
    computeAndSendTransformationMatrices(modelMatrix, viewMatrix, projectionMatrix,
                                         bezierShaderProgramUniforms.mvpMatrix);
    glBindVertexArray( vaos[VAOS.PATCH] );
    glDrawElements(GL_PATCHES, numSurfaces * POINTS_PER_PATCH, GL_UNSIGNED_SHORT, (void*)0);

}

/// updateScene() ////////////////////////////////////////////////////////////////
///
///      Update all of our scene objects - perform animation here
///
////////////////////////////////////////////////////////////////////////////////
void updateScene() {

}

/// run() ///////////////////////////////////////////////////////////////////////
///
///      Runs our draw loop and renders/updates our scene
///
////////////////////////////////////////////////////////////////////////////////
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

///**********************************************************************************************************************************************************
//
// Our main function

/// main() //////////////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////////////////////////
int main() {
    GLFWwindow *window = initialize();                  // create OpenGL context and setup EVERYTHING for our program
    run(window);                                        // enter our draw loop and run our program
    shutdown(window);                                   // free up all the memory used and close OpenGL context
    return EXIT_SUCCESS;				                // exit our program successfully!
}
