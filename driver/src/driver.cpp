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

struct FlagInfo { // TODO: add this
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

    std::string input_file = "";
    std::string extra_flags = "";
    std::string base_name = fs::path(input_file).stem().string();
    std::string output_bin_name = base_name; // Default to input name

    if (cfg.verbose) extra_flags += " -verbose";
    if (cfg.debug)   extra_flags += " -debug";

    // TODO: add a -help flag that would open a cool help screen, make it possible to use multiple single char flags at once

    // Flag map
    std::unordered_map<std::string, bool*> flag_map = {
        {"-obj",        &cfg.obj_only},
        {"-c",          &cfg.obj_only},
        {"-asm",        &cfg.asm_only},
        {"-S",          &cfg.asm_only},
        {"-dp",         &cfg.no_preproc},
        {"-verbose",    &cfg.verbose},
        {"-debug",      &cfg.debug},
	{"--version",   &cfg.show_version},
	{"-v",          &cfg.show_version},
	{"-clean",      &cfg.clean},
	{"--clear",     &cfg.clean},
	{"-help",       &cfg.help},
    };

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (flag_map.count(arg)) {
            *flag_map[arg] = true;
	    if (arg == "-debug" || arg == "-verbose") extra_flags += " " + arg;
	    else if (cfg.show_version) {
		std::cout << "Nytrogen Toolchain v" << NYTRO_VERSION << std::endl;
		return 0;
	    } else if (cfg.help) std::cout << "Im too lazy to make the help screen just read driver.cpp and u will know everything" << std::endl;
        } else if (arg == "-o" && i + 1 < argc) {
            // Handle flags that need values separately
            output_bin_name = argv[++i];
        } else if (arg[0] != '-') { 
            input_file = arg;
            base_name = fs::path(input_file).stem().string();
        }
    }

    // files to get compiled
    std::vector<std::string> files_to_compile;
    if (!lua_config.sources.empty()) {
	files_to_compile = lua_config.sources;
    } else if (argc >= 2 && argv[1][0] != '-') {
	files_to_compile = { argv[1] };
    }
    if (files_to_compile.empty()) {
        std::cerr << "Error: No input files found in init.lua or CLI." << std::endl;
        return 1;
    }

    // extra libraries, will get added to the linker cmd
    std::vector<std::string> extra_libs;
    if (!lua_config.extra_libs.empty()) {
	extra_libs = lua_config.extra_libs;
    }

    base_name = fs::path(input_file).stem().string();
    if (output_bin_name.empty()) output_bin_name = lua_config.project_name;

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
    for (const auto& obj_path : object_files) {
	all_objs += "\"" + obj_path + "\" ";
    }
    std::string link_cmd = lua_config.linker_bin + " -o \"" + final_exe + "\" " + all_objs + " -lc --dynamic-linker /lib64/ld-linux-x86-64.so.2";
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
