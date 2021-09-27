/*
 *   Vertex Shader
 *
 *   CSCI 441, Computer Graphics, Colorado School of Mines
 */

#version 410 core

// TODO #A create a uniform for the ModelViewProjection matrix
uniform mat4 mvpMatrix;
// TODO #G1 create a uniform for time
uniform float time;

// TODO #B create an attribute for the vertex position
in vec3 vPosition;
// TODO #F1 create varying output for color to send to fragment shader
out vec3 theColor;

void main() {
    /*****************************************/
    /********* Vertex Calculations  **********/
    /*****************************************/

    // TODO #G2 modify our vertex position in object space by sine of time equation
    vec3 newVertex = vPosition;
    if (newVertex.x >= 0 && newVertex.y >= 0 && newVertex.z >= 0) {
        newVertex = vPosition + 1.2*((sin(time) + 1)/2) - 0.2;
    }
    
    // TODO #C set gl_Position to the vertex position transformed in to clip space
    gl_Position = mvpMatrix * vec4(newVertex, 1.0);

    // TODO #F2 set the color to be the vertex position in object space
    theColor = vPosition;
}
