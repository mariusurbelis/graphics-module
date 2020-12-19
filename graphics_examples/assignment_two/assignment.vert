#version 400

// These are the vertex attributes
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texcoord;

// Uniform variables are passed in from the application
uniform mat4 model, view, projection;
uniform uint colourmode;
uniform vec4 lightpos;

// Output the vertex colour - to be rasterized into pixel fragments
out vec4 vertexPosition;
out vec3 lightVector;
out vec3 vertexNormal;

// Output a texture coordinate as a vertex attribute
out vec2 ftexcoord;
out float distanceToLight;

void main()
{
	vec4 position_h = vec4(position, 1.0);
	
	mat4 mv_matrix = view * model;

	vertexNormal = normalize(transpose(inverse(mat3(mv_matrix))) * normal);
	vertexPosition = mv_matrix * position_h;

	lightVector = lightpos.xyz - vertexPosition.xyz;
	distanceToLight = length(lightVector);
	lightVector = normalize(lightVector);
	//lightVector = normalize(lightpos.xyz);
	
	ftexcoord = texcoord;

	gl_Position = projection * mv_matrix * position_h;
}