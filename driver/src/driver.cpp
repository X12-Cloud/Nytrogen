#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <filesystem>
#include <algorithm>
#include <sys/wait.h>
#include <unordered_map>
#include "config_loader.hpp"

namespace fs = std::filesystem;

struct Config {
    bool obj_only = false;
    bool asm_only = false;
    bool no_preproc = false;
    bool verbose = false;
    bool debug = false;
    bool show_version = false;
    bool clean = false;
    bool help = false;
} cfg;

struct FlagInfo {
    bool* value;
    std::string description;
};

namespace Color {
    const char* RED   = "\033[31m";
    const char* GREEN = "\033[32m";
    const char* YELLOW= "\033[33m";
    const char* RESET = "\033[0m";
};

int main(int argc, char* argv[]) {
    auto lua_config = ConfigLoader::load("init.lua"); // load the lua config file
    cfg.verbose = lua_config.verbose;
    cfg.debug = lua_config.debug;
    cfg.clean = lua_config.clean;
    std::string output_bin_name = lua_config.project_name;
    std::vector<std::string> sources_to_compile = lua_config.sources;

    std::vector<std::string> cli_sources;
    std::string extra_flags = "";

    // Flag map
    std::unordered_map<std::string, FlagInfo> flag_map = {
        {"--obj",     {&cfg.obj_only, "Compile to object file only."}},
        {"--asm",     {&cfg.asm_only, "Stop after assembly generation."}},
        {"--disable-preprocessor", {&cfg.no_preproc, "Skip the pre-processing stage."}},
        {"--verbose", {&cfg.verbose,  "Enable detailed logging."}},
        {"--debug",   {&cfg.debug,    "Include debug symbols."}},
        {"--version", {&cfg.show_version, "Show Nytrogen version."}},
        {"--clear",   {&cfg.clean,    "Clean the output directory."}},
        {"--help",    {&cfg.help,    "Show this menu."}}
    };

    std::unordered_map<std::string, std::string> flag_aliases = {
        {"-c", "--obj"},
        {"-S", "--asm"},
        {"-v", "--version"},
        {"-dp", "--disable-preprocessor"},
        {"-h", "--help"}
    };

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (flag_aliases.count(arg)) arg = flag_aliases[arg];

        if (flag_map.count(arg)) {
            *flag_map[arg].value = true;

            if (cfg.show_version) {
                std::cout << "Nytrogen Toolchain v" << NYTRO_VERSION << std::endl;
                return 0;
            }
            if (cfg.help) {
                std::cout << Color::YELLOW << "Nytrogen Toolchain v" << NYTRO_VERSION << Color::RESET << "\n";
                std::cout << "Usage: nytrogen [options] [files...]\n\n";
                std::cout << "Options:\n";

                for (auto const& [flag, info] : flag_map) {
                    // Find if this flag has an alias
                    std::string alias = "    "; // default padding if no alias
                    for (auto const& [shorthand, primary] : flag_aliases) {
                        if (primary == flag) {
                            alias = shorthand + ",";
                            break;
                        }
                    }

                    // Print columns: Shorthand (padded 4) | Flag (padded 20) | Description
                    std::printf("  %s%-4s %-22s%s %s\n", 
                    Color::GREEN, alias.c_str(), flag.c_str(), Color::RESET, 
                    info.description.c_str());
                }
                std::cout << "\nExample:\n  nytrogen -c main.ny\n";
                return 0;
            }
        } else if (arg == "-o" && i + 1 < argc) {
            output_bin_name = argv[++i];
        } else if (arg[0] != '-') {
            cli_sources.push_back(arg);
        }
    }

    std::vector<std::string> files_to_compile;
    if (!cli_sources.empty()) {
        files_to_compile = cli_sources; // cli priority
    } else {
        files_to_compile = lua_config.sources; // fallback to the sources in the lua config
    }

    if (cfg.verbose) extra_flags += " -verbose";
    if (cfg.debug)   extra_flags += " -debug";
    if (files_to_compile.empty()) {
        std::cerr << "Error: No input files found in init.lua or CLI." << std::endl;
        return 1;
    }

    // extra libraries, will get added to the linker cmd
    std::vector<std::string> extra_libs;
    if (!lua_config.extra_libs.empty()) extra_libs = lua_config.extra_libs;

    // Path Resolution
    fs::path exe_path = fs::canonical("/proc/self/exe");
    fs::path bin_dir = exe_path.parent_path();

    fs::path compiler_bin;
    fs::path pre_bin;

    if (fs::exists(bin_dir / "nytro-c") && fs::exists(bin_dir / "nytro-pre")) {
        compiler_bin = bin_dir / "nytro-c";
        pre_bin = bin_dir / "nytro-pre";
    } else if (fs::exists(fs::current_path() / "build/bin/nytro-c")) {
        compiler_bin = fs::current_path() / "build/bin/nytro-c";
        pre_bin = fs::current_path() / "build/bin/nytro-pre";
    } else {
        compiler_bin = "/usr/local/bin/nytro-c";
        pre_bin = "/usr/local/bin/nytro-pre";
    }

    if (!lua_config.preprocessor_bin.empty() && lua_config.preprocessor_bin != "nytro-pre") {
        pre_bin = lua_config.preprocessor_bin;
    } // use the preprocessor defined in the lua config

    // Setup Output Directory (Current Working Directory/out)
    fs::path out_dir = fs::current_path() / "out";
    if (!lua_config.output_dir.empty()) {
        if (ConfigLoader::is_safe_path(lua_config.output_dir)) {
            out_dir = lua_config.output_dir;
        } else {
            std::cerr << Color::RED << "FATAL: Unsafe output_dir detected in Lua config: " << lua_config.output_dir << Color::RESET << "\n";
            std::cerr << "Paths must be relative and cannot contain '..'\n";
            return 1;
        }
    }
    if (cfg.clean) {
	if (cfg.verbose) std::cout << "--- Cleaning out/ directory ---" << std::endl;
	fs::remove_all(out_dir);
    }
    fs::create_directories(out_dir);

    std::vector<std::string> object_files;

    std::string final_exe = (out_dir / output_bin_name).string();

    for (const auto& current_input : files_to_compile) {
        bool is_entry = (current_input == files_to_compile[0]);

        std::string current_base = fs::path(current_input).stem().string();
        std::string current_pre   = (out_dir / (current_base + ".pre.nyt")).string();
        std::string current_asm   = (out_dir / (current_base + ".asm")).string();
        std::string current_obj   = (out_dir / (current_base + ".o")).string();

        // Preprocessor
        if (!cfg.no_preproc) {
            if (cfg.verbose) std::cout << "--- Running Nytrogen Preprocessor ---" << std::endl;
            std::string pre_cmd = "\"" + pre_bin.string() + "\" \"" + current_input + "\" > \"" + current_pre + "\"";
        if (cfg.verbose) std::cout << "Running: " << pre_cmd << std::endl;
            if (std::system(pre_cmd.c_str()) != 0) return 1;
        }

        // Compiler
        if (cfg.verbose) std::cout << "--- Running Nytrogen Compiler ---" << std::endl;
        std::string entry_flag = is_entry ? " -entry" : "";
        std::string comp_cmd = "\"" + compiler_bin.string() + "\" \"" + current_pre + "\" \"" + current_asm + "\" " + extra_flags + entry_flag;
        if (cfg.verbose) std::cout << "Running: " << comp_cmd << std::endl;
        if (std::system(comp_cmd.c_str()) != 0) return 1;
        if (cfg.asm_only) return 0;

        // NASM (Assembler)
        if (cfg.verbose) std::cout << "\n--- Assembling " << current_base << ".asm ---" << std::endl;
        std::string nasm_cmd = "nasm -f elf64 \"" + current_asm + "\" -o \"" + current_obj + "\"";
        if (cfg.verbose) std::cout << "Running: " << nasm_cmd << std::endl;
        if (std::system(nasm_cmd.c_str()) != 0) return 1;

        object_files.push_back(current_obj);
    }

    if (cfg.obj_only) {
        std::cout << "Object files generated" << std::endl;
        return 0;
    }

    // LD (Linker)
    if (cfg.verbose) std::cout << "\n--- Linking ---" << std::endl;
    std::string all_objs = "";
    std::string lib_flags = "";
    for (const auto& lib : lua_config.extra_libs) {
	lib_flags += "-l" + lib + " ";
    }
    for (const auto& obj_path : object_files) {
	all_objs += "\"" + obj_path + "\" ";
    }
    std::string link_cmd = lua_config.linker_bin + " -o \"" + final_exe + "\" " + all_objs + lib_flags + " -lc --dynamic-linker /lib64/ld-linux-x86-64.so.2";
    if (cfg.verbose) std::cout << "Running: " << link_cmd << std::endl;
    if (std::system(link_cmd.c_str()) != 0) {
        std::cerr << Color::RED << "Linker Error: Failed to create executable." << Color::RESET << std::endl;
        return 1;
    }

    // Execution
    if (cfg.verbose) std::cout << "\n--- Running output program ---" << std::endl;
    std::string run_cmd = "./" + fs::relative(final_exe, fs::current_path()).string();
    int status = std::system(run_cmd.c_str());
    if (WIFEXITED(status)) {
        std::cout << "\nExit Code: " << WEXITSTATUS(status) << std::endl;
    }

    return 0;
}
