/*
 *   Fragment Shader
 *
 *   CSCI 441, Computer Graphics, Colorado School of Mines
 */

#version 410 core

// TODO #F3 create varying input for color to receive from vertex shader
in vec3 theColor;

// TODO #D create the output for our fragment shader
out vec4 fragColorOut;

void main() {

    /*****************************************/
    /******* Final Color Calculations ********/
    /*****************************************/
    
    // TODO #E assign our fragment shader output to be white
    fragColorOut = vec4(1, 1, 1, 1);
    // TODO #F4 assigned our fragment shader output to be our varying input
    fragColorOut = vec4(theColor, 1);
}
