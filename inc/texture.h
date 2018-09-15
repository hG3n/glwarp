#ifndef TEXTURE_H
#define TEXTURE_H


#include <GL/glew.h>

class Texture {

public:
    static GLuint loadBMP(const char *imagepath);

};

#endif