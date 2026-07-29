#ifndef MANGLER_HPP
#define MANGLER_HPP

#include <string>
#include <vector>

namespace Mangler {

inline auto mangleVariable(const std::vector<std::string>& scopes, const std::string& varName)
    -> std::string {
    std::string result = "_N";
    for (const auto& scope : scopes) {
        result += std::to_string(scope.length()) + scope;
    }
    result += std::to_string(varName.length()) + varName;
    return result;
}

inline auto mangleFunction(const std::vector<std::string>& scopes, const std::string& name)
    -> std::string {
    if (name == "main") {
        return "main";
    }

    std::string result = "_N";
    for (const auto& scope : scopes) {
        result += std::to_string(scope.length()) + scope;
    }
    result += std::to_string(name.length()) + name;
    return result;
}

}  // namespace Mangler

#endif
