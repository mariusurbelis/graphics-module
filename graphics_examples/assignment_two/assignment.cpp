#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "glloadD.lib")
#pragma comment(lib, "opengl32.lib")
#include "wrapper_glfw.h"
#include <iostream>
#include <string>
#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtc/type_ptr.hpp>
#include "tiny_loader_texture.h"
#include "sphere_tex.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <stack>
#include "assignment.h"
#include <cube_tex.h>

#include <chrono>

#define GROUND_OFFSET 3.33
#define ROCK_WALL_OFFSET_X 5.85
#define ROCK_WALL_OFFSET_Y 3.3

using namespace std;
using namespace glm;

bool LoadTexture(string filename, GLuint& texID, bool bGenMipmaps);
void DrawModel(TinyObjLoader object, GLuint textureID, vec3 position, vec3 rotation, float size, bool shiny, bool emissive);
void SetShiny(bool active);
void SetEmissive(bool active);

GLuint program, shadow;
GLuint vao;
GLuint colourmode;

GLfloat angle_x, angle_inc_x, scaler;
GLfloat angle_y, angle_inc_y, angle_z, angle_inc_z;
GLuint drawmode;

vec4 lightPosition = vec4(0.6f, 2.f, 3.f, 1.0f);
vec3 cameraPosition = vec3(0.f, 0.8f, 5.f);
vec3 buddhaPosition = vec3(0, 0, 0);

GLfloat sunPower = 0.05f;

GLuint modelID, viewID, projectionID, lightposID, sunPowerID;
GLuint modelShadowID, viewShadowID, projectionShadowID;
GLuint colourmodeID, emitmodeID, specularmodeID;

GLfloat aspect_ratio;

TinyObjLoader buddhaObject, squirrelObject, blockObject, rockWall, katana, bookshelf;

Sphere aSphere(false);
Cube aCube;

GLuint texID, groundTextureID, squirrelTextureID, rockTextureID, bookshelfTextureID;

stack<mat4> model;

double offset = 0;
int buddhaPosAngle = 0;

void init(GLWrapper* glw)
{
	/* Set the object transformation controls to their initial values */
	angle_x = angle_y = angle_z = 0;
	angle_inc_x = angle_inc_y = angle_inc_z = 0;
	scaler = 1.f;
	aspect_ratio = 1.3333f;
	colourmode = 0;

	// Generate index (name) for one vertex array object
	glGenVertexArrays(1, &vao);

	// Create the vertex array object and make it current
	glBindVertexArray(vao);

	/* Load and create our object*/
	buddhaObject.load_obj("Models/Buddha/buddha.obj");
	blockObject.load_obj("Models/Ground/ground.obj");
	rockWall.load_obj("Models/Rock Wall/rock-wall.obj");
	katana.load_obj("Models/Katana/katana.obj");
	bookshelf.load_obj("Models/Books/books.obj");


	// Creater the sphere (params are num_lats and num_longs)
	aSphere.makeSphere(60, 60);


	/* Load and build the vertex and fragment shaders */
	try
	{
		program = glw->LoadShader("assignment.vert", "assignment.frag");
		shadow = glw->LoadShader("shadow.vert", "shadow.frag");
	}
	catch (exception& e)
	{
		cout << "Caught exception: " << e.what() << endl;
		cin.ignore();
		exit(0);
	}

	//stbi_set_flip_vertically_on_load(true);
	LoadTexture("Ground/ground-2.jpg", groundTextureID, true);
	LoadTexture("Rock Wall/Maps/2.jpg", rockTextureID, true);
	LoadTexture("Books/uv.png", bookshelfTextureID, true);

	aCube.makeCube();

	/* Define uniforms to send to vertex shader */
	modelID = glGetUniformLocation(program, "model");
	emitmodeID = glGetUniformLocation(program, "emitmode");
	colourmodeID = glGetUniformLocation(program, "colourmode");
	specularmodeID = glGetUniformLocation(program, "specularmode");
	viewID = glGetUniformLocation(program, "view");
	projectionID = glGetUniformLocation(program, "projection");

	modelShadowID = glGetUniformLocation(shadow, "model");
	viewShadowID = glGetUniformLocation(shadow, "view");
	projectionShadowID = glGetUniformLocation(shadow, "projection");

	sunPowerID = glGetUniformLocation(program, "sunPower");
	lightposID = glGetUniformLocation(program, "lightpos");

	model.push(mat4(1.0f));
}

// Image parameters
int width, height, nrChannels;

// Calculate shadow projection matrix from Light pos (or direction)
// and the plane equatino coefficients.
// This matrix works both for direction and positional lights.
mat4 shadow_matrix(vec4 L, vec4 P)
{
	//dot product of normal and light position/direction
	GLfloat rdotl = P.x * L.x + P.y * L.y + P.z * L.z + P.w * L.w;

	// Define the shadow projection matix
	return mat4(-P.x * L.x + rdotl,
		-P.x * L.y,
		-P.x * L.z,
		-P.x * L.w,
		-P.y * L.x,
		-P.y * L.y + rdotl,
		-P.y * L.z,
		-P.y * L.w,
		-P.z * L.x,
		-P.z * L.y,
		-P.z * L.z + rdotl,
		-P.z * L.w,
		-P.w * L.x,
		-P.w * L.y,
		-P.w * L.z,
		-P.w * L.w + rdotl);
}

/**
 * Function to use stb_image to load a texture, generate a texture ID and defined
 * the texture from the image.
 * Mipmaps can be generated if the flag is set to true.
 */
bool LoadTexture(string filename, GLuint& texID, bool bGenMipmaps)
{
	glGenTextures(1, &texID);
	// local image parameters
	int width, height, nrChannels;

	/* load an image file using stb_image */
	unsigned char* data = stbi_load(("Models/" + filename).c_str(), &width, &height, &nrChannels, 0);

	// check for an error during the load process
	if (data)
	{
		// Note: this is not a full check of all pixel format types, just the most common two!
		int pixel_format = 0;
		if (nrChannels == 3)
			pixel_format = GL_RGB;
		else
			pixel_format = GL_RGBA;

		// Bind the texture ID before the call to create the texture.
			// texID[i] will now be the identifier for this specific texture
		glBindTexture(GL_TEXTURE_2D, texID);

		// Create the texture, passing in the pointer to the loaded image pixel data
		glTexImage2D(GL_TEXTURE_2D, 0, pixel_format, width, height, 0, pixel_format, GL_UNSIGNED_BYTE, data);

		// Generate Mip Maps
		if (bGenMipmaps)
		{
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else
		{
			// If mipmaps are not used then ensure that the min filter is defined
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		}
	}
	else
	{
		//printf("stb_image  loading error: filename=%s", filename);
		return false;
	}
	stbi_image_free(data);
	return true;
}

void SetShiny(bool active)
{
	glUniform1ui(specularmodeID, active ? 1 : 0);
}

void SetEmissive(bool active)
{
	glUniform1ui(emitmodeID, active ? 1 : 0);
}

mat3 normalmatrix;

void DrawWithShadow(mat4 view, TinyObjLoader object, GLuint textureID, vec3 objectPosition, float scaler)
{
	// Draw the object (monkey), with a projected shadow, spinning around it's y-axis
	model.push(model.top());
	{
		// Draw the shadow
		model.push(model.top());
		{
			// Switch to our shadow renderer shader
			glUseProgram(shadow);

			model.top() = translate(model.top(), vec3(0, -0.19f, 0));

			// Calculate the shadow matrix from light position and plane normal
			mat4 shadow = shadow_matrix(lightPosition - vec4(objectPosition, 1.0), vec4(0, 1.0, 0, 0.0));
			//model.top() = scale(model.top(), vec3(scaler, scaler, scaler));
			model.top() = model.top() * shadow;


			model.top() = translate(model.top(), vec3(objectPosition.x, objectPosition.y + 0.19f, objectPosition.z));

			// Send our current model, view and projection ONLY to our shadow matrix
			glUniformMatrix4fv(modelShadowID, 1, GL_FALSE, &(model.top()[0][0]));
			glUniformMatrix4fv(viewShadowID, 1, GL_FALSE, &view[0][0]);

			/* Draw our shadow object */
			object.drawObject(drawmode);

			/* Make the compiled shader program current again */
			glUseProgram(program);
		}
		model.pop();

		DrawModel(object, textureID, objectPosition, vec3(0, 0, 0), scaler, true, false);
	}
	model.pop();
}

void DrawModel(TinyObjLoader object, GLuint textureID, vec3 position, vec3 rotation, float size, bool shiny, bool emissive)
{
	model.push(model.top());
	{
		model.top() = translate(model.top(), position);
		model.top() = scale(model.top(), vec3(size / 3.f, size / 3.f, size / 3.f));
		model.top() = rotate(model.top(), -radians(rotation.x), vec3(1, 0, 0));
		model.top() = rotate(model.top(), -radians(rotation.y), vec3(0, 1, 0));
		model.top() = rotate(model.top(), -radians(rotation.z), vec3(0, 0, 1));

		glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));
		//glUniformMatrix4fv(modelShadowID, 1, GL_FALSE, &(model.top()[0][0]));

		glBindTexture(GL_TEXTURE_2D, textureID);

		SetShiny(shiny);
		SetEmissive(emissive);

		object.drawObject(drawmode);

		SetShiny(!shiny);
		SetEmissive(!emissive);

		glBindTexture(GL_TEXTURE_2D, 0);
	}
	model.pop();
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

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	mat4 projection = perspective(radians(30.0f), aspect_ratio, 0.1f, 100.0f);

	// Camera matrix
	mat4 view = lookAt(
		cameraPosition, // Camera is at (0,0,4), in World Space
		vec3(0, 1, 0), // and looks at the origin
		vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
	);

	buddhaPosAngle++;
	buddhaPosition.y = 0.1 * sin(buddhaPosAngle * 3.14 / 180);
	if (buddhaPosAngle >= 360) buddhaPosAngle = 0;

	glUniform1ui(colourmodeID, colourmode);
	glUniformMatrix4fv(viewID, 1, GL_FALSE, &view[0][0]);
	glUniform1f(sunPowerID, sunPower);
	glUniformMatrix4fv(projectionID, 1, GL_FALSE, &projection[0][0]);
	glUniform1ui(specularmodeID, 0);
	glUniform1ui(emitmodeID, 0);

	vec4 light = view * lightPosition;
	glUniform4fv(lightposID, 1, value_ptr(light));

	glUseProgram(shadow);

	glUniformMatrix4fv(viewShadowID, 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(projectionShadowID, 1, GL_FALSE, &projection[0][0]);

	glUseProgram(program);

	DrawWithShadow(view, buddhaObject, rockTextureID, buddhaPosition, 2);
	DrawWithShadow(view, bookshelf, bookshelfTextureID, vec3(-3, -0.08, -6.2), 3);
	DrawWithShadow(view, bookshelf, bookshelfTextureID, vec3(3, -0.08, -6.2), 3);
	DrawWithShadow(view, katana, rockTextureID, vec3(0, -0.05, -6.2), 3);

	//DrawModel(squirrelObject, squirrelTextureID, vec3(x - 0.5f, y, z), vec3(angle_x, angle_y, angle_z), 1, false, false);

	for (int x = -9; x < 9; x++)
		for (int y = -6; y < 10; y++)
			DrawModel(blockObject, groundTextureID, vec3(GROUND_OFFSET * x, -0.2f, GROUND_OFFSET * y), vec3(0, 0, 0), 0.5, false, false);

	for (int x = -3; x <= 3; x++)
		for (int y = -1; y < 5; y++)
			DrawModel(rockWall, rockTextureID, vec3(ROCK_WALL_OFFSET_X * x, ROCK_WALL_OFFSET_Y * y, -20), vec3(0, 180, 0), 1, false, false);

	for (int z = -3; z <= 3; z++)
		for (int y = -1; y < 5; y++)
			DrawModel(rockWall, rockTextureID, vec3(ROCK_WALL_OFFSET_X * 3, ROCK_WALL_OFFSET_Y * y, ROCK_WALL_OFFSET_X * z), vec3(0, -90, 0), 1, false, false);

	for (int z = -3; z <= 3; z++)
		for (int y = -1; y < 5; y++)
			DrawModel(rockWall, rockTextureID, vec3(ROCK_WALL_OFFSET_X * -3, ROCK_WALL_OFFSET_Y * y, ROCK_WALL_OFFSET_X * z), vec3(0, 90, 0), 1, false, false);


	model.push(model.top());
	{
		model.top() = translate(model.top(), vec3(lightPosition.x, lightPosition.y, lightPosition.z));
		model.top() = scale(model.top(), vec3(0.01f, 0.01f, 0.01f));
		model.top() = rotate(model.top(), -radians(angle_x), vec3(1, 0, 0));
		model.top() = rotate(model.top(), -radians(angle_y), vec3(0, 1, 0));
		model.top() = rotate(model.top(), -radians(angle_z), vec3(0, 0, 1));

		glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));

		/* Draw our sphere */
		/* Note that you probably want a different texture for this Sphere! */
		glBindTexture(GL_TEXTURE_2D, texID);
		SetEmissive(true);
		aSphere.drawSphere(drawmode);
		SetEmissive(false);
		glBindTexture(GL_TEXTURE_2D, 0);
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

bool enable_repeat = true;

/* change view angle, exit upon ESC */
static void keyCallback(GLFWwindow* window, int key, int s, int action, int mods)
{
	/* Enable this call if you want to disable key responses to a held down key*/
	if (action != GLFW_PRESS && !enable_repeat) return;

	//if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) enable_repeat = !enable_repeat;

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == 'W' && cameraPosition.z > 1) cameraPosition.z -= 0.05f;
	if (key == 'S' && cameraPosition.z < 15) cameraPosition.z += 0.05f;
	if (key == 'A' && cameraPosition.x > -15) cameraPosition.x -= 0.05f;
	if (key == 'D' && cameraPosition.x < 15) cameraPosition.x += 0.05f;

	/*
	if (key == 'O' || key == '9')
	{
		offset -= key == '9' ? 0.1f : 0.01f;
		cout << "Offset: " << offset << endl;
	}
	if (key == 'P' || key == '0')
	{
		offset += key == '0' ? 0.1f : 0.01f;
		cout << "Offset: " << offset << endl;
	}
	*/

	if (key == GLFW_KEY_UP) lightPosition.z -= 0.05f;
	if (key == GLFW_KEY_DOWN) lightPosition.z += 0.05f;
	if (key == GLFW_KEY_RIGHT) lightPosition.x += 0.05f;
	if (key == GLFW_KEY_LEFT) lightPosition.x -= 0.05f;
	if (key == GLFW_KEY_HOME) lightPosition.y += 0.05f;
	if (key == GLFW_KEY_END) lightPosition.y -= 0.05f;

	/*
	if (key == 'M' && action != GLFW_PRESS)
	{
		colourmode = !colourmode;
		cout << "colourmode=" << colourmode << endl;
	}

	if (key == 'N' && action != GLFW_PRESS)
	{
		drawmode++;
		if (drawmode > 2) drawmode = 0;
	}
	*/

}

void printInstructions()
{
	cout << endl << endl << endl << " Program controls:" << endl << endl << endl << endl;
	cout << " Camera movement (always looks at the center - Buddha statue):" << endl << endl;
	cout << "   W  " << "     |       North     " << endl;
	cout << " A S D" << "     |  West South East   " << endl << endl << endl << endl;

	cout << " Light controls (arrows):" << endl << endl;
	cout << "       Up  " << "               Home     " << endl;
	cout << " Left Down Right" << "          End   " << endl << endl << endl << endl;
}

/* Entry point of program */
int main(int argc, char* argv[])
{
	GLWrapper* glw = new GLWrapper(1920 * 0.8f, 1080 * 0.9f, "Assignment Two - Marius Urbelis");
	//GLWrapper *glw = new GLWrapper(1920 * 1.5f, 1080 * 1.5f, "Assignment Two - Marius Urbelis");

	if (!ogl_LoadFunctions())
	{
		fprintf(stderr, "ogl_LoadFunctions() failed. Exiting\n");
		return 0;
	}

	glw->setRenderer(display);
	glw->setKeyCallback(keyCallback);
	glw->setReshapeCallback(reshape);

	init(glw);

	printInstructions();

	glw->eventLoop();

	delete(glw);
	return 0;
}