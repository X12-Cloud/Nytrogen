#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <iostream>
#include <string>

#define ANSI_RESET "\033[0m"
#define ANSI_RED "\033[1;31m"
#define ANSI_YELLOW "\033[1;33m"
#define ANSI_CYAN "\033[1;36m"
#define ANSI_WHITE "\033[1;37m"

namespace Logger {

[[noreturn]] inline void report_error(const std::string& err_type, const std::string& msg,
                                      int line) {
    std::cerr << ANSI_RED << err_type << ": " << ANSI_WHITE << msg << ANSI_YELLOW << " (Line "
              << line << ")" << ANSI_RESET << "\n";
    std::exit(1);
}

[[noreturn]] inline void report_error(const std::string& err_type, const std::string& msg) {
    std::cerr << ANSI_RED << err_type << ": " << ANSI_WHITE << msg << ANSI_RESET << "\n";
    std::exit(1);
}

}  // namespace Logger

#endif
