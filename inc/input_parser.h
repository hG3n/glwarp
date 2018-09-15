#ifndef INPUT_PARSER_H
#define INPUT_PARSER_H

/// taken from: https://stackoverflow.com/questions/865668/how-to-parse-command-line-arguments-in-c
/// @author iain

#include <vector>
#include <string>
#include <algorithm>

class InputParser {

    public:
        InputParser (int &argc, char **argv);

        const std::string& getCmdOption(const std::string &option) const;
        bool cmdOptionExists(const std::string &option) const;

    private:
        std::vector <std::string> tokens_;
};

#endif // INPUT_PARSER_H