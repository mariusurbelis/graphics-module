/*
 poslight.cpp
 Demonstrates a positional light with attenuation
 with per-vertex lighting (Gouraud shading) coded in the vertex shader.
 Displays a cube and a sphere and a small sphere to show the light position
 Includes controls to move the light source and rotate the view
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
#include <stack>

   /* Include GLM core and matrix extensions*/
#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtc/type_ptr.hpp>

// Include headers for our objects
#include "sphere.h"
#include "cube.h"
#include "claw.h"
#include "cylinder.h"

using namespace std;
using namespace glm;

void printInstructions();
void setColor(float red, float green, float blue);

/* Define buffer object indices */
GLuint elementbuffer;

GLuint program;		/* Identifier for the shader prgoram */
GLuint vao;			/* Vertex array (Containor) object. This is the index of the VAO that will be the container for
					   our buffer objects */

GLuint colourmode;	/* Index of a uniform to switch the colour mode in the vertex shader
					  I've included this to show you how to pass in an unsigned integer into
					  your vertex shader. */
GLuint emitmode;
GLuint attenuationmode;

/* Position and view globals */
GLfloat angle_x, angle_inc_x, x, model_scale, z, y, vx, vy, vz;
GLfloat angle_y, angle_inc_y, angle_z, angle_inc_z;
GLuint drawmode;			// Defines drawing mode of sphere as points, lines or filled polygons
GLuint numlats, numlongs;	//Define the resolution of the sphere object
GLfloat speed;				// movement increment

GLfloat sunPower;

GLfloat panelOneRotation, panelTwoRotation;

vec3 lightPosition;
vec3 partColor;
vec3 issPosition;

vec3 armRootRotation;
vec3 armJointRotation;
vec3 armTipRotation;

vec3 pivot;

/* Uniforms*/
GLuint modelID, viewID, projectionID, lightposID, normalmatrixID, sunPowerID, partColorID;
GLuint colourmodeID, emitmodeID, attenuationmodeID;

GLfloat aspect_ratio;		/* Aspect ratio of the window defined in the reshape callback*/
GLuint numspherevertices;

/* Global instances of our objects */
Sphere aSphere;
Cube aCube;
Claw aClaw;
Cylinder aCylinder;

/*
This function is called before entering the main rendering loop.
Use it for all your initialisation stuff
*/
void init(GLWrapper* glw)
{
	/* Set the object transformation controls to their initial values */
	speed = 0.9f;
	x = 0.05f;
	y = 0;
	z = 0;
	vx = 0; vy = 140, vz = 4.f;
	angle_x = angle_y = angle_z = 0;
	angle_inc_x = angle_inc_y = angle_inc_z = 0;
	model_scale = 1.f;
	aspect_ratio = 1.3333f;
	colourmode = 1; emitmode = 0;
	attenuationmode = 0; // Attenuation is on by default
	numlats = 60;		// Number of latitudes in our sphere
	numlongs = 60;		// Number of longitudes in our sphere

	issPosition = vec3(0);

	armRootRotation = vec3(0);
	armJointRotation = vec3(0);
	armTipRotation = vec3(0);

	lightPosition = vec3(1, 0, 2);
	
	partColor = vec3(0.5f, 0.5f, 0.5f);

	sunPower = 0.05f;

	// Generate index (name) for one vertex array object
	glGenVertexArrays(1, &vao);

	// Create the vertex array object and make it current
	glBindVertexArray(vao);

	/* Load and build the vertex and fragment shaders */
	try
	{
		program = glw->LoadShader("assignment.vert", "assignment.frag");
	}
	catch (exception& e)
	{
		cout << "Caught exception: " << e.what() << endl;
		cin.ignore();
		exit(0);
	}

	/* Define uniforms to send to vertex shader */
	modelID = glGetUniformLocation(program, "model");
	colourmodeID = glGetUniformLocation(program, "colourmode");
	emitmodeID = glGetUniformLocation(program, "emitmode");
	attenuationmodeID = glGetUniformLocation(program, "attenuationmode");
	viewID = glGetUniformLocation(program, "view");
	projectionID = glGetUniformLocation(program, "projection");
	lightposID = glGetUniformLocation(program, "lightpos");
	normalmatrixID = glGetUniformLocation(program, "normalmatrix");
	sunPowerID = glGetUniformLocation(program, "sunPower");
	partColorID = glGetUniformLocation(program, "partColor");

	/* create our sphere and cube objects */
	aSphere.makeSphere(numlats, numlongs);
	aCube.makeCube();
	aClaw.makeClaw();
	aCylinder.makeCylinder();

	printInstructions();
}

/* Called to update the display. Note that this function is called in the event loop in the wrapper
   class because we registered display as a callback function */
void display()
{
	/* Define the background colour */
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	/* Clear the colour and frame buffers */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* Enable depth test  */
	glEnable(GL_DEPTH_TEST);

	/* Make the compiled shader program current */
	glUseProgram(program);

	// Define our model transformation in a stack and 
	// push the identity matrix onto the stack
	stack<mat4> model;
	model.push(mat4(1.0f));

	// Define the normal matrix
	mat3 normalmatrix;

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	mat4 projection = perspective(radians(30.0f), aspect_ratio, 0.1f, 100.0f);

	// Camera matrix
	mat4 view = lookAt(
		vec3(0, 2, 6), // Camera is at (0,0,4), in World Space
		vec3(0, 0, 0), // and looks at the origin
		vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
	);

	// Apply rotations to the view position. This wil get appleid to the whole scene
	view = rotate(view, -radians(vx), vec3(1, 0, 0)); //rotating in clockwise direction around x-axis
	view = rotate(view, -radians(vy), vec3(0, 1, 0)); //rotating in clockwise direction around y-axis
	view = rotate(view, -radians(vz), vec3(0, 0, 1));

	// Define the light position and transform by the view matrix
	vec4 lightpos = view * vec4(lightPosition, 1.0);

	// Send our projection and view uniforms to the currently bound shader
	// I do that here because they are the same for all objects
	glUniform1ui(colourmodeID, colourmode);
	glUniform1ui(attenuationmodeID, attenuationmode);
	glUniform1f(sunPowerID, sunPower);
	glUniformMatrix4fv(viewID, 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(projectionID, 1, GL_FALSE, &projection[0][0]);
	glUniform4fv(lightposID, 1, value_ptr(lightpos));
	glUniform3fv(partColorID, 1, value_ptr(partColor));

	/* Draw a small sphere in the lightsource position to visually represent the light source */
	model.push(model.top());
	{
		model.top() = translate(model.top(), vec3(lightPosition.x, lightPosition.y, lightPosition.z));
		model.top() = scale(model.top(), vec3(0.05f, 0.05f, 0.05f)); // make a small sphere
		// Recalculate the normal matrix and send the model and normal matrices to the vertex shader
		glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

		/* Draw our lightposition sphere  with emit mode on*/
		emitmode = 1;
		glUniform1ui(emitmodeID, emitmode);
		aSphere.drawSphere(drawmode);
		emitmode = 0;
		glUniform1ui(emitmodeID, emitmode);
	}
	model.pop();

	// Define the global model transformations (rotate and scale). Note, we're not modifying thel ight source position
	model.top() = scale(model.top(), vec3(model_scale, model_scale, model_scale));//scale equally in all axis
	//model.top() = rotate(model.top(), -radians(angle_x), glm::vec3(1, 0, 0)); //rotating in clockwise direction around x-axis
	//model.top() = rotate(model.top(), -radians(angle_y), glm::vec3(0, 1, 0)); //rotating in clockwise direction around y-axis
	//model.top() = rotate(model.top(), -radians(angle_z), glm::vec3(0, 0, 1)); //rotating in clockwise direction around z-axis

	// ISS
	{
		// Hull
		model.push(model.top());
		{
			setColor(0.5f, 0.5f, 0.5f);

			model.top() = translate(model.top(), vec3(issPosition.x, issPosition.y, issPosition.z));
			model.top() = rotate(model.top(), -radians(90.0f), glm::vec3(1, 0, 0));
			model.top() = scale(model.top(), vec3(0.3f, 1.5f, 0.3f)); // make a small sphere
			glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));
			normalmatrix = transpose(inverse(mat3(view * model.top())));
			glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);
			aCylinder.drawCylinder(drawmode);
		}
		model.pop();

		model.push(model.top());
		{
			model.top() = translate(model.top(), vec3(issPosition.x, issPosition.y, issPosition.z + 1.0f));
			model.top() = rotate(model.top(), -radians(90.0f), glm::vec3(1, 0, 0));
			model.top() = scale(model.top(), vec3(0.15f, 1.5f, 0.15f)); // make a small sphere
			glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));
			normalmatrix = transpose(inverse(mat3(view * model.top())));
			glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);
			aCylinder.drawCylinder(drawmode);
		}
		model.pop();

		model.push(model.top());
		{
			model.top() = translate(model.top(), vec3(issPosition.x, issPosition.y, issPosition.z + 1.6f));
			model.top() = rotate(model.top(), -radians(90.0f), glm::vec3(1, 0, 0));
			model.top() = scale(model.top(), vec3(0.3f, 1.5f, 0.3f)); // make a small sphere
			glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));
			normalmatrix = transpose(inverse(mat3(view * model.top())));
			glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);
			aCylinder.drawCylinder(drawmode);
		}
		model.pop();

		model.push(model.top());
		{

			model.top() = translate(model.top(), vec3(issPosition.x, issPosition.y + 0.9f, issPosition.z + 1.6f));
			model.top() = rotate(model.top(), -radians(90.0f), glm::vec3(0, 1, 0));
			model.top() = scale(model.top(), vec3(0.3f, 1.0f, 0.3f)); // make a small sphere
			glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));
			normalmatrix = transpose(inverse(mat3(view * model.top())));
			glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);
			aCylinder.drawCylinder(drawmode);
		}
		model.pop();

		model.push(model.top());
		{
			model.top() = translate(model.top(), vec3(issPosition.x, issPosition.y + 0.5f, issPosition.z + 1.6f));
			model.top() = rotate(model.top(), -radians(90.0f), glm::vec3(0, 1, 0));
			model.top() = scale(model.top(), vec3(0.15f, 1.0f, 0.15f)); // make a small sphere
			glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));
			normalmatrix = transpose(inverse(mat3(view * model.top())));
			glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);
			aCylinder.drawCylinder(drawmode);
		}
		model.pop();

		model.push(model.top());
		{
			model.top() = translate(model.top(), vec3(issPosition.x, issPosition.y - 0.9f, issPosition.z + 1.6f));
			model.top() = rotate(model.top(), -radians(90.0f), glm::vec3(0, 1, 0));
			model.top() = scale(model.top(), vec3(0.3f, 1.0f, 0.3f)); // make a small sphere
			glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));
			normalmatrix = transpose(inverse(mat3(view * model.top())));
			glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);
			aCylinder.drawCylinder(drawmode);
		}
		model.pop();

		model.push(model.top());
		{
			model.top() = translate(model.top(), vec3(issPosition.x, issPosition.y - 0.5f, issPosition.z + 1.6f));
			model.top() = rotate(model.top(), -radians(90.0f), glm::vec3(0, 1, 0));
			model.top() = scale(model.top(), vec3(0.15f, 1.0f, 0.15f)); // make a small sphere
			glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));
			normalmatrix = transpose(inverse(mat3(view * model.top())));
			glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);
			aCylinder.drawCylinder(drawmode);
		}
		model.pop();

		// Solar Array Rod Left
		model.push(model.top());
		{
			setColor(0.5f, 0.5f, 0.5f);
			// Define the model transformations for the cube
			model.top() = translate(model.top(), vec3(issPosition.x + 0.7f, issPosition.y, issPosition.z + 0.4f));
			model.top() = rotate(model.top(), -radians(panelOneRotation), glm::vec3(1, 0, 0));
			model.top() = scale(model.top(), vec3(2.5f, 0.06f, 0.05f)); // make a small sphere

			glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));
			normalmatrix = transpose(inverse(mat3(view * model.top())));
			glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);
			aCube.drawCube(drawmode);
		}
		model.pop();

		// Solar Array Left
		model.push(model.top());
		{
			setColor(0.2f, 0.2f, 0.2f);

			// Define the model transformations for the cube
			model.top() = translate(model.top(), vec3(issPosition.x + 0.8f, issPosition.y, issPosition.z + 0.4f));
			model.top() = rotate(model.top(), -radians(panelOneRotation), glm::vec3(1, 0, 0));
			model.top() = scale(model.top(), vec3(1.8f, 0.05f, 0.5f)); // make a small sphere

			glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));
			normalmatrix = transpose(inverse(mat3(view * model.top())));
			glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);
			aCube.drawCube(drawmode);
		}
		model.pop();

		// Solar Array Rod Left
		model.push(model.top());
		{
			setColor(0.5f, 0.5f, 0.5f);
			// Define the model transformations for the cube
			model.top() = translate(model.top(), vec3(issPosition.x + 0.7f, issPosition.y, issPosition.z - 0.4f));
			model.top() = rotate(model.top(), -radians(panelOneRotation), glm::vec3(1, 0, 0));
			model.top() = scale(model.top(), vec3(2.5f, 0.06f, 0.05f)); // make a small sphere

			glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));
			normalmatrix = transpose(inverse(mat3(view * model.top())));
			glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);
			aCube.drawCube(drawmode);
		}
		model.pop();

		// Solar Array Left
		model.push(model.top());
		{
			setColor(0.2f, 0.2f, 0.2f);

			// Define the model transformations for the cube
			model.top() = translate(model.top(), vec3(issPosition.x + 0.8f, issPosition.y, issPosition.z - 0.4f));
			model.top() = rotate(model.top(), -radians(panelOneRotation), glm::vec3(1, 0, 0));
			model.top() = scale(model.top(), vec3(1.8f, 0.05f, 0.5f)); // make a small sphere

			glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));
			normalmatrix = transpose(inverse(mat3(view * model.top())));
			glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);
			aCube.drawCube(drawmode);
		}
		model.pop();

		// Solar Array Rod Right
		model.push(model.top());
		{
			setColor(0.5f, 0.5f, 0.5f);
			// Define the model transformations for the cube
			model.top() = translate(model.top(), vec3(issPosition.x - 0.7f, issPosition.y, issPosition.z - 0.4f));
			model.top() = rotate(model.top(), -radians(panelTwoRotation), glm::vec3(1, 0, 0));
			model.top() = scale(model.top(), vec3(2.5f, 0.06f, 0.05f));

			glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));
			normalmatrix = transpose(inverse(mat3(view * model.top())));
			glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);
			aCube.drawCube(drawmode);
		}
		model.pop();

		// Solar Array Right
		model.push(model.top());
		{
			setColor(0.2f, 0.2f, 0.2f);
			// Define the model transformations for the cube
			model.top() = translate(model.top(), vec3(issPosition.x - 0.8f, issPosition.y, issPosition.z - 0.4f));
			model.top() = rotate(model.top(), -radians(panelTwoRotation), glm::vec3(1, 0, 0));
			model.top() = scale(model.top(), vec3(1.8f, 0.05f, 0.5f)); // make a small sphere

			glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));
			normalmatrix = transpose(inverse(mat3(view * model.top())));
			glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);
			aCube.drawCube(drawmode);
		}
		model.pop();

		// Solar Array Rod Right
		model.push(model.top());
		{
			setColor(0.5f, 0.5f, 0.5f);
			// Define the model transformations for the cube
			model.top() = translate(model.top(), vec3(issPosition.x - 0.7f, issPosition.y, issPosition.z + 0.4f));
			model.top() = rotate(model.top(), -radians(panelTwoRotation), glm::vec3(1, 0, 0));
			model.top() = scale(model.top(), vec3(2.5f, 0.06f, 0.05f));

			glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));
			normalmatrix = transpose(inverse(mat3(view * model.top())));
			glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);
			aCube.drawCube(drawmode);
		}
		model.pop();

		// Solar Array Right
		model.push(model.top());
		{
			setColor(0.2f, 0.2f, 0.2f);
			// Define the model transformations for the cube
			model.top() = translate(model.top(), vec3(issPosition.x - 0.8f, issPosition.y, issPosition.z + 0.4f));
			model.top() = rotate(model.top(), -radians(panelTwoRotation), glm::vec3(1, 0, 0));
			model.top() = scale(model.top(), vec3(1.8f, 0.05f, 0.5f)); // make a small sphere

			glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));
			normalmatrix = transpose(inverse(mat3(view * model.top())));
			glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);
			aCube.drawCube(drawmode);
		}
		model.pop();

		mat4 previous;

		// Arm Root
		model.push(model.top());
		{
			setColor(0.2f, 0.2f, 0.5f);

			model.top() = translate(model.top(), vec3(issPosition.x, issPosition.y, issPosition.z - 1.f));
			pivot = vec3(0, 0, 0.25f);

			model.top() = translate(model.top(), pivot);
			model.top() = rotate(model.top(), -radians(armRootRotation.x), glm::vec3(1, 0, 0)); //rotating in clockwise direction around x-axis
			model.top() = rotate(model.top(), -radians(armRootRotation.y), glm::vec3(0, 1, 0)); //rotating in clockwise direction around y-axis
			model.top() = rotate(model.top(), -radians(armRootRotation.z), glm::vec3(0, 0, 1)); //rotating in clockwise direction around z-axis
			model.top() = translate(model.top(), -pivot);

			model.top() = scale(model.top(), vec3(0.2f, 0.2f, 1.f));

			glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));
			normalmatrix = transpose(inverse(mat3(view * model.top())));
			glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

			previous = model.top();

			aCube.drawCube(drawmode);
		}
		model.pop();

		// Arm Joint
		model.push(model.top());
		{
			setColor(0.1f, 0.1f, 0.6f);
			// Define the model transformations for the cube

			model.top() = translate(model.top(), vec3(issPosition.x, issPosition.y, issPosition.z - 1.5f));

			//model.top() = rotate(model.top(), -radians(45.f), glm::vec3(0, 1, 0)); //rotating in clockwise direction around x-axis


			pivot = vec3(0, 0, 0.75f);

			model.top() = translate(model.top(), pivot);
			model.top() = rotate(model.top(), -radians(armRootRotation.x), glm::vec3(1, 0, 0)); //rotating in clockwise direction around x-axis
			model.top() = rotate(model.top(), -radians(armRootRotation.y), glm::vec3(0, 1, 0)); //rotating in clockwise direction around y-axis
			model.top() = rotate(model.top(), -radians(armRootRotation.z), glm::vec3(0, 0, 1)); //rotating in clockwise direction around z-axis
			model.top() = translate(model.top(), -pivot);

			pivot = vec3(0, 0, 0.25f);

			model.top() = translate(model.top(), pivot);
			model.top() = rotate(model.top(), -radians(armJointRotation.x), glm::vec3(1, 0, 0)); //rotating in clockwise direction around x-axis
			model.top() = rotate(model.top(), -radians(armJointRotation.y), glm::vec3(0, 1, 0)); //rotating in clockwise direction around y-axis
			model.top() = rotate(model.top(), -radians(armJointRotation.z), glm::vec3(0, 0, 1)); //rotating in clockwise direction around z-axis
			model.top() = translate(model.top(), -pivot);

			// local rotation relative to parent was something like parent rotation times inverse of local rotation 

			model.top() = scale(model.top(), vec3(0.2f, 0.2f, 1.f));

			// Send the model uniform and normal matrix to the currently bound shader,
			glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));

			// Recalculate the normal matrix and send to the vertex shader
			normalmatrix = transpose(inverse(mat3(view * model.top())));
			glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

			previous = model.top();

			aCube.drawCube(drawmode);
		}
		model.pop();

		// Arm Tip
		model.push(model.top());
		{
			setColor(0.9f, 0.9f, 0.9f);
			// Define the model transformations for the cube

			model.top() = translate(model.top(), vec3(issPosition.x, issPosition.y, issPosition.z - 2.0f));

			//model.top() = rotate(model.top(), -radians(45.f), glm::vec3(0, 1, 0)); //rotating in clockwise direction around x-axis


			pivot = vec3(0, 0, 1.25f);

			model.top() = translate(model.top(), pivot);
			model.top() = rotate(model.top(), -radians(armRootRotation.x), glm::vec3(1, 0, 0)); //rotating in clockwise direction around x-axis
			model.top() = rotate(model.top(), -radians(armRootRotation.y), glm::vec3(0, 1, 0)); //rotating in clockwise direction around y-axis
			model.top() = rotate(model.top(), -radians(armRootRotation.z), glm::vec3(0, 0, 1)); //rotating in clockwise direction around z-axis
			model.top() = translate(model.top(), -pivot);

			pivot = vec3(0, 0, 0.75f);

			model.top() = translate(model.top(), pivot);
			model.top() = rotate(model.top(), -radians(armJointRotation.x), glm::vec3(1, 0, 0)); //rotating in clockwise direction around x-axis
			model.top() = rotate(model.top(), -radians(armJointRotation.y), glm::vec3(0, 1, 0)); //rotating in clockwise direction around y-axis
			model.top() = rotate(model.top(), -radians(armJointRotation.z), glm::vec3(0, 0, 1)); //rotating in clockwise direction around z-axis
			model.top() = translate(model.top(), -pivot);

			pivot = vec3(0, 0, 0.25f);

			model.top() = translate(model.top(), pivot);
			model.top() = rotate(model.top(), -radians(armTipRotation.x), glm::vec3(1, 0, 0)); //rotating in clockwise direction around x-axis
			model.top() = rotate(model.top(), -radians(armTipRotation.y), glm::vec3(0, 1, 0)); //rotating in clockwise direction around y-axis
			model.top() = rotate(model.top(), -radians(armTipRotation.z), glm::vec3(0, 0, 1)); //rotating in clockwise direction around z-axis
			model.top() = translate(model.top(), -pivot);

			// local rotation relative to parent was something like parent rotation times inverse of local rotation 

			model.top() = scale(model.top(), vec3(0.2f, 0.2f, 1.f));

			// Send the model uniform and normal matrix to the currently bound shader,
			glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));

			// Recalculate the normal matrix and send to the vertex shader
			normalmatrix = transpose(inverse(mat3(view * model.top())));
			glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

			previous = model.top();

			aCube.drawCube(drawmode);
		}
		model.pop();

		// Arm Tip
		model.push(model.top());
		{
			setColor(0.9f, 0.9f, 0.9f);
			// Define the model transformations for the cube

			model.top() = translate(model.top(), vec3(issPosition.x, issPosition.y, issPosition.z - 2.25f));

			//model.top() = rotate(model.top(), -radians(45.f), glm::vec3(0, 1, 0)); //rotating in clockwise direction around x-axis


			pivot = vec3(0, 0, 1.5f);

			model.top() = translate(model.top(), pivot);
			model.top() = rotate(model.top(), -radians(armRootRotation.x), glm::vec3(1, 0, 0)); //rotating in clockwise direction around x-axis
			model.top() = rotate(model.top(), -radians(armRootRotation.y), glm::vec3(0, 1, 0)); //rotating in clockwise direction around y-axis
			model.top() = rotate(model.top(), -radians(armRootRotation.z), glm::vec3(0, 0, 1)); //rotating in clockwise direction around z-axis
			model.top() = translate(model.top(), -pivot);

			pivot = vec3(0, 0, 1.0f);

			model.top() = translate(model.top(), pivot);
			model.top() = rotate(model.top(), -radians(armJointRotation.x), glm::vec3(1, 0, 0)); //rotating in clockwise direction around x-axis
			model.top() = rotate(model.top(), -radians(armJointRotation.y), glm::vec3(0, 1, 0)); //rotating in clockwise direction around y-axis
			model.top() = rotate(model.top(), -radians(armJointRotation.z), glm::vec3(0, 0, 1)); //rotating in clockwise direction around z-axis
			model.top() = translate(model.top(), -pivot);

			pivot = vec3(0, 0, 0.5f);

			model.top() = translate(model.top(), pivot);
			model.top() = rotate(model.top(), -radians(armTipRotation.x), glm::vec3(1, 0, 0)); //rotating in clockwise direction around x-axis
			model.top() = rotate(model.top(), -radians(armTipRotation.y), glm::vec3(0, 1, 0)); //rotating in clockwise direction around y-axis
			model.top() = rotate(model.top(), -radians(armTipRotation.z), glm::vec3(0, 0, 1)); //rotating in clockwise direction around z-axis
			model.top() = translate(model.top(), -pivot);

			model.top() = scale(model.top(), vec3(0.2f, 0.2f, 0.2f));
			glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));
			normalmatrix = transpose(inverse(mat3(view * model.top())));
			glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);
			previous = model.top();
			aClaw.drawClaw(drawmode);
		}
		model.pop();
	}

	// This block of code draws the sphere
	model.push(model.top());
	{
		model.top() = translate(model.top(), vec3(-x - 0.5f, 0, 0));
		model.top() = scale(model.top(), vec3(model_scale / 3.f, model_scale / 3.f, model_scale / 3.f));//scale equally in all axis

		glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

		//aSphere.drawSphere(drawmode); // Draw our sphere
	}
	model.pop();

	glDisableVertexAttribArray(0);
	glUseProgram(0);

	/* Modify our animation variables */
	angle_x += angle_inc_x;
	angle_y += angle_inc_y;
	angle_z += angle_inc_z;
}

/* Called whenever the window is resized. The new window size is given, in pixels. */
static void reshape(GLFWwindow* window, int w, int h)
{
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	aspect_ratio = ((float)w / 640.f * 4.f) / ((float)h / 480.f * 3.f);
}

/* change view angle, exit upon ESC */
static void keyCallback(GLFWwindow* window, int key, int s, int action, int mods)
{
	/* Enable this call if you want to disable key responses to a held down key*/
	//if (action != GLFW_PRESS) return;

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	//cout << "X: " << armRootRotation.x << " Y: " << armRootRotation.y << " Z: " << armRootRotation.z << endl;

	if (key == 'W') armRootRotation.x -= speed;
	if (key == 'S') armRootRotation.x += speed;
	if (key == 'A') armRootRotation.y -= speed;
	if (key == 'D') armRootRotation.y += speed;

	if (key == 'Q') armRootRotation.z -= speed;
	if (key == 'E') armRootRotation.z += speed;

	// TODO: Limit rotations

	if (key == 'T') armJointRotation.x -= speed;
	if (key == 'G') armJointRotation.x += speed;
	if (key == 'F') armJointRotation.y -= speed;
	if (key == 'H') armJointRotation.y += speed;

	if (key == 'I') armTipRotation.x -= speed;
	if (key == 'K') armTipRotation.x += speed;
	if (key == 'J') armTipRotation.y -= speed;
	if (key == 'L') armTipRotation.y += speed;

	//if (armRootRotation.x > 360 || armRootRotation.x < -360) armRootRotation.x = 0;
	//if (armRootRotation.y > 360 || armRootRotation.y < -360) armRootRotation.y = 0;
	//if (armRootRotation.z > 360 || armRootRotation.z < -360) armRootRotation.z = 0;

	//if (abs(armRootRotation.y) < 90 && armJointRotation.x > 90) armJointRotation.x = 90;

	//if (key == 'A') model_scale -= speed / 0.5f;
	//if (key == 'S') model_scale += speed / 0.5f;
	if (key == 'Z') panelOneRotation -= speed;
	if (key == 'X') panelOneRotation += speed;
	if (key == 'C') panelTwoRotation -= speed;
	if (key == 'V') panelTwoRotation += speed;

	if (key == '1') lightPosition.x -= speed / 30;
	if (key == '2') lightPosition.x += speed / 30;
	if (key == '3') lightPosition.y -= speed / 30;
	if (key == '4') lightPosition.y += speed / 30;
	if (key == '5') lightPosition.z -= speed / 30;
	if (key == '6') lightPosition.z += speed / 30;

	if (key == '7') vx -= 1.f;
	if (key == '8') vx += 1.f;
	if (key == '9') vy -= 1.f;
	if (key == '0') vy += 1.f;
	if (key == '-') vz -= 1.f;
	if (key == '=') vz += 1.f;

	if (key == GLFW_KEY_UP) issPosition.z -= 0.05f;
	if (key == GLFW_KEY_DOWN) issPosition.z += 0.05f;
	if (key == GLFW_KEY_RIGHT) issPosition.x += 0.05f;
	if (key == GLFW_KEY_LEFT) issPosition.x -= 0.05f;


	if (key == GLFW_KEY_SPACE && action != GLFW_PRESS)
	{
		angle_inc_x = 0;
		angle_inc_y = 0;
		angle_inc_z = 0;
	}

	if (key == 'M' && action != GLFW_PRESS)
	{
		colourmode = !colourmode;
		cout << "colourmode=" << colourmode << endl;
	}

	/* Turn attenuation on and off */
	if (key == '.' && action != GLFW_PRESS)
	{
		attenuationmode = !attenuationmode;
	}

	/* Cycle between drawing vertices, mesh and filled polygons */
	if (key == ',' && action != GLFW_PRESS)
	{
		drawmode++;
		if (drawmode > 2) drawmode = 0;
	}
}


/* Entry point of program */
int main(int argc, char* argv[])
{
	GLWrapper* glw = new GLWrapper(1024, 768, "Assignment One - Marius Urbelis");
	//GLWrapper* glw = new GLWrapper(1920 * 1.5f, 1080 * 1.5f, "Assignment One - Marius Urbelis");

	if (!ogl_LoadFunctions())
	{
		fprintf(stderr, "ogl_LoadFunctions() failed. Exiting\n");
		return 0;
	}

	glw->setRenderer(display);
	glw->setKeyCallback(keyCallback);
	glw->setKeyCallback(keyCallback);
	glw->setReshapeCallback(reshape);

	/* Output the OpenGL vendor and version */
	glw->DisplayVersion();

	init(glw);

	glw->eventLoop();

	delete(glw);
	return 0;
}

void printInstructions()
{
	cout << endl << endl << endl << " Program controls:" << endl << endl << endl << endl;
	cout << " Canadarm controls:" << endl << endl;
	cout << " Q W E" << "   " << "  T  " << "   " << "  I  " << "   " << endl;
	cout << " A S D" << "   " << "F G H" << "   " << "J K L" << "   " << endl << endl << endl << endl;

	cout << " Solar panels controls:" << endl << endl;
	cout << " Z X" << "   " << "C V" << endl << endl << endl << endl;

	cout << " Light controls:" << endl << endl;
	cout << " 1 2 3 4 5 6" << endl << endl << endl << endl;

	cout << " Camera controls:" << endl << endl;
	cout << " 7 8 9 0 - =" << endl << endl << endl;
}

void setColor(float red, float green, float blue)
{
	partColor = vec3(red, green, blue);
	glUniform3fv(partColorID, 1, value_ptr(partColor));
}