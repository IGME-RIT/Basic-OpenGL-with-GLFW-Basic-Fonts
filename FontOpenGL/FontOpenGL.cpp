/*
Title: Buffer Free 1
File Name: main.cpp
Copyright © 2019
Original authors: Niko Procopi
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

// GLM headers
#define GLM_FORCE_PURE
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "stb_image.h"

int width, height;
char* pngData;
int png_size, nchan;
stbi_uc* img;
GLuint textureId;
GLuint sampler;

GLFWwindow* window;

GLuint vertexShader;
GLuint fragmentShader;
GLuint shaderProgram;

#define GLSL(src) "#version 450 \n" #src

const char* menuVS = GLSL(
	precision mediump float;

// our uniforms
uniform mat4 mdlvMtx;
uniform int text[100];

// output UV to fragment shader
out vec2 vtxUV;

void main()
{
	// generate a basic quad with VertexID,
	// just like previous tutorials

	vec2 pos;
	if (gl_VertexID == 0) pos = vec2(1, 0);
	if (gl_VertexID == 1) pos = vec2(0, 0);
	if (gl_VertexID == 2) pos = vec2(1, 1);
	if (gl_VertexID == 3) pos = vec2(0, 1);

	// scale to prevent everything from being huge
	vec2 scale = vec2(0.4, 0.4);

	// scale coordinates by inverse of
	// aspect ratio. Our aspect ratio is 
	// 4:3, so the inverse is (3,4)
	scale *= vec2(3, 4);

	// make square half as wide
	// make UV double as wide
	// this makes vertical rectangles,
	// instead of squares with empty space
	scale.x *= 0.5;

	// multiply position by scale
	pos *= scale;

	// each letter should be farther to the
	// right, so shift to the right depending
	// on which letter we are rendering.
	
	// gl_InstanceID tells us which character we
	// are drawing: character #0, #1, #2, etc
	pos.x += float(gl_InstanceID) * scale.x;

	// set the final position
	gl_Position = mdlvMtx * vec4(pos, 0.0, 1.0);

	// get the Uv coordinate
	vec2 uv = pos;

	// get the letter
	int letter = text[gl_InstanceID];

	// get the X,Y coordinate on the letter grid
	int letterX = 0;
	int letterY = 0;

	// subtract 32 to change ascii value to grid value
	letter -= 32;

	// 10 letters per row, find which row it is on
	while (letter >= 10)
	{
		letter -= 10;
		letterY += 1;
	}

	// now find which column it is in
	letterX = letter;

	// the texture is 250 pixels wide, and 250
	// pixels tall. The grid is a 10x10 grid of letters,
	// which means each square is in the grid is 
	// 25x25 pixels large.

	// The texture UV ranges from 0 to 1.0,
	// There are 10 rows and 10 columns of characters
	// That means the UV for each letter ranges
	// from 0 to 0.1

	// Lets take the first character for example.
	// If we want to render "Hello World", lets look
	// at the space: " ", which comes between "Hello"
	// and "World". The space has an ascii value of 32,
	// we subtract 32 from 32 to get zero.
	// when 'letter' is equal to zero, letterX and
	// letterY is zero. 

	// to get the UVs of space " ", we want 
	// those UVs to be:
	// (0.1, 0.1)
	// (0.0, 0.1)
	// (0.1, 0.0)
	// (0.0, 0.0)

	// However, we don't want each letter to be
	// a square, because then each letter will
	// have a lot of black space between them
	// (look at the texture), so if we want each
	// one to be a vertical rectangle, we want
	// these UVs of the space " ", to be:
	// (0.06, 0.1)
	// (0.0, 0.1)
	// (0.06, 0.0)
	// (0.0, 0.0)

	// Finally, to get UVs for any letter,
	// we take letterX and letterY into consideration,
	// to get a different square, in a different location,
	// with the same size

	if (gl_VertexID == 0)
	{
		uv.x = (float(letterX) + 0.6f) * 0.1;
		uv.y = (float(letterY) + 1.0f) * 0.1;
	}

	if (gl_VertexID == 1)
	{
		uv.x = (float(letterX) + 0.0f) * 0.1;
		uv.y = (float(letterY) + 1.0f) * 0.1;
	}

	if (gl_VertexID == 2)
	{
		uv.x = (float(letterX) + 0.6f) * 0.1;
		uv.y = (float(letterY) + 0.0f) * 0.1;
	}

	if (gl_VertexID == 3)
	{
		uv.x = (float(letterX) + 0.0f) * 0.1;
		uv.y = (float(letterY) + 0.0f) * 0.1;
	}

	// send this UV to the fragment shader
	vtxUV = uv;
}
);

const char* menuFS = GLSL(
	precision mediump float;

in vec2 vtxUV;
layout(location = 0) uniform sampler2D tex_diffuse;
out vec4 fragColor;

void main()
{
	// no need to undo or redo gamma correction
	fragColor = texture(tex_diffuse, vtxUV);
}
);


void initShaders()
{
	// Compile the vertex shader
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &menuVS, NULL);
	glCompileShader(vertexShader);

	GLint isCompiled = 0;
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &isCompiled);
	if (isCompiled == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character
		char* infoLog = (char*)malloc(maxLength);
		glGetShaderInfoLog(vertexShader, maxLength, &maxLength, &infoLog[0]);

		printf("%s\n", infoLog);
		system("pause");

		// We don't need the shader anymore.
		glDeleteShader(vertexShader);

		// Use the infoLog as you see fit.

		// In this simple program, we'll just leave
		return;
	}

	// Compile the fragment shader
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &menuFS, NULL);
	glCompileShader(fragmentShader);

	isCompiled = 0;
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &isCompiled);
	if (isCompiled == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character
		char* infoLog = (char*)malloc(maxLength);
		glGetShaderInfoLog(fragmentShader, maxLength, &maxLength, &infoLog[0]);

		printf("%s\n", infoLog);
		system("pause");

		// We don't need the shader anymore.
		glDeleteShader(fragmentShader);

		// Use the infoLog as you see fit.

		// In this simple program, we'll just leave
		return;
	}

	// Combine shaders into a program, bind, link and use
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);

	glLinkProgram(shaderProgram);

	glUseProgram(shaderProgram);
}

void readPNG(const char* path1, char** data, int* size)
{
	FILE *fp = fopen(path1, "rb");
	
	fseek(fp, 0L, SEEK_END);
	*size = ftell(fp);
	rewind(fp);

	*data = (char*)calloc(1, *size + 1);
	fread(*data, *size, 1, fp);

	fclose(fp);
}

void init()
{
	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	window = glfwCreateWindow(800, 600, "Look Ma! No vertex buffer!", nullptr, nullptr);

	glfwMakeContextCurrent(window);

	glewExperimental = true;
	glewInit();

	//Compile shaders
	initShaders();

	FILE* fp = fopen("../Assets/font2.png", "rb");
	img = stbi_load_from_file(fp, &width, &height, &nchan, 4);

	// Textures
	glGenTextures(1, &textureId);
	glActiveTexture(GL_TEXTURE0 + textureId); // activate the texture unit first before binding texture
	glBindTexture(GL_TEXTURE_2D, textureId);

	// common parameters for all textures
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// make the sampler if it does not exist
	if (sampler == 0)
		glGenSamplers(1, &sampler);

	// bind the sampler to the texture
	glBindSampler(textureId, sampler);

	// GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT = 34047
	// GL_TEXTURE_MAX_ANISOTROPY_EXT = 34046

	GLfloat maxAnisotropy = 0.0f;
	glGetFloatv(34047, &maxAnisotropy);

	// Trilinear Mipmapping
	glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glSamplerParameteri(sampler, 34046, (GLint)maxAnisotropy);

	// load texture and make mipmaps
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img);
	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(img);
	free(pngData);
}

void drawText(char message[100], glm::vec3 pos, glm::vec3 scale)
{
	glm::mat4x4 model = glm::mat4();
	model = glm::translate(model, pos);
	model = glm::scale(model, scale);

	// get length of name
	int numCharacters = (int)strlen(message);

	// uniform 0
	glUniform1i(0, (GLuint)textureId);

	// uniform 1, one matrix, and we give it the CPU pointer
	glUniformMatrix4fv(1, 1, GL_FALSE, &model[0][0]);

	// uniform 2 to 102
	for (int i = 0; i < numCharacters; i++)
		glUniform1i(i + 2, message[i]);

	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, numCharacters);
}

void draw()
{
	//Clear to a neutral grey
	glClearColor(0.5f, 0.5f, 0.5f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// line 1
	drawText((char*)"Hello World", glm::vec3(-0.5f, 0.5f, 0), glm::vec3(0.2f));
	
	// line 2
	drawText((char*)"There is room for improvement", glm::vec3(-1, 0, 0), glm::vec3(0.1f));

	// line 3
	drawText((char*)"But for a basic tutorial, it works, and gets the job done", 
		glm::vec3(-0.75f, -0.5f, 0), glm::vec3(0.05f));

	// line 4
	drawText((char*)"It works best with small text",
		glm::vec3(0.0f, -1.0f, 0), glm::vec3(0.05f));

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
