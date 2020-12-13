// Basic fragment shader to demosntrate combining texturing with 
// vertex lighting
// Iain Martin 2018

#version 400

in vec4 fcolour;
in vec2 ftexcoord;
out vec4 outputColor;

in vec4 vertexPosition;
in vec4 color;
in vec3 lightDirection;
in vec3 vertexNormal;

uniform sampler2D tex1;
uniform uint colourmode, emitmode, attenuationmode;
uniform float sunPower;

vec3 specular_albedo = vec3(1.0, 0.8, 0.6);
vec3 global_ambient = vec3(0.05, 0.05, 0.05);
int  shininess = 50;

void main()
{
	vec4 texcolour = texture(tex1, ftexcoord);
	//outputColor = fcolour * texcolour;

	//outputColor = texcolour;

	vec3 emissive = vec3(0);				// Create a vec3(0, 0, 0) for our emmissive light
	vec4 position_h = vertexPosition;	// Convert the (x,y,z) position to homogeneous coords (x,y,z,w)
	vec4 diffuse_albedo;					// This is the vertex colour, used to handle the colourmode change
	vec3 light_pos3 = lightDirection.xyz;			

	float distanceToLight = length(lightDirection);

	vec3 normalizedLightPosition = normalize(light_pos3);
	vec3 normalizedLightVector = normalize(lightDirection);

	// Calculate the diffuse component
	vec3 diffuse = max(dot(vertexNormal, normalizedLightPosition), 0.0) * color.xyz;

	vec3 V = normalize(-vertexPosition.xyz);	
	vec3 R = reflect(-normalizedLightVector, vertexNormal);
	vec3 specular = pow(max(dot(R, V), 0.0), shininess) * specular_albedo;

	float attenuation;
	if (attenuationmode != 1)
	{
		attenuation = 1.0;
	}
	else
	{
		// Define attenuation constants. These could be uniforms for greater flexibility
		float attenuation_k1 = sunPower;
		float attenuation_k2 = sunPower;
		float attenuation_k3 = sunPower;
		attenuation = 1.0 / (attenuation_k1 + attenuation_k2 * distanceToLight + attenuation_k3 * pow(distanceToLight, 2));
	}



	if (emitmode == 1) emissive = vec3(1.0, 1.0, 0.8);
	
	vec3 ambient = color.xyz * 0.2;


	outputColor = vec4(attenuation * (ambient + diffuse + specular + texcolour.xyz) + emissive, 1.0f);
}