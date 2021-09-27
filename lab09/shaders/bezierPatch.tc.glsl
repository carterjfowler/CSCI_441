/*
 *  CSCI 444, Advanced Computer Graphics, Fall 2020
 *
 *  Project: lab09
 *  File: bezierPatch.tc.glsl
 *
 *  Description:
 *      TCS to set up control points for tessellation
 *
 *  Author:
 *      Dr. Jeffrey Paone, Colorado School of Mines
 *
 *  Notes:
 *
 */

// we are using OpenGL 4.1 Core profile
#version 410 core

// ***** TESSELLATION CONTROL SHADER UNIFORMS *****


// ***** TESSELLATION CONTROL SHADER INPUT *****


// ***** TESSELLATION CONTROL SHADER OUTPUT *****
layout(vertices = 16) out;

// ***** TESSELLATION CONTROL SHADER HELPER FUNCTIONS *****


// ***** TESSELLATION CONTROL SHADER MAIN FUNCTION *****
void main() {
    gl_out[ gl_InvocationID ].gl_Position = gl_in[ gl_InvocationID ].gl_Position;

    gl_TessLevelOuter[0] = 20;
    gl_TessLevelOuter[1] = 20;
    gl_TessLevelOuter[2] = 20;
    gl_TessLevelOuter[3] = 20;

    gl_TessLevelInner[0] = 20;
    gl_TessLevelInner[1] = 20;
}
