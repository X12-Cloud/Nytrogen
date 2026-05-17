#ifndef MACRO_HANDLER_HPP
#define MACRO_HANDLER_HPP

#include <string>
#include <map>

class MacroHandler {
public:
    MacroHandler();
    void define(const std::string& name, const std::string& value);
    bool isDefined(const std::string& name) const;
    std::string expandMacros(const std::string& line);

private:
    std::map<std::string, std::string> m_macros;
    void predefineBuiltins();
};

#endif
