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

// Function to process a single file, handling includes and macros
void processFile(const std::string& input_filepath, std::ostream& output_stream) {
    std::ifstream input_file(input_filepath);
    if (!input_file.is_open()) {
        std::cerr << "Error: Could not open input file: " << input_filepath << std::endl;
        return;
    }

    // Prepare macro values
    std::map<std::string, std::string> macroValues;
    
    // Time-based macros
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::tm* local_tm = std::localtime(&now_time);
    
    std::stringstream ss_datetime, ss_date, ss_time;
    ss_datetime << std::put_time(local_tm, "\"%Y-%m-%d %H:%M:%S\"");
    ss_date << std::put_time(local_tm, "\"%Y-%m-%d\"");
    ss_time << std::put_time(local_tm, "\"%H:%M:%S\"");
    
    macroValues["__DATE_TIME__"] = ss_datetime.str();
    macroValues["__DATE__"] = ss_date.str();
    macroValues["__TIME__"] = ss_time.str();

    // Static macros
    macroValues["__VERSION__"] = "\"0.1 beta\"";
#if defined(__linux__)
    macroValues["__SYSTEM__"] = "\"Linux\"";
#elif defined(_WIN32)
    macroValues["__SYSTEM__"] = "\"Windows\"";
#elif defined(__APPLE__)
    macroValues["__SYSTEM__"] = "\"macOS\"";
#else
    macroValues["__SYSTEM__"] = "\"Unknown\"";
#endif

    std::string line;
    std::regex macro_regex("__([A-Z_]+)__");

    while (std::getline(input_file, line)) {
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
                
                std::string included_content = readFileContent(included_file_path.string());
                if (!included_content.empty()) {
                    output_stream << included_content << std::endl;
                } else {
                    std::cerr << "Preprocessor Warning: Could not find included file: " << included_file_path.string() << std::endl;
                }
            } else {
                std::cerr << "Preprocessor Error: Invalid include directive: " << line << std::endl;
                output_stream << line << std::endl;
            }
        } else {
            auto matches = std::sregex_iterator(line.begin(), line.end(), macro_regex);
            std::vector<std::smatch> all_matches;
            for (auto i = matches; i != std::sregex_iterator(); ++i) {
                all_matches.push_back(*i);
            }

            for (auto i = all_matches.rbegin(); i != all_matches.rend(); ++i) {
                const std::smatch& match = *i;
                std::string full_macro = match.str(0);
                if (macroValues.count(full_macro)) {
                    line.replace(match.position(), match.length(), macroValues[full_macro]);
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

    processFile(input_filepath, std::cout);

    return 0;
}
