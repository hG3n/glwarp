#include "../inc/input_parser.h"

InputParser::InputParser(int &argc, char **argv)
        : tokens_()
{
    for (int i = 1; i < argc; ++i)
        tokens_.push_back(std::string(argv[i]));
}

const std::string &InputParser::getCmdOption(const std::string &option) const
{
    std::vector<std::string>::const_iterator itr;
    itr = std::find(this->tokens_.begin(), this->tokens_.end(), option);
    if (itr != this->tokens_.end() && ++itr != this->tokens_.end()) {
        return *itr;
    }
    static const std::string empty_string("");
    return empty_string;
}

bool InputParser::cmdOptionExists(const std::string &option) const
{
    return std::find(this->tokens_.begin(), this->tokens_.end(), option)
           != this->tokens_.end();
}

