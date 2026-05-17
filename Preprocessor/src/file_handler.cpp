#include "file_handler.hpp"
#include <iostream>

fs::path FileHandler::resolvePath(const std::string& target_path, const fs::path& current_file, bool is_stdlib) {
    if (is_stdlib) {
        return fs::absolute(fs::path("/usr/include/nytrogen") / target_path);
    }
    return fs::absolute(current_file.parent_path() / target_path);
}

bool FileHandler::enterFile(const fs::path& absolute_path) {
    for (const auto& active_path : m_include_stack) {
        if (active_path == absolute_path) {
            std::cerr << "\nPreprocess Error: Circular dependency loop detected!\n";
            for (const auto& trace : m_include_stack) {
                std::cerr << "  -> Included from: " << trace << "\n";
            }
            std::cerr << "  -> Culprit file: " << absolute_path << "\n";
            return false; // Stop the compiler before it crashes the stack
        }
    }
    m_include_stack.push_back(absolute_path);
    return true;
}

void FileHandler::leaveFile() {
    if (!m_include_stack.empty()) {
        m_include_stack.pop_back();
    }
}

void FileHandler::markAsProcessed(const fs::path& absolute_path) {
    m_processed_files.insert(absolute_path.string());
}

bool FileHandler::hasBeenProcessed(const fs::path& absolute_path) const {
    return m_processed_files.count(absolute_path.string()) > 0;
}
