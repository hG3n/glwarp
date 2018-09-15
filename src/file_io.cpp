#include "../inc/file_io.h"

#include <fstream>
#include <sstream>
#include <string>
#include <iostream>

bool FileIO::loadFile(const char *filepath, std::vector<glm::vec3> *to_fill)
{
    std::ifstream f;
    std::string s;

    f.open(filepath, std::ios::in);

    if (f.is_open()) {
        std::cout << "Loading file: '" << filepath << "'!" << std::endl;
        while (!f.eof()) {
            getline(f, s);
            std::istringstream iss(s);

            float x, y, z;
            iss >> x >> y >> z;

            // append to input vector
            to_fill->push_back(glm::vec3(x, y, z));
        }
        return true;
    }

    std::cout << "Error loading file: '" << filepath << "'!" << std::endl;
    return false;
}