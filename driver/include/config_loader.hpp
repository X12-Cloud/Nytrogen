#pragma once
#include <string>
#include <vector>
#include "sol/sol.hpp"

struct NytroConfig {
    std::string project_name = "default";
    std::string output_dir = "./out";
    std::string preprocessor_bin = "";
    std::string linker_bin = "ld";
    std::string assembler_bin = "nasm";
    bool use_tui = false;
    bool verbose = false;
    bool debug = false;
    bool clean = false;
    std::vector<std::string> sources;
    std::vector<std::string> extra_libs;
};

class ConfigLoader {
public:
    static NytroConfig load(const std::string& filename);
    static bool is_safe_path(const std::string& path);
};
