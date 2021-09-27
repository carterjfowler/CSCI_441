/*
 *   Geometry Shader
 *
 *   CSCI 441, Computer Graphics, Colorado School of Mines
 */

#version 410 core

// TODO #A
layout( points ) in;
// TODO #B
layout( triangle_strip, max_vertices = 24 ) out;

uniform mat4 projMatrix;

// TODO #I
out vec2 texToDraw;

void main() {
    float size = 1.0f;
    // TODO #C
//    gl_Position = projMatrix * (gl_in[0].gl_Position + vec4(-1, -1, 0, 0));
//    // TODO #D
//    texToDraw = vec2(0,0);
//    EmitVertex();
//    // TODO #F
//    gl_Position = projMatrix * (gl_in[0].gl_Position + vec4(-1, 1, 0, 0));
//    texToDraw = vec2(1,0);
//    EmitVertex();
//    // TODO #G
//    gl_Position = projMatrix * (gl_in[0].gl_Position + vec4(1, -1, 0, 0));
//    texToDraw = vec2(0,1);
//    EmitVertex();
//    // TODO #H
//    gl_Position = projMatrix * (gl_in[0].gl_Position + vec4(1, 1, 0, 0));
//    texToDraw = vec2(1,1);
//    EmitVertex();

    //front of the cube
    gl_Position = projMatrix * (gl_in[0].gl_Position + vec4(-1*size, -1*size, 0.5f * size, 0));
    texToDraw = vec2(0,0);
    EmitVertex();

    gl_Position = projMatrix * (gl_in[0].gl_Position + vec4(-1*size, size, 0.5f * size, 0));
    texToDraw = vec2(1,0);
    EmitVertex();

    gl_Position = projMatrix * (gl_in[0].gl_Position + vec4(size, -1*size, 0.5f * size, 0));
    texToDraw = vec2(0,1);
    EmitVertex();

    gl_Position = projMatrix * (gl_in[0].gl_Position + vec4(size, size, 0.5f * size, 0));
    texToDraw = vec2(1,1);
    EmitVertex();

    //back of the cube
    gl_Position = projMatrix * (gl_in[0].gl_Position + vec4(-1*size, -1*size, -0.5f * size, 0));
    texToDraw = vec2(0,0);
    EmitVertex();

    gl_Position = projMatrix * (gl_in[0].gl_Position + vec4(-1*size, size, -0.5f * size, 0));
    texToDraw = vec2(1,0);
    EmitVertex();

    gl_Position = projMatrix * (gl_in[0].gl_Position + vec4(size, -1*size, -0.5f * size, 0));
    texToDraw = vec2(0,1);
    EmitVertex();

    gl_Position = projMatrix * (gl_in[0].gl_Position + vec4(size, size, -0.5f * size, 0));
    texToDraw = vec2(1,1);
    EmitVertex();

    //left side of the cube
    gl_Position = projMatrix * (gl_in[0].gl_Position + vec4(-0.5f * size, -1*size, -1*size, 0));
    texToDraw = vec2(0,0);
    EmitVertex();

    gl_Position = projMatrix * (gl_in[0].gl_Position + vec4(-0.5f * size, size, -1*size, 0));
    texToDraw = vec2(1,0);
    EmitVertex();

    gl_Position = projMatrix * (gl_in[0].gl_Position + vec4(-0.5f * size, -1*size, size, 0));
    texToDraw = vec2(0,1);
    EmitVertex();

    gl_Position = projMatrix * (gl_in[0].gl_Position + vec4(-0.5f * size, size, size, 0));
    texToDraw = vec2(1,1);
    EmitVertex();

    //right side of the cube
    gl_Position = projMatrix * (gl_in[0].gl_Position + vec4(0.5f * size, -1*size, -1*size, 0));
    texToDraw = vec2(0,0);
    EmitVertex();

    gl_Position = projMatrix * (gl_in[0].gl_Position + vec4(0.5f * size, size, -1*size, 0));
    texToDraw = vec2(1,0);
    EmitVertex();

    gl_Position = projMatrix * (gl_in[0].gl_Position + vec4(0.5f * size, -1*size, size, 0));
    texToDraw = vec2(0,1);
    EmitVertex();

    gl_Position = projMatrix * (gl_in[0].gl_Position + vec4(0.5f * size, size, size, 0));
    texToDraw = vec2(1,1);
    EmitVertex();

    //top of the cube
    gl_Position = projMatrix * (gl_in[0].gl_Position + vec4(-1*size, 0.5f * size, -1*size, 0));
    texToDraw = vec2(0,0);
    EmitVertex();

    gl_Position = projMatrix * (gl_in[0].gl_Position + vec4(size, 0.5f * size, -1*size, 0));
    texToDraw = vec2(1,0);
    EmitVertex();

    gl_Position = projMatrix * (gl_in[0].gl_Position + vec4(-1*size, 0.5f * size, size, 0));
    texToDraw = vec2(0,1);
    EmitVertex();

    gl_Position = projMatrix * (gl_in[0].gl_Position + vec4(size, 0.5f * size, size, 0));
    texToDraw = vec2(1,1);
    EmitVertex();

    //botton of the cube
    gl_Position = projMatrix * (gl_in[0].gl_Position + vec4(-1*size, -0.5f * size, -1*size, 0));
    texToDraw = vec2(0,0);
    EmitVertex();

    gl_Position = projMatrix * (gl_in[0].gl_Position + vec4(size, -0.5f * size, -1*size, 0));
    texToDraw = vec2(1,0);
    EmitVertex();

    gl_Position = projMatrix * (gl_in[0].gl_Position + vec4(-1*size, -0.5f * size, size, 0));
    texToDraw = vec2(0,1);
    EmitVertex();

    gl_Position = projMatrix * (gl_in[0].gl_Position + vec4(size, -0.5f * size, size, 0));
    texToDraw = vec2(1,1);
    EmitVertex();
    // TODO #E
    EndPrimitive();
}
