/*****************************
 * File: fshader42.glsl
 *       A simple fragment shader
 *****************************/

#version 120  // YJC: Comment/un-comment this line to resolve compilation errors
              //      due to different settings of the default GLSL version

varying vec4 color;
//varying vec4 fColor;

void main()
{
    gl_FragColor = color;
}

