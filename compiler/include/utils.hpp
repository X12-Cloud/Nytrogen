#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <algorithm>
#include <cctype>
#include <fstream>
#include <vector>

namespace Utils {
    static std::string cleanString(std::string s) {
        s.erase(std::remove_if(s.begin(), s.end(), [](unsigned char c) {
            return std::iscntrl(c) || std::isspace(c);
        }), s.end());
        return s;
    }
    static std::string get_distro_name() {
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
};

namespace Mangler {
    inline std::string mangleVariable(const std::vector<std::string>& scopes, const std::string& varName) {
        std::string result = "_N";
        for (const auto& scope : scopes) {
            result += std::to_string(scope.length()) + scope;
        }
        result += std::to_string(varName.length()) + varName;
        return result;
    }

    inline std::string mangleFunction(const std::vector<std::string>& scopes, const std::string& name) {
        if (name == "main") return "main";

        std::string result = "_Z";
        for (const auto& scope : scopes) {
            result += std::to_string(scope.length()) + scope;
        }
        result += std::to_string(name.length()) + name;
        return result;
    }
}

#endif
