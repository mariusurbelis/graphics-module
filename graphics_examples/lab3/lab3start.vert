// Starter vertex shader for lab3
// The goal is to update this shader to implement Gourand shading
// which is per-vertex lighting

#version 420

// These are the vertex attributes
layout(location = 0) in vec3 position;
layout(location = 1) in vec4 colour;
layout(location = 2) in vec3 normal;

// Uniform variables are passed in from the application
uniform mat4 model, view, projection;
uniform uint colourmode;

// Output the vertex colour - to be rasterized into pixel fragments
out vec4 fcolour;

//vec4 ambient = vec4(0.2, 0.2, 0.2, 1.0);

void main()
{
	vec4 diffuse_colour;
	vec4 position_h = vec4(position, 1.0);

	vec3 lightDirection = vec3(0, 1, 1);

	mat4 model_view = model * view;

	vec4 vertexPosition = vec4(position, 1.0);

	mat3 normal_matrix = transpose(inverse(mat3(model_view)));

	vec3 normalVector = normal_matrix * normal;

	lightDirection = normalize(lightDirection);
	normalVector = normalize(normalVector);

	float dotProduct = max(dot(lightDirection, normalVector), 0);

	if (colourmode == 1)
	{
		diffuse_colour = colour;
	}
	else
	{
		diffuse_colour = vec4(0.568, 0.568, 0.568, 1.0);
	}

	vec4 ambient = vec4(vec3(1.0) * 0.15, 1.0) * diffuse_colour;

	// V is the unit length direction vector of the vertex position to the origin
	// P is the vertex position transformed by the model-view matrix.
	// R is the reflected light beam of the plane defined by the vertex normal
	// Specular colour is an RGB colour (e.g. white)
	vec3 V = normalize(-(vertexPosition * model_view).xyz);
	vec3 R = reflect(lightDirection, normal);
	vec3 specular = pow(max(dot(R, V), 0.0), 8) * vec3(1, 0, 0);

	// Define the vertex colour
	//fcolour = (gl_VertexID % 2 == 0) ? ambient * diffuse_colour : diffuse_colour;
	//fcolour = diffuse_colour * 0.01 * gl_VertexID;
	//fcolour = diffuse_colour * 0.2;

	fcolour = diffuse_colour * dotProduct + ambient + vec4(specular, 1.0);

	// Define the vertex position
	gl_Position = projection * view * model * position_h;
}

