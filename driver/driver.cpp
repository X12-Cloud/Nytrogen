#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <filesystem>
#include <algorithm>
#include <sys/wait.h> 

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: nytro <input_file> [-o output_name] [-obj]\n";
        return 1;
    }

    // 1. Path Resolution
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

    // 2. Argument Handling
    std::string input_file = argv[1];
    std::string base_name = fs::path(input_file).stem().string();
    std::string output_bin_name = base_name; // Default to input name
    bool obj_only = false;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-o" && i + 1 < argc) {
            output_bin_name = argv[++i];
        } else if (arg == "-obj") {
            obj_only = true;
        }
    }

    // 3. Setup Output Directory (Current Working Directory/out)
    fs::path out_dir = fs::current_path() / "out";
    fs::create_directories(out_dir);

    std::string pre_out   = (out_dir / (base_name + ".pre.nyt")).string();
    std::string asm_file  = (out_dir / (base_name + ".asm")).string();
    std::string obj_file  = (out_dir / (base_name + ".o")).string();
    // We name it specifically inside /out to avoid clashing with the folder 'out'
    std::string final_exe = (out_dir / output_bin_name).string();

    // 4. Preprocessor
    std::cout << "--- Running Nytrogen Preprocessor ---" << std::endl;
    std::string pre_cmd = "\"" + pre_bin.string() + "\" \"" + input_file + "\" > \"" + pre_out + "\"";
    if (std::system(pre_cmd.c_str()) != 0) return 1;

    // 5. Compiler
    std::cout << "--- Running Nytrogen Compiler ---" << std::endl;
    std::string comp_cmd = "\"" + compiler_bin.string() + "\" \"" + pre_out + "\" \"" + asm_file + "\"";
    if (std::system(comp_cmd.c_str()) != 0) return 1;

    // 6. NASM (Assembler)
    std::cout << "\n--- Assembling " << base_name << ".asm ---" << std::endl;
    std::string nasm_cmd = "nasm -f elf64 \"" + asm_file + "\" -o \"" + obj_file + "\"";
    if (std::system(nasm_cmd.c_str()) != 0) return 1;

    if (obj_only) {
        std::cout << "Object file generated: " << obj_file << std::endl;
        return 0;
    }

    // 7. LD (Linker)
    std::cout << "\n--- Linking ---" << std::endl;
    std::string link_cmd = "ld -o \"" + final_exe + "\" \"" + obj_file + "\" -lc --dynamic-linker /usr/lib64/ld-linux-x86-64.so.2";
    if (std::system(link_cmd.c_str()) != 0) return 1;

    // 8. Execution
    std::cout << "\n--- Running output program ---" << std::endl;
    std::string run_cmd = "\"" + final_exe + "\"";
    int status = std::system(run_cmd.c_str());
    
    if (WIFEXITED(status)) {
        std::cout << "\nExit Code: " << WEXITSTATUS(status) << std::endl;
    }

    return 0;
}
