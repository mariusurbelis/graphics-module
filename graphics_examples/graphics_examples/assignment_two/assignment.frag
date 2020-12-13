#version 400

in vec4 fcolour;
in vec2 ftexcoord;
in float distanceToLight;
out vec4 outputColor;

in vec4 vertexPosition;
in vec3 lightVector;
in vec3 vertexNormal;

uniform sampler2D tex1;
uniform uint colourmode, emitmode, attenuationmode, specularmode;
uniform float sunPower;

vec3 specular_albedo = vec3(1.0, 0.8, 0.6);
vec3 global_ambient = vec3(0.05, 0.05, 0.05);
int shininess = 80;

void main()
{
	vec4 texcolour = texture(tex1, ftexcoord);

	vec3 diffuse = max(dot(vertexNormal, lightVector), 0.0) * texcolour.xyz;

	vec3 emissive = vec3(0);
	if (emitmode == 1) emissive = vec3(1.0, 1.0, 0.8);
	
	vec3 V = normalize(-vertexPosition.xyz);
	vec3 R = reflect(-normalize(lightVector), normalize(vertexNormal));
	vec3 specular = pow(max(dot(R, V), 0.0), shininess) * specular_albedo;

	float attenuation;
	
	float attenuation_k1 = sunPower;
	float attenuation_k2 = sunPower;
	float attenuation_k3 = sunPower;
	
	attenuation = 1.0 / (attenuation_k1 +
						 attenuation_k2 * distanceToLight +
						 attenuation_k3 * pow(distanceToLight, 2));


	vec3 ambient = texcolour.xyz * 0.3;
	
	if (specularmode == 1)
		outputColor = vec4(attenuation * (diffuse + ambient + specular) + emissive, 1.0f);
	else
		outputColor = vec4(attenuation * (diffuse + ambient) + emissive, 1.0f);
}