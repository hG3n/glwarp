#ifndef SHADER_H
#define SHADER_H

#include <GL/glew.h>

class Shader {
public:
    static GLuint loadShaders(const char *, const char *);
};

#endif