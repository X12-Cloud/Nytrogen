#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <algorithm>
#include <cctype>

class Utils {
public:
    static std::string cleanString(std::string s) {
        s.erase(std::remove_if(s.begin(), s.end(), [](unsigned char c) {
            return std::iscntrl(c) || std::isspace(c);
        }), s.end());
        return s;
    }
};

#endif
