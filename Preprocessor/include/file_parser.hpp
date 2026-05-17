#ifndef FILE_PARSER_HPP
#define FILE_PARSER_HPP

#include <string>
#include <iostream>
#include <vector>
#include "file_handler.hpp"
#include "macro_handler.hpp"

class FileParser {
public:
    FileParser();
    void parse(const std::string& filepath, std::ostream& output_stream);

private:
    FileHandler m_file_handler;
    MacroHandler m_macro_handler;
    std::vector<bool> m_conditional_stack;

    std::string trim(const std::string& str);
    void handleDirective(const std::string& line, const fs::path& current_path, std::ostream& output);
};

#endif
