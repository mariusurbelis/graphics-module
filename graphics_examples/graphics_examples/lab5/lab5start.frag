// Lab5 Solution
// Fragment shader which combines colour and texture.
// The colour is the lighting colour outputted by the vertex shader
// Iain Martin 2019

#version 420

in vec4 fcolour;
out vec4 outputColor;
in vec2 ftexcoord;

uniform sampler2D tex1;	

void main()
{
	// Extract the texture colour to colour our pixel
	vec4 texcolour = texture(tex1, ftexcoord);

	// Set the poixel colour to be a combination of our lit colour and the texture
	outputColor = fcolour * texcolour;
}