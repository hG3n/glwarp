#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertex_pos_modelspace;
layout(location = 1) in vec2 vertexUV;

// output data interpolated for each fragment
out vec2 UV;

// constant model view projection matrix
uniform mat4 MVP;

void main(){

    // output position of vertex
    gl_Position = MVP * vec4(vertex_pos_modelspace, 1);

    // uv of the vertex
    UV = vertexUV;

}

