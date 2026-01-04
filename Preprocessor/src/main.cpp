#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <filesystem>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <regex>
#include <map>
namespace fs = std::filesystem;

// Function to read the entire content of a file into a string
std::string readFileContent(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file for reading: " << filepath << std::endl;
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void processFile(const std::string& input_filepath, std::ostream& output_stream, std::map<std::string, std::string>& defined_macros) {
    std::ifstream input_file(input_filepath);
    if (!input_file.is_open()) {
        std::cerr << "Error: Could not open input file: " << input_filepath << std::endl;
        return;
    }

    bool skipping = false;
    std::string line;
    while (std::getline(input_file, line)) {
        std::string trimmed_line = line;
        size_t first = trimmed_line.find_first_not_of(" \t");
        if (first != std::string::npos) {
            trimmed_line = trimmed_line.substr(first);
        }

        if (trimmed_line.rfind("#ifndef", 0) == 0) {
            std::istringstream iss(trimmed_line);
            std::string directive, macro_name;
            iss >> directive >> macro_name;
            if (defined_macros.count(macro_name)) {
                skipping = true;
            }
            continue;
        } else if (trimmed_line.rfind("#endif", 0) == 0) {
            skipping = false;
            continue;
        }

        if (skipping) {
            continue;
        }

        if (trimmed_line.rfind("#define", 0) == 0) {
            std::istringstream iss(trimmed_line);
            std::string directive, macro_name, macro_value;
            iss >> directive >> macro_name;
            std::getline(iss, macro_value);
            size_t val_first = macro_value.find_first_not_of(" \t");
            if (std::string::npos != val_first) {
                macro_value = macro_value.substr(val_first);
            }
            defined_macros[macro_name] = macro_value;
            continue;
        }

        if (line.rfind("include ", 0) == 0) {
            std::string include_path;
            fs::path current_file_path(input_filepath);
            fs::path included_file_path;

            size_t start_delim = line.find_first_of("\"<");
            size_t end_delim = line.find_last_of("\">");

            if (start_delim != std::string::npos && end_delim != std::string::npos) {
                include_path = line.substr(start_delim + 1, end_delim - start_delim - 1);
                if (line[start_delim] == '<') {
                    included_file_path = fs::path("std/") / include_path;
                } else {
                    included_file_path = current_file_path.parent_path() / include_path;
                }
                processFile(included_file_path.string(), output_stream, defined_macros);
            } else {
                std::cerr << "Preprocessor Error: Invalid include directive: " << line << std::endl;
                output_stream << line << std::endl;
            }
        } else {
            // Macro replacement
            for (const auto& pair : defined_macros) {
                 if (!pair.first.empty()) {
                    std::regex re("\\b" + pair.first + "\\b");
                    line = std::regex_replace(line, re, pair.second);
                }
            }
            output_stream << line << std::endl;
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <input_file>" << std::endl;
        return 1;
    }

    std::string input_filepath = argv[1];
    std::map<std::string, std::string> defined_macros;

    // Pre-define __BUILT_IN__ macros
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::tm* local_tm = std::localtime(&now_time);
    std::stringstream ss_datetime, ss_date, ss_time;
    ss_datetime << std::put_time(local_tm, "\"%Y-%m-%d %H:%M:%S\"");
    ss_date << std::put_time(local_tm, "\"%Y-%m-%d\"");
    ss_time << std::put_time(local_tm, "\"%H:%M:%S\"");
    defined_macros["__DATE_TIME__"] = ss_datetime.str();
    defined_macros["__DATE__"] = ss_date.str();
    defined_macros["__TIME__"] = ss_time.str();
    defined_macros["__VERSION__"] = "\"0.1 beta\"";
#if defined(__linux__)
    defined_macros["__SYSTEM__"] = "\"Linux\"";
#elif defined(_WIN32)
    defined_macros["__SYSTEM__"] = "\"Windows\"";
#elif defined(__APPLE__)
    defined_macros["__SYSTEM__"] = "\"macOS\"";
#else
    defined_macros["__SYSTEM__"] = "\"Unknown\"";
#endif

    processFile(input_filepath, std::cout, defined_macros);

    return 0;
}

