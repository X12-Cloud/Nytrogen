#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <filesystem>
#include <algorithm>
#include <sys/wait.h>
#include <unordered_map>

namespace fs = std::filesystem;

struct Config {
    bool obj_only = false;
    bool asm_only = false;
    bool no_preproc = false;
    bool verbose = false;
    bool debug = false;
    bool show_version = false;
    bool clean = false;
} cfg;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: nytro <input_file> [-o output_name] [-obj]\n";
        return 1;
    }

    std::string input_file = argv[1];
    std::string extra_flags = "";
    std::string base_name = fs::path(input_file).stem().string();
    std::string output_bin_name = base_name; // Default to input name

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
    };

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (flag_map.count(arg)) {
            *flag_map[arg] = true;
	    if (arg == "-debug" || arg == "-verbose") extra_flags += " " + arg;
	    else if (cfg.show_version) {
		std::cout << "Nytrogen Toolchain v" << NYTRO_VERSION << std::endl;
		return 0;
	    }
        } else if (arg == "-o" && i + 1 < argc) {
            // Handle flags that need values separately
            output_bin_name = argv[++i];
        } else if (arg[0] != '-') { 
            input_file = arg;
            base_name = fs::path(input_file).stem().string();
        }
    }

    if (input_file.empty()) {
        std::cerr << "Error: No input file specified." << std::endl;
        return 1;
    }

    base_name = fs::path(input_file).stem().string();
    if (output_bin_name.empty()) output_bin_name = base_name;

    // Path Resolution
    fs::path exe_path = fs::canonical("/proc/self/exe");
    fs::path bin_dir = exe_path.parent_path(); 

    fs::path compiler_bin;
    fs::path pre_bin;

    // Check if siblings exist
    if (fs::exists(bin_dir / "Nytro") && fs::exists(bin_dir / "nytro-pre")) {
        compiler_bin = bin_dir / "Nytro";
        pre_bin = bin_dir / "nytro-pre";
    } else {
        // Fallback
        compiler_bin = "/usr/lib/nytro/nytro-cc1"; 
        pre_bin = "/usr/lib/nytro/nytro-pre";
    }

    // Setup Output Directory (Current Working Directory/out)
    fs::path out_dir = fs::current_path() / "out";
    if (cfg.clean) {
	if (cfg.verbose) std::cout << "--- Cleaning out/ directory ---" << std::endl;
	fs::remove_all(out_dir);
    }
    fs::create_directories(out_dir);

    std::string pre_out   = (out_dir / (base_name + ".pre.nyt")).string();
    std::string asm_file  = (out_dir / (base_name + ".asm")).string();
    std::string obj_file  = (out_dir / (base_name + ".o")).string();
    std::string final_exe = (out_dir / output_bin_name).string();

    // Preprocessor
    if (!cfg.no_preproc) {
        if (cfg.verbose) std::cout << "--- Running Nytrogen Preprocessor ---" << std::endl;
        std::string pre_cmd = "\"" + pre_bin.string() + "\" \"" + input_file + "\" > \"" + pre_out + "\"";
        if (std::system(pre_cmd.c_str()) != 0) return 1;
    }

    // Compiler
    if (cfg.verbose) std::cout << "--- Running Nytrogen Compiler ---" << std::endl;
    std::string comp_cmd = "\"" + compiler_bin.string() + "\" \"" + pre_out + "\" \"" + asm_file + "\" " + extra_flags;
    if (std::system(comp_cmd.c_str()) != 0) return 1;
    if (cfg.asm_only) return 0;

    // NASM (Assembler)
    if (cfg.verbose) std::cout << "\n--- Assembling " << base_name << ".asm ---" << std::endl;
    std::string nasm_cmd = "nasm -f elf64 \"" + asm_file + "\" -o \"" + obj_file + "\"";
    if (std::system(nasm_cmd.c_str()) != 0) return 1;

    if (cfg.obj_only) {
        std::cout << "Object file generated: " << obj_file << std::endl;
        return 0;
    }

    // LD (Linker)
    if (cfg.verbose) std::cout << "\n--- Linking ---" << std::endl;
    std::string link_cmd = "ld -o \"" + final_exe + "\" \"" + obj_file + "\" -lc --dynamic-linker /usr/lib64/ld-linux-x86-64.so.2";
    if (std::system(link_cmd.c_str()) != 0) return 1;

    // Execution
    if (cfg.verbose) std::cout << "\n--- Running output program ---" << std::endl;
    std::string run_cmd = "\"" + final_exe + "\"";
    int status = std::system(run_cmd.c_str());
    
    if (WIFEXITED(status)) {
        std::cout << "\nExit Code: " << WEXITSTATUS(status) << std::endl;
    }

    return 0;
}
