/*
Title: Buffer Free 1
File Name: main.cpp
Copyright © 2015
Original authors: Joshua Alway (Updated by David Erbelding)
Written under the supervision of David I. Schwartz, Ph.D., and
supported by a professional development seed grant from the B. Thomas
Golisano College of Computing & Information Sciences
(https://www.rit.edu/gccis) at the Rochester Institute of Technology.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or (at
your option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

Description:
This program serves to demonstrate rendering without a buffer, using modulus math on just the gl_VertexID variable.
This data can be used to render without actually sending any data to the shader. This helps would-be graphics 
programmers understand how the graphics pipeline actually works. Each vertex is given a VertexID to identify it uniquely.
In this demo, the VertexID data is played with using modulus math to create a triangle strip.
*/

#include "GL\glew.h"
#include "GLFW/glfw3.h"
#include <iostream>

GLFWwindow* window;

GLuint vertexShader;
GLuint fragmentShader;
GLuint shaderProgram;

#define GLSL(src) "#version 400 \n" #src

// gl_VertexID is a built-in variable, similar to gl_Position.
// There are a variety of these you can find in
// openGL docs; they're worth knowing since you 
// can use them for creative and highly efficient things.
// They give very valuable data without the associated cost
// of using buffers to send it in.
//
// Here we use them to create a set of triangle vertices using
// some simple modulus math.
// 0, 1, 0, 1, 0, 1 .... = i % 2
// 0, 0, 1, 1, 0, 0 .... = (i / 2) % 2
//	Which if used as x and y coordinates, results in the following 
//  triangle strip, given 4 vertices as input:
//	2---3
//	| \	|
//	|  \|
//	0---1

const char* vertexSource = GLSL(

	void main()
	{
	    gl_Position = vec4(
	    mod( gl_VertexID, 2 ),
	    mod( gl_VertexID / 2, 2 ),
	    0.0,
	    1.0 );
	}
);

// In the fragment shader, we will use a second built-in variable:
// gl_PrimitiveID. This is the index of the primitive being drawn. In the case of 2 triangles,
// this will be 0 and 1; which we now plug into the green color channel.

// Additionally, we will use a third built-in variable:
// gl_FragCoord. This gives x and y pixel coordinates, among other things.
// These are in pixel coordinates relative to the bottom left corner of the window.
// These range from 0 to max width and height (800, and 600 for this demo).
// We're going to do a modulus of 50 pixels on the pixel height value, then multiply by 0.02 to
// give us a 0 to 1 range we will create some blue stripes with.
const char* fragmentSource = GLSL(

	void main()
	{
		float verticalStripes = mod( gl_FragCoord.y, 50.0 ) * 0.02;
		gl_FragColor = vec4( 0.0, gl_PrimitiveID, verticalStripes, 1.0 );
	}
);


void initShaders()
{
	// Compile the vertex shader
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	glCompileShader(vertexShader);

	// Compile the fragment shader
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);

	// Combine shaders into a program, bind, link and use
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);

	glLinkProgram(shaderProgram);

	glUseProgram(shaderProgram);
}

void init()
{
    glfwInit();

    window = glfwCreateWindow(800, 600, "Look Ma! No vertex buffer!", nullptr, nullptr);

    glfwMakeContextCurrent(window);

	glewExperimental = true;
	glewInit();

	//Compile shaders
	initShaders();
}

void draw()
{
	//Clear to a neutral grey
	glClearColor(0.5f, 0.5f, 0.5f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Draw two triangles. Triangle strip mode draws count - 2 triangles, sharing the last 2 vertices of a triangle with the next triangle.
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	//Swap buffers
    glfwSwapBuffers(window);
    glfwPollEvents();
}

void cleanUp()
{
	glDeleteProgram(shaderProgram);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}

int main()
{
	init();

	while (!glfwWindowShouldClose(window))
	{
		draw();
	}

	cleanUp();

	return 0;
}
