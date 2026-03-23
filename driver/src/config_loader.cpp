#include "config_loader.hpp"
#include <iostream>

NytroConfig ConfigLoader::load(const std::string& filename) {
    sol::state lua;
    lua.open_libraries(sol::lib::base);
    
    NytroConfig conf;

    try {
        auto result = lua.script_file(filename);
        if (result.valid()) {
            sol::table project = lua["project"];
            if (project.valid()) {
                conf.project_name = project["name"].get_or(std::string("untitled"));

                sol::table settings = project["settings"];
                if (settings.valid()) {
                    conf.verbose = settings["verbose"].get_or(false);
                    conf.debug = settings["debug"].get_or(false);
                    conf.clean = settings["clean"].get_or(false);
                    conf.use_tui = settings["tui"].get_or(false);
                    conf.output_dir = settings["output_dir"].get_or(std::string("./out"));
                    conf.preprocessor_bin = settings["preprocessor"].get_or(std::string("nytro-pre"));
                    conf.linker_bin = settings["linker"].get_or(std::string("ld"));
                    conf.assembler_bin = settings["assembler"].get_or(std::string("nasm"));
		    sol::optional<sol::table> sources_table = project["sources"]; // sources
    		    if (sources_table) conf.sources = sources_table->as<std::vector<std::string>>();
		    sol::optional<sol::table> extra_libs = project["extra_libs"]; // extra libraries
    		    if (extra_libs) conf.extra_libs = extra_libs->as<std::vector<std::string>>();
                }
            }
        }
    } catch (const sol::error& e) {
        std::cerr << "Lua Config Error: " << e.what() << std::endl;
    }

    return conf;
}

bool ConfigLoader::is_safe_path(const std::string& path) {
    if (path.empty()) return true;

    // more agressive so commented for now but maybe uncomment later if it gets destructive
    // if (!is_alphanum(path[0])) return false;

    if (path[0] == '/' || path.find("..") != std::string::npos) return false; // block slashes in the beginning and going back using ".."
    if (path.size() > 1 && path[1] == ':') return false; // block this too cuz if i ever get on windows

    return true;
}
