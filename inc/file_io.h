#ifndef FILE_IO_H
#define  FILE_IO_H

#include <glm/vec3.hpp>
#include <vector>

class FileIO {

public:
    static bool loadFile(const char *filepath, std::vector<glm::vec3> *to_fill);

};

#endif