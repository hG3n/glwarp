#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec2 vertexUV;

out vec2 UV;

uniform mat4 MVP;

void main(){

    // output position of vertex
    gl_Position = MVP * vec4(vertexPosition_modelspace, 1);

    // uv of the vertex
    UV = vertexUV;

}

