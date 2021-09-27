/*
 *  CSCI 444, Advanced Computer Graphics, Fall 2020
 *
 *  Project: lab09
 *  File: bezierPatch.te.glsl
 *
 *  Description:
 *      TES to interpolate a Bezier Patch at a given (u,v) coordinate
 *
 *  Author:
 *      Dr. Jeffrey Paone, Colorado School of Mines
 *
 *  Notes:
 *
 */

// we are using OpenGL 4.1 Core profile
#version 410 core

// TODO A specify the primitive information
layout(quads, equal_spacing, ccw) in;

// ***** TESSELLATION EVALUATION SHADER UNIFORMS *****
uniform mat4 mvpMatrix;

// ***** TESSELLATION EVALUATION SHADER INPUT *****


// ***** TESSELLATION EVALUATION SHADER OUTPUT *****
out vec3 color;

// ***** TESSELLATION EVALUATION SHADER HELPER FUNCTIONS *****
// TODO D write the Bezier Curve equation
vec4 BC(vec4 p0, vec4 p1, vec4 p2, vec4 p3, float t) {
    vec4 curvePoint;
    //f(t) = (-p0 + 3p1 - 3p2 + p3)*t^3 + (3p0 - 6p1 + 3p2)*t^2 + (-3p0 + 3p1)*t + p0
    curvePoint = (-1.0f * p0 + 3.0f * p1 - 3.0f * p2 + p3) * float(pow(t, 3))
    + (3.0f * p0 - 6.0f * p1 + 3.0f * p2) * float(pow(t, 2))
    + (-3.0f * p0 + 3.0f * p1)*t + p0;
    return curvePoint;
}

// ***** TESSELLATION EVALUATION SHADER MAIN FUNCTION *****
void main() {
    // TODO B get the tessellation coordinate
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;

    // TODO C get the control points
    vec4 p00 = gl_in[0].gl_Position;
    vec4 p01 = gl_in[1].gl_Position;
    vec4 p02 = gl_in[2].gl_Position;
    vec4 p03 = gl_in[3].gl_Position;
    vec4 p04 = gl_in[4].gl_Position;
    vec4 p05 = gl_in[5].gl_Position;
    vec4 p06 = gl_in[6].gl_Position;
    vec4 p07 = gl_in[7].gl_Position;
    vec4 p08 = gl_in[8].gl_Position;
    vec4 p09 = gl_in[9].gl_Position;
    vec4 p10 = gl_in[10].gl_Position;
    vec4 p11 = gl_in[11].gl_Position;
    vec4 p12 = gl_in[12].gl_Position;
    vec4 p13 = gl_in[13].gl_Position;
    vec4 p14 = gl_in[14].gl_Position;
    vec4 p15 = gl_in[15].gl_Position;

    // TODO E evaluate our Bezier Patch at coordinate (u,v) and store the result of the function
    vec4 bezPoint = BC(BC(p00, p01, p02, p03, u), BC(p04, p05, p06, p07, u), BC(p08, p09, p10, p11, u),
    BC(p12, p13, p14, p15, u), v);

    // TODO F set gl_Position to be our bezierPoint
    gl_Position = mvpMatrix * bezPoint;
    color = vec3(u, bezPoint.y, v);
}
