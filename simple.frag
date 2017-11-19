#version 330 core

// intepolated values from vertex shader
in vec2 UV;

// Ouput data
out vec3 color;

// constant values for the whole mesh
uniform sampler2D myTextureSampler;


void main()
{
    // red color
//	color = vec3(1.0,0.0,0.0);

    color = vec3(UV.x, UV.y, 0);

//	color = texture(myTextureSampler, UV).rgb;
}