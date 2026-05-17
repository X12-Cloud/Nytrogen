#include "macro_handler.hpp"
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <cctype>

MacroHandler::MacroHandler() {
    predefineBuiltins();
}

void MacroHandler::define(const std::string& name, const std::string& value) {
    m_macros[name] = value;
}

bool MacroHandler::isDefined(const std::string& name) const {
    return m_macros.find(name) != m_macros.end();
}

std::string MacroHandler::expandMacros(const std::string& line) {
    if (m_macros.empty()) return line;

    std::string result;
    std::string current_token;

    bool in_string = false;
    bool in_char_literal = false;

    auto flushToken = [&]() {
        if (!current_token.empty()) {
            if (!in_string && !in_char_literal && m_macros.count(current_token)) {
                result += m_macros[current_token];
            } else {
                result += current_token;
            }
            current_token.clear();
        }
    };

    for (size_t i = 0; i < line.size(); ++i) {
        char ch = line[i];

        // Comment safety check
        if (!in_string && !in_char_literal && ch == '/' && i + 1 < line.size() && line[i+1] == '/') {
            flushToken();
            result += line.substr(i);
            return result;
        }

        // String literal boundaries
        if (ch == '"' && (i == 0 || line[i-1] != '\\') && !in_char_literal) {
            flushToken();
            in_string = !in_string;
            result += ch;
            continue;
        }

        // Char literal boundaries
        if (ch == '\'' && (i == 0 || line[i-1] != '\\') && !in_string) {
            flushToken();
            in_char_literal = !in_char_literal;
            result += ch;
            continue;
        }

        // Build identifiers/tokens
        if (!in_string && !in_char_literal && (std::isalnum(ch) || ch == '_')) {
            current_token += ch;
        } else {
            flushToken();
            result += ch;
        }
    }

    flushToken();
    return result;
}

void MacroHandler::predefineBuiltins() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::tm* local_tm = std::localtime(&now_time);

    std::stringstream ss_datetime, ss_date, ss_time;
    ss_datetime << std::put_time(local_tm, "\"%Y-%m-%d %H:%M:%S\"");
    ss_date << std::put_time(local_tm, "\"%Y-%m-%d\"");
    ss_time << std::put_time(local_tm, "\"%H:%M:%S\"");

    m_macros["__DATE_TIME__"] = ss_datetime.str();
    m_macros["__DATE__"]      = ss_date.str();
    m_macros["__TIME__"]      = ss_time.str();
    m_macros["__VERSION__"]   = NYTRO_VERSION;

#if defined(__linux__)
    m_macros["__SYSTEM__"] = "\"Linux\"";
#elif defined(_WIN32)
    m_macros["__SYSTEM__"] = "\"Windows\"";
#elif defined(__APPLE__)
    m_macros["__SYSTEM__"] = "\"macOS\"";
#else
    m_macros["__SYSTEM__"] = "\"Unknown\"";
#endif
}
