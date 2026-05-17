#ifndef FILE_HANDLER_HPP
#define FILE_HANDLER_HPP

#include <string>
#include <vector>
#include <set>
#include <filesystem>

namespace fs = std::filesystem;

class FileHandler {
public:
    fs::path resolvePath(const std::string& target, const fs::path& current_file, bool is_stdlib);
    bool enterFile(const fs::path& absolute_path);
    void leaveFile();
    void markAsProcessed(const fs::path& absolute_path);
    bool hasBeenProcessed(const fs::path& absolute_path) const;

private:
    std::set<std::string> m_processed_files;
    std::vector<fs::path> m_include_stack;
};

#endif
