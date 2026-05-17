#include "file_parser.hpp"
#include <fstream>
#include <sstream>

FileParser::FileParser() {}

// Helper function to strip leading and trailing whitespace
std::string FileParser::trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

void FileParser::parse(const std::string& filepath, std::ostream& output_stream) {
    fs::path abs_path = fs::absolute(filepath);

    // If a file was already processed skip it (#pragma once logic)
    if (m_file_handler.hasBeenProcessed(abs_path)) {
        return;
    }

    if (!m_file_handler.enterFile(abs_path)) {
        return;
    }

    std::ifstream file(abs_path);
    if (!file.is_open()) {
        std::cerr << "Preprocess Error: Cannot open file: " << abs_path << "\n";
        m_file_handler.leaveFile();
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::string trimmed = trim(line);

        // Preserve empty lines so error line numbers match the original source code
        if (trimmed.empty()) {
            output_stream << "\n";
            continue;
        }

        // DIRECTIVE ROUTE: #ifndef
        if (trimmed.rfind("#ifndef", 0) == 0) {
            std::istringstream iss(trimmed);
            std::string directive, macro_name;
            iss >> directive >> macro_name;

            // If the macro IS defined we want to skip the contents inside the block
            bool skip_this_block = m_macro_handler.isDefined(macro_name);
            m_conditional_stack.push_back(skip_this_block);
            continue;
        }

        // DIRECTIVE ROUTE: #endif
        if (trimmed.rfind("#endif", 0) == 0) {
            if (!m_conditional_stack.empty()) {
                m_conditional_stack.pop_back();
            } else {
                std::cerr << "Preprocess Error: Dangling #endif found in: " << abs_path << "\n";
            }
            continue;
        }

        // EVALUATE CONDITIONAL SKIPPING
        bool currently_skipping = false;
        for (bool skip : m_conditional_stack) {
            if (skip) {
                currently_skipping = true;
                break;
            }
        }
        if (currently_skipping) {
            continue; // Ignore this line completely
        }

        // DIRECTIVE ROUTE: #define
        if (trimmed.rfind("#define", 0) == 0) {
            std::istringstream iss(trimmed);
            std::string directive, macro_name, macro_value;
            iss >> directive >> macro_name;
            std::getline(iss, macro_value);

            m_macro_handler.define(macro_name, trim(macro_value));
            continue;
        }

        // DIRECTIVE ROUTE: #include
        if (trimmed.rfind("#include", 0) == 0) {
            size_t start_delim = line.find_first_of("\"<");
            size_t end_delim = line.find_last_of("\">");

            if (start_delim != std::string::npos && end_delim != std::string::npos) {
                std::string include_path = line.substr(start_delim + 1, end_delim - start_delim - 1);
                bool is_stdlib = (line[start_delim] == '<');

                // Resolve the correct path using the file handler
                fs::path child_file = m_file_handler.resolvePath(include_path, abs_path, is_stdlib);

                parse(child_file.string(), output_stream);
            } else {
                std::cerr << "Preprocess Error: Malformed include syntax: " << line << "\n";
            }
            continue;
        }

        output_stream << m_macro_handler.expandMacros(line) << "\n";
    }

    file.close();

    // Mark as processed
    m_file_handler.markAsProcessed(abs_path);
    m_file_handler.leaveFile();
}
