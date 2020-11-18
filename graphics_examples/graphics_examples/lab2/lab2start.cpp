/*
 Lab2start.cpp
 Creates a cube and defines a uniform variable to pass a transformation
 to the vertx shader.
 Use this example as a start to lab2 or extract bits and add to
 an example of your own to practice working with 3D transformations
 and uniform variables.
 Iain Martin October 2018
*/

/* Link to static libraries, could define these as linker inputs in the project settings instead
if you prefer */
#ifdef _DEBUG
#pragma comment(lib, "glfw3D.lib")
#pragma comment(lib, "glloadD.lib")
#else
#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "glload.lib")
#endif
#pragma comment(lib, "opengl32.lib")

/* Include the header to the GLFW wrapper class which
   also includes the OpenGL extension initialisation*/
#include "wrapper_glfw.h"
#include <iostream>

   /* GLM headers */
#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <stack>

using namespace std;
using namespace glm;

GLuint positionBufferObject, colourObject;
GLuint program;
GLuint vao;

/* Position and view globals */
GLfloat angle_x, angle_x_inc;
GLfloat angle_y, angle_y_inc;
GLfloat angle_z, angle_z_inc;

GLfloat cameraZ = 0.7f;

/* Uniforms*/
GLuint modelID;
GLuint shaderSwitch = 0;

vec3 cubePosition;

/*
This function is called before entering the main rendering loop.
Use it for all your initialisation stuff
*/
void init(GLWrapper* glw)
{
	angle_x = 0;
	angle_x_inc = 0;

	angle_y = 0;
	angle_y_inc = 0;

	angle_z = 0;
	angle_z_inc = 0;

	cubePosition = vec3(0);

	glEnable(GL_DEPTH_TEST);

	// Generate index (name) for one vertex array object
	glGenVertexArrays(1, &vao);

	// Create the vertex array object and make it current
	glBindVertexArray(vao);

	/* Define vertices for a cube in 12 triangles */
	GLfloat vertexPositions[] =
	{
		-0.25f, 0.25f, -0.25f, 1.f,
		-0.25f, -0.25f, -0.25f, 1.f,
		0.25f, -0.25f, -0.25f, 1.f,

		0.25f, -0.25f, -0.25f, 1.f,
		0.25f, 0.25f, -0.25f, 1.f,
		-0.25f, 0.25f, -0.25f, 1.f,

		0.25f, -0.25f, -0.25f, 1.f,
		0.25f, -0.25f, 0.25f, 1.f,
		0.25f, 0.25f, -0.25f, 1.f,

		0.25f, -0.25f, 0.25f, 1.f,
		0.25f, 0.25f, 0.25f, 1.f,
		0.25f, 0.25f, -0.25f, 1.f,

		0.25f, -0.25f, 0.25f, 1.f,
		-0.25f, -0.25f, 0.25f, 1.f,
		0.25f, 0.25f, 0.25f, 1.f,

		-0.25f, -0.25f, 0.25f, 1.f,
		-0.25f, 0.25f, 0.25f, 1.f,
		0.25f, 0.25f, 0.25f, 1.f,

		-0.25f, -0.25f, 0.25f, 1.f,
		-0.25f, -0.25f, -0.25f, 1.f,
		-0.25f, 0.25f, 0.25f, 1.f,

		-0.25f, -0.25f, -0.25f, 1.f,
		-0.25f, 0.25f, -0.25f, 1.f,
		-0.25f, 0.25f, 0.25f, 1.f,

		-0.25f, -0.25f, 0.25f, 1.f,
		0.25f, -0.25f, 0.25f, 1.f,
		0.25f, -0.25f, -0.25f, 1.f,

		0.25f, -0.25f, -0.25f, 1.f,
		-0.25f, -0.25f, -0.25f, 1.f,
		-0.25f, -0.25f, 0.25f, 1.f,

		-0.25f, 0.25f, -0.25f, 1.f,
		0.25f, 0.25f, -0.25f, 1.f,
		0.25f, 0.25f, 0.25f, 1.f,

		0.25f, 0.25f, 0.25f, 1.f,
		-0.25f, 0.25f, 0.25f, 1.f,
		-0.25f, 0.25f, -0.25f, 1.f
	};

	/* Define an array of colours */
	float vertexColours[] = {
		0.0f, 0.0f, 1.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f,

		0.0f, 1.0f, 0.0f, 1.0f,
		0.0f, 1.0f, 0.0f, 1.0f,
		0.0f, 1.0f, 0.0f, 1.0f,
		0.0f, 1.0f, 0.0f, 1.0f,
		0.0f, 1.0f, 0.0f, 1.0f,
		0.0f, 1.0f, 0.0f, 1.0f,

		1.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 0.0f, 1.0f,

		1.0f, 0.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 0.0f, 1.0f,

		1.0f, 0.0f, 1.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 1.0f,

		0.0f, 1.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 1.0f, 1.0f,
	};

	/* Create a vertex buffer object to store vertices */
	glGenBuffers(1, &positionBufferObject);
	glBindBuffer(GL_ARRAY_BUFFER, positionBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexPositions), vertexPositions, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	/* Create a vertex buffer object to store vertex colours */
	glGenBuffers(1, &colourObject);
	glBindBuffer(GL_ARRAY_BUFFER, colourObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexColours), vertexColours, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	try
	{
		program = glw->LoadShader("lab2.vert", "lab2.frag");
	}
	catch (exception& e)
	{
		cout << "Caught exception: " << e.what() << endl;
		cin.ignore();
		exit(0);
	}

	/* Define uniforms to send to vertex shader */
	modelID = glGetUniformLocation(program, "model");

	shaderSwitch = glGetUniformLocation(program, "shaderSwitch");
	glUniform1ui(shaderSwitch, 1);
}

//Called to update the display.
//You should call glfwSwapBuffers() after all of your rendering to display what you rendered.
void display()
{
	/* Define the background colour */
	glClearColor(0.180f, 0.203f, 0.250f, 1.0f);

	/* Clear the colour and frame buffers */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//glClear(GL_DEPTH_BUFFER_BIT);

	glUseProgram(program);

	glBindBuffer(GL_ARRAY_BUFFER, positionBufferObject);
	glEnableVertexAttribArray(0);

	/* glVertexAttribPointer(index, size, type, normalised, stride, pointer)
	   index relates to the layout qualifier in the vertex shader and in
	   glEnableVertexAttribArray() and glDisableVertexAttribArray() */
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, colourObject);
	glEnableVertexAttribArray(1);

	/* glVertexAttribPointer(index, size, type, normalised, stride, pointer)
	index relates to the layout qualifier in the vertex shader and in
	glEnableVertexAttribArray() and glDisableVertexAttribArray() */
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);

	mat4 view = lookAt(
		vec3(0, 0, cameraZ),
		vec3(0, 0, 0),
		vec3(0.5f, 1, 0)
	);

	// Model matrix : an identity matrix (model will be at the origin)
	stack<mat4> model;
	model.push(mat4(1.0f) * view);

	for (float i = -5; i < 5; i += 0.4f)
	{
		for (float j = -5; j < 5; j += 0.4f)
		{
			model.push(model.top());
			{
				model.top() = translate(model.top(), vec3(cubePosition.x + i, cubePosition.y + j, cubePosition.z)); //moving

				model.top() = rotate(model.top(), -angle_x * i, vec3(1, 0, 0)); //rotating in clockwise direction around x-axis
				model.top() = rotate(model.top(), -angle_y * j, vec3(0, 1, 0)); //rotating in clockwise direction around y-axis
				model.top() = rotate(model.top(), -angle_z, vec3(0, 0, 1)); //rotating in clockwise direction around z-axis

				model.top() = scale(model.top(), vec3(0.5f, 0.5f, 0.5f));

				// Send our transformations to the currently bound shader,
				glUniformMatrix4fv(modelID, 1, GL_FALSE, &model.top()[0][0]);
				//glUniformMatrix4fv(modelID, 1, GL_FALSE, &view[0][0]);

				glDrawArrays(GL_TRIANGLES, 0, 36);
			}
			model.pop();
		}
	}

	glDisableVertexAttribArray(0);
	glUseProgram(0);

	/* Modify our animation variables */
	angle_x += angle_x_inc;
	angle_y += angle_y_inc;
	angle_z += angle_z_inc;
}


/* Called whenever the window is resized. The new window size is given, in pixels. */
static void reshape(GLFWwindow* window, int w, int h)
{
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
}

/* change view angle, exit upon ESC */
static void keyCallback(GLFWwindow* window, int k, int s, int action, int mods)
{
	if (action != GLFW_PRESS) return;

	if (k == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (k == 'S') angle_x_inc += 0.005f;
	if (k == 'W') angle_x_inc -= 0.005f;
	if (k == 'D') angle_y_inc += 0.005f;
	if (k == 'A') angle_y_inc -= 0.005f;
	if (k == 'E') angle_z_inc += 0.005f;
	if (k == 'Q') angle_z_inc -= 0.005f;

	if (k == 'O') cameraZ -= 0.05f;
	if (k == 'P') cameraZ += 0.05f;


	if (k == 'Z') cubePosition.x -= 0.05f;
	if (k == 'X') cubePosition.x += 0.05f;

	if (k == 'C') cubePosition.y -= 0.05f;
	if (k == 'V') cubePosition.y += 0.05f;

	if (k == 'B') cubePosition.z -= 0.05f;
	if (k == 'N') cubePosition.z += 0.05f;

	if (k == GLFW_KEY_SPACE && action == GLFW_PRESS)
	{
		angle_x_inc = 0;
		angle_x_inc = 0;
		angle_y_inc = 0;
		angle_y_inc = 0;
		angle_z_inc = 0;
		angle_z_inc = 0;
	}
}

/* An error callback function to output GLFW errors*/
static void error_callback(int error, const char* description)
{
	fputs(description, stderr);
}

/* Entry point of program */
int main(int argc, char* argv[])
{
	GLWrapper* glw = new GLWrapper(900, 600, "Lab2: 3D Cube Transforms");;

	if (!ogl_LoadFunctions())
	{
		fprintf(stderr, "ogl_LoadFunctions() failed. Exiting\n");
		return 0;
	}

	/* Note it you might want to move this call to the wrapper class */
	glw->setErrorCallback(error_callback);

	glw->setRenderer(display);
	glw->setKeyCallback(keyCallback);
	glw->setReshapeCallback(reshape);

	// Output version
	glw->DisplayVersion();

	init(glw);

	glw->eventLoop();

	delete(glw);
	return 0;
}



