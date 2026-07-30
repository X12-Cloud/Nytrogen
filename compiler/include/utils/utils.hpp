#ifndef UTILS_HPP
#define UTILS_HPP

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "logger.hpp"
#include "mangler.hpp"
#include "lexer.hpp"

namespace Utils {

static auto cleanString(std::string s) -> std::string {
    s.erase(
        std::remove_if(s.begin(), s.end(),
                       [](unsigned char c) -> bool { return std::iscntrl(c) || std::isspace(c); }),
        s.end());
    return s;
}

static auto get_distro_name() -> std::string {
    std::ifstream file("/etc/os-release");
    std::string line;
    if (file.is_open()) {
        while (getline(file, line)) {
            // PRETTY_NAME usually contains the full name
            if (line.find("PRETTY_NAME=") == 0) {
                size_t first = line.find('\"');
                size_t last = line.find_last_of('\"');
                if (first != std::string::npos && last != std::string::npos && first != last) {
                    return line.substr(first + 1, last - first - 1);
                }
            }
        }
    }
    return "Unknown Distribution";
}

static bool isTypeKeyword(Token::Type type) {
    switch (type) {
        case Token::KEYWORD_INT:
        case Token::KEYWORD_CHAR:
        case Token::KEYWORD_FLOAT:
        case Token::KEYWORD_DOUBLE:
        case Token::KEYWORD_BOOL:
        case Token::KEYWORD_STRING:
            return true;
        default:
            return false;
    }
}

};  // namespace Utils

#endif
