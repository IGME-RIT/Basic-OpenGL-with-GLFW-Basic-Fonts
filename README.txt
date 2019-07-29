Documentation Author: Niko Procopi 2019

This tutorial was designed for Visual Studio 2017 / 2019
If the solution does not compile, retarget the solution
to a different version of the Windows SDK. If you do not
have any version of the Windows SDK, it can be installed
from the Visual Studio Installer Tool

Welcome to the Bufferless Font Tutorial!
Prerequesites: 
	Bufferless Rendering, 
	Instanced Rendering,
	Uniform Buffers (particles),
	C strings,
	Mipmapping
	Anisotropic Filtering

Fonts can be used to draw text in the OpenGL Window, 
rather than printing to the console. This can be used to
put text in menus, HUDs, tutorials, and credits. 

Two screenshots were included, one of them shows
this font system being used in a game that I am making
(to prove that the tutorial is useful), and the other 

[How it works]

If you look at Assets/font2.png, you will see an image
that has a grid of characters. We will be dividing this
square into smaller squares.Square 0 will be empty,
Square 3 will be #, Square 4 will be $, etc.
screenshot shows what this tutorial does.

Look at this website
http://www.asciitable.com/

Every time we make an array of char (for strings),
we are making an array of bytes. Each character
corresponds to a value, so 'a' is equal to 97.

Notice a pattern though, Space has a value of 32,
# has a value of 35, $ has a value of 36.

After we divide our grid in font.png into smaller squares,
to figure out which square corresponds to which character
we want to draw, we subtract 32 from the character we want
to draw

[Code]

First we call init() in C++, which sets the OpenGL version to 3.0,
this is important because newer versionf of OpenGL require
an empty VAO to be bound, or else it refuses to draw. Our bufferless
rendering tutorials do not use VAO or VBO, but if you are using a 
newer version of OpenGL, just make an empty VAO and bind it.

Next (in init()) we initialize the shaders, which are baked into the 
CPP file. If you want to transfer the shader code to seperate files,
feel free, its just a matter of preference. We compile, link, and
attach the shaders, just like previous tutorials.

We need to load the font image, rather than using FreeImage,
I used STB image, which accomplishes the same thing,
its just preference, feel free to change it
	FILE* fp = fopen("../Assets/font2.png", "rb");
	img = stbi_load_from_file(fp, &width, &height, &nchan, 4);

After that, the image is passed to GPU with mipmaps, just like
previous tutorials, like the Anisotropic Filtering tutorial

When that's done, we are fully initialized and ready to render.
In Draw(), clear the screen, draw some text, and swap buffers

To draw text, we call drawText(...), which takes a message, a 
glm::vec3 for position, and a glm::vec3 for scale.

Inside drawText(...), we make a model matrix from the parameters
	glm::mat4 model = glm::mat4();
	model = glm::translate(model, pos);
	model = glm::scale(model, scale);
	
We get the number of characters from the string that we want
to draw, we simply do this with strlen
	int numCharacters = (int)strlen(message);
	
We pass the texture to the fragment shader, we know that
the uniform location for this will be 0, but you can
use glGetUniformLocation if you want
	glUniform1i(0, (GLuint)textureId);
	
We know the model matrix will be in uniform 1, because
it is the first uniform that comes after the texture
	glUniformMatrix4fv(1, 1, GL_FALSE, &model[0][0]);
	
We know each text character will be uniforms 2-102, because
we have 100 characters after the model matrix. We need to
pass all the characters to the vertex shader
	for (int i = 0; i < numCharacters; i++)
		glUniform1i(i + 2, message[i]);
		
Here is where the magic happens, we use instanced rendering
with procedurally-generated geometry. With one draw-call we can
draw an entire line of text.
Each letter will have 4 vertices (one quad), and there will
be one quad for every character on the line
	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, numCharacters);

[Vertex Shader]
	
The Vertex Shader is absolutely filled with comments,
lines 58 to 200, so please read those comments
	
[Fragment Shader]

Just export the color from the texture and UV
	fragColor = texture(tex_diffuse, vtxUV);
	
How to improve:
Add a way to change the color of the text
Look at try_on_your_own.png, try to render text in relation to 3D objects
	this involves projection and view matrices to get a 3D position,
	and then converting that to a 2D position, and then drawing text

