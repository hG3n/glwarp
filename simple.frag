#version 330 core

// Interpolated values from the vertex shaders
in vec2 UV;

// Ouput data
out vec4 color;

// Values that stay constant for the whole mesh.
uniform sampler2D myTextureSampler;

void main(){
	// Output color = color of the texture at the specified UV
	//color = texture( myTextureSampler, UV).bgra;
	color = texture( myTextureSampler, vec2(UV.x, 1.0f- UV.y)).bgra;
    //	color = vec3(1.0, 0.0, 0.0);
	}