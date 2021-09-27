/*
 *  CSCI 441, Computer Graphics, Fall 2020
 *
 *  Project: lab08
 *  File: main.cpp
 *
 *  Description:
 *      This file contains the start to render and move along a curve.
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

#include <CSCI441/OpenGLUtils.hpp>      // prints OpenGL information
#include <CSCI441/objects.hpp>          // draws 3D objects
#include <CSCI441/ShaderProgram.hpp>    // wrapper class for GLSL shader programs
#include <vector>

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

// keep track of our objects
GLuint platformVAO, platformVBOs[2];    // the ground platform everything is hovering over
GLfloat currentSpherePos = 0;
// Bezier Curve Information
GLuint numControlPoints;                // number of control points in the curve system
GLuint numCurves;                       // number of curves in the system
glm::vec3 *controlPoints = nullptr;     // control points array
GLuint bezierCageVAO, bezierCageVBO;    // the bezier control cage VAO
GLuint numEvalPoints;                   // number of points evaluated
GLuint bezierCurveVAO, bezierCurveVBO;  // the actual curve VAO
int *resolution;

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

// represent all of the material properties
struct Material {
    glm::vec3 diffuse;                  // the diffuse property
    glm::vec3 specular;                 // the specular property
    GLfloat shininess;                  // the shininess factor for specular
    glm::vec3 ambient;                  // the ambient property
} materialEmerald, materialRuby, materialBronze;

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
void sendMaterialProperties(Material material, GLint diffuseLocation, GLint specularLocation, GLint shininessLocation, GLint ambientLocation) {
    glUniform3fv(diffuseLocation, 1, &(material.diffuse[0]));
    glUniform3fv(specularLocation, 1, &(material.specular[0]));
    glUniform1f(shininessLocation, material.shininess);
    glUniform3fv(ambientLocation, 1, &(material.ambient[0]));
}

/// loadControlPointsFromFile() /////////////////////////////////////////////////
///
/// This function loads the Bezier control points from a given file.  Upon
/// completion, the parameters will store the number of points read in, the
/// number of curves they represent, and the array of actual points.
///
////////////////////////////////////////////////////////////////////////////////
void loadControlPointsFromFile(const char* FILENAME, GLuint *numBezierPoints, GLuint *numBezierCurves, glm::vec3* &bezierPoints) {
    FILE *file = fopen(FILENAME, "r");

    if(!file) {
        fprintf( stderr, "[ERROR]: Could not open \"%s\"\n", FILENAME );
    } else {
        fscanf( file, "%u\n", numBezierPoints );

        *numBezierCurves = (*numBezierPoints-1)/3;

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

/// evalBezierCurve() //////////////////////////////////////////////////////////
///
/// This function solves the Bezier curve equation for four given control
/// points at a given location t.
///
////////////////////////////////////////////////////////////////////////////////
glm::vec3 evalBezierCurve(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, GLfloat t) {
    // TODO #01 - solve our Bezier Curve equation
    glm::vec3 curvePoint(0.0f);
    //f(t) = (-p0 + 3p1 - 3p2 + p3)*t^3 + (3p0 - 6p1 + 3p2)*t^2 + (-3p0 + 3p1)*t + p0
    curvePoint = (-1.0f * p0 + 3.0f * p1 - 3.0f * p2 + p3) * float(pow(t, 3))
            + (3.0f * p0 - 6.0f * p1 + 3.0f * p2) * float(pow(t, 2))
            + (-3.0f * p0 + 3.0f * p1)*t + p0;
    return curvePoint;
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
	GLFWwindow *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Lab08: Bezier Curves", nullptr, nullptr );
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
}

void genCurve() {
//    fprintf( stdout, "[INFO]: Read in %u points comprising %u curves\n", numControlPoints, numCurves );

    // --------------------------------------------------------------------------------------------------
    // generate cage

    glGenVertexArrays( 1, &bezierCageVAO );
    glBindVertexArray( bezierCageVAO );

    glGenBuffers( 1, &bezierCageVBO );
    glBindBuffer( GL_ARRAY_BUFFER, bezierCageVBO );
    glBufferData( GL_ARRAY_BUFFER, numControlPoints * sizeof(glm::vec3), controlPoints, GL_STATIC_DRAW );

    glEnableVertexAttribArray( flatShaderProgramAttributes.vPos );
    glVertexAttribPointer( flatShaderProgramAttributes.vPos, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0 );

//    fprintf( stdout, "[INFO]: control points cage read in with VAO %d\n", bezierCageVAO );

    // --------------------------------------------------------------------------------------------------
    // generate curve

    // TODO #02 - generate the curve for a given resolution

    numEvalPoints = *resolution + 1;
    std::vector<glm::vec3> calcPoints;
    for (int i = 0; i < numControlPoints; i += 3) {
        glm::vec3 p0 = controlPoints[0 + i], p1 = controlPoints[1 + i],
                p2 = controlPoints[2 + i], p3 = controlPoints[3 + i];
        for (int j = 0; j < numEvalPoints; ++j) {
            float time = float(j) / float(*resolution);
            calcPoints.emplace_back(evalBezierCurve(p0, p1, p2, p3, time));
        }
    }
    glGenVertexArrays( 1,  &bezierCurveVAO);
    glBindVertexArray( bezierCurveVAO );

    glGenBuffers( 1, &bezierCurveVBO );
    glBindBuffer( GL_ARRAY_BUFFER, bezierCurveVBO );
    glBufferData( GL_ARRAY_BUFFER, numCurves * numEvalPoints * sizeof(glm::vec3), &calcPoints[0], GL_STATIC_DRAW );

    glEnableVertexAttribArray( flatShaderProgramAttributes.vPos );
    glVertexAttribPointer( flatShaderProgramAttributes.vPos, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0 );
//    fprintf( stdout, "[INFO]: bezier curve read in with VAO %d\n", bezierCurveVAO );
}

/// setupBuffers() //////////////////////////////////////////////////////////////
///
///      Create our VAOs & VBOs. Send vertex data to the GPU for future rendering
///
////////////////////////////////////////////////////////////////////////////////
void setupBuffers() {

    // ------------------------------------------------------------------------------------------------------
    // create the ground plane

	struct Vertex {
		float x, y, z;
        float nx, ny, nz;
        float s, t;
	};

	// create our platform
    Vertex platformVertices[4] = {
			{ -0.5f, 0.0f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f }, // 0 - BL
			{  0.5f, 0.0f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f }, // 1 - BR
			{ -0.5f, 0.0f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f }, // 2 - TL
			{  0.5f, 0.0f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f }  // 3 - TR
	};

	unsigned short platformIndices[4] = { 0, 1, 2, 3 };

	glGenVertexArrays( 1, &platformVAO );
	glBindVertexArray( platformVAO );

	glGenBuffers( 2, platformVBOs );

	glBindBuffer( GL_ARRAY_BUFFER, platformVBOs[0] );
	glBufferData( GL_ARRAY_BUFFER, sizeof( platformVertices ), platformVertices, GL_STATIC_DRAW );

	glEnableVertexAttribArray( gouradShaderProgramAttributes.vPos );
	glVertexAttribPointer( gouradShaderProgramAttributes.vPos, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) 0 );

    glEnableVertexAttribArray( gouradShaderProgramAttributes.vNormal );
    glVertexAttribPointer( gouradShaderProgramAttributes.vNormal, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float)) );

	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, platformVBOs[1] );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( platformIndices ), platformIndices, GL_STATIC_DRAW );

	fprintf( stdout, "[INFO]: platform read in with VAO %d\n", platformVAO );

    // ------------------------------------------------------------------------------------------------------
    // read in the control points

	fprintf( stdout, "\nEnter filename of Bezier Control Points to load: " );
	char *filename = (char*)malloc(sizeof(char)*256);
	fscanf( stdin, "%s", filename );
	loadControlPointsFromFile(filename, &numControlPoints, &numCurves, controlPoints);

	if(!controlPoints) {
	    fprintf( stderr, "[ERROR]: Error loading control points from file\n" );
	} else {
        fprintf( stdout, "\nEnter the desired resolution of the Bezier Curve: " );
        resolution = (int*)malloc(sizeof(int)*256);
        fscanf( stdin, "%d", resolution );
        genCurve();
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

    // set up material properties
    materialEmerald.diffuse   = glm::vec3(0.07568f, 0.61424f, 0.07568f);
    materialEmerald.specular  = glm::vec3(0.633f, 0.727811f, 0.633f);
    materialEmerald.shininess = 128.0f * 0.6f;
    materialEmerald.ambient   = glm::vec3(0.0215f, 0.1745f, 0.0215f);

    materialRuby.diffuse      = glm::vec3(0.61424f, 0.04136f, 0.04136f);
    materialRuby.specular     = glm::vec3(0.727811f, 0.626959f, 0.626959f);
    materialRuby.shininess    = 128.0f * 0.6f;
    materialRuby.ambient      = glm::vec3(0.1745f, 0.01175f, 0.01175f);

    materialBronze.diffuse    = glm::vec3(0.714f, 0.4284f, 0.18144f);
    materialBronze.specular   = glm::vec3(0.393548f, 0.271906f, 0.166721f);
    materialBronze.shininess  = 128.0f * 0.2f;
    materialBronze.ambient    = glm::vec3(0.2125f, 0.1275f, 0.054f);

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
}

/// cleanupBuffers() ////////////////////////////////////////////////////////////
///
///      Delete VAOs and VBOs off of the GPU
///
////////////////////////////////////////////////////////////////////////////////
void cleanupBuffers() {
    fprintf( stdout, "[INFO]: ...deleting VBOs....\n" );

    glDeleteBuffers( 2, platformVBOs );
    glDeleteBuffers( 1, &bezierCageVBO );
    glDeleteBuffers( 1, &bezierCurveVBO );
    CSCI441::deleteObjectVBOs();

    fprintf( stdout, "[INFO]: ...deleting VAOs....\n" );

    glDeleteVertexArrays( 1, &platformVAO );
    glDeleteVertexArrays( 1, &bezierCageVAO );
    glDeleteVertexArrays( 1, &bezierCurveVAO );
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
    // use the gourad shader
    gouradShaderProgram->useProgram();
    // set the eye position - needed for specular reflection
    glUniform3fv(gouradShaderProgramUniforms.eyePos, 1, &(arcballCam.eyePos[0]));

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
                                          gouradShaderProgramAttributes.vNormal); // vertex normal location

    glm::mat4 modelMatrix = glm::mat4(1.0f);

    // use the emerald material
    sendMaterialProperties(materialEmerald,
                           gouradShaderProgramUniforms.materialDiffColor,
                           gouradShaderProgramUniforms.materialSpecColor, gouradShaderProgramUniforms.materialShininess,
                           gouradShaderProgramUniforms.materialAmbColor);


    // draw a larger ground plane by translating a single quad across a grid
    for(int i = -5; i <= 5; i++) {
        for(int j = -5; j <= 5; j++) {
            modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(i, 0, j));
            computeAndSendTransformationMatrices(modelMatrix, viewMatrix, projectionMatrix,
                                                 gouradShaderProgramUniforms.mvpMatrix,
                                                 gouradShaderProgramUniforms.modelMatrix,
                                                 gouradShaderProgramUniforms.normalMtx);
            glBindVertexArray( platformVAO );
            glDrawElements( GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, (void*)0 );
        }
    }

    genCurve();

    // use the ruby material
    sendMaterialProperties(materialRuby,
                           gouradShaderProgramUniforms.materialDiffColor,
                           gouradShaderProgramUniforms.materialSpecColor, gouradShaderProgramUniforms.materialShininess,
                           gouradShaderProgramUniforms.materialAmbColor);

    // draw each of the control points represented by a sphere
    for(int i = 0; i < numControlPoints; i++) {
        modelMatrix = glm::translate(glm::mat4(1.0f), controlPoints[i]);
        computeAndSendTransformationMatrices(modelMatrix, viewMatrix, projectionMatrix,
                                             gouradShaderProgramUniforms.mvpMatrix,
                                             gouradShaderProgramUniforms.modelMatrix,
                                             gouradShaderProgramUniforms.normalMtx);
        CSCI441::drawSolidSphere(0.25f, 16, 16);
    }

    // use the bronze material
    sendMaterialProperties(materialBronze,
                           gouradShaderProgramUniforms.materialDiffColor,
                           gouradShaderProgramUniforms.materialSpecColor, gouradShaderProgramUniforms.materialShininess,
                           gouradShaderProgramUniforms.materialAmbColor);

    // TODO #03 - evaluate the current position along the curve system and draw a sphere at the current point
    GLint curveNum = floor(currentSpherePos);
    glm::vec3 posOnCurve = evalBezierCurve(controlPoints[0 + 3 * curveNum], controlPoints[1 + 3 * curveNum],
        controlPoints[2 + 3 * curveNum], controlPoints[3 + 3 * curveNum], currentSpherePos - GLfloat(curveNum));
    modelMatrix = glm::translate(glm::mat4(1.0f), posOnCurve);
    computeAndSendTransformationMatrices(modelMatrix, viewMatrix, projectionMatrix,
                                         gouradShaderProgramUniforms.mvpMatrix,
                                         gouradShaderProgramUniforms.modelMatrix,
                                         gouradShaderProgramUniforms.normalMtx);
    CSCI441::drawSolidSphere(0.25f, 16, 16);


    // use the flat shader to draw lines
    flatShaderProgram->useProgram();
    modelMatrix = glm::mat4(1.0f);
    computeAndSendTransformationMatrices(modelMatrix, viewMatrix, projectionMatrix,
                                         flatShaderProgramUniforms.mvpMatrix);

    // draw the curve control cage
    glBindVertexArray( bezierCageVAO );
    glDrawArrays(GL_LINE_STRIP, 0, numControlPoints);

    // LOOKHERE #1 draw the curve itself
    glBindVertexArray( bezierCurveVAO );
    glDrawArrays(GL_LINE_STRIP, 0, numCurves * numEvalPoints);
}

/// updateScene() ////////////////////////////////////////////////////////////////
///
///      Update all of our scene objects - perform animation here
///
////////////////////////////////////////////////////////////////////////////////
void updateScene() {
    if (currentSpherePos > numCurves)
        currentSpherePos = 0.0f;
    else
        currentSpherePos += 0.005f;
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
