#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <fstream>
#include <vector>
#include <thread>
#include <chrono>

using namespace ftxui;

// Helper to reload file contents into a vector
void reload_file(const std::string& filename, std::vector<std::string>& lines) {
    std::ifstream file(filename);
    lines.clear();
    if (file.is_open()) {
        std::string l;
        while (std::getline(file, l)) lines.push_back(l);
        file.close();
    } else {
        lines.push_back("Waiting for " + filename + "...");
    }
}

int main(int argc, char* argv[]) {
    std::string ast_file = (argc > 1) ? argv[1] : "ast.txt";
    std::string out_file = "out/output.txt";

    std::vector<std::string> ast_lines;
    std::vector<std::string> out_lines;

    // Initial load
    reload_file(ast_file, ast_lines);
    reload_file(out_file, out_lines);

    auto screen = ScreenInteractive::Fullscreen();

    int selected = 0;
    auto menu = Menu(&ast_lines, &selected);

    auto main_container = Renderer(menu, [&] {
        // Build AST view
        Elements ast_elements;
        for (int i = 0; i < ast_lines.size(); ++i) {
            auto color_style = (i == selected) ? focus : nothing;
            auto text_color = color(Color::White);
            if (ast_lines[i].find("FUNCTION_DEF") != std::string::npos) text_color = color(Color::Yellow);
            else if (ast_lines[i].find("VAR_REF") != std::string::npos) text_color = color(Color::Cyan);
            ast_elements.push_back(text(ast_lines[i]) | text_color | color_style);
        }

        // Build Output view
        Elements out_elements;
        for (const auto& line : out_lines) {
            out_elements.push_back(text("  " + line) | color(Color::GreenLight));
        }

        // MAIN LAYOUT
        return border(hbox({
            // LEFT SIDE: AST & OUTPUT
            vbox({
                // AST Panel (Top)
                vbox(std::move(ast_elements)) | vscroll_indicator | yframe | yflex | flex,
                separator(),
                // Output Panel (Bottom)
                vbox({
                    text(" PROGRAM OUTPUT ") | bold | color(Color::Green),
                    separator(),
                    vbox(std::move(out_elements)) | yframe | size(HEIGHT, EQUAL, 8) 
                })
            }) | flex,

            separator(),

            // RIGHT SIDE: SIDEBAR
            vbox({
                text(" NYTROGEN ") | bold | center | color(Color::Red),
                text(" DEBUGGER ") | bold | center | color(Color::Red),
                separator(),
                text(" AST: " + ast_file) | dim,
                text(" OUT: " + out_file) | dim,
                filler(),
                text(" Refresh: 1000ms") | dim,
                text(" Scroll: ↑/↓") | dim,
                text(" [Q] Quit ") | inverted | center,
            }) | size(WIDTH, EQUAL, 20)
        }));
    });

    auto component = CatchEvent(main_container, [&](Event event) {
        if (event == Event::Character('q') || event == Event::Character('Q')) {
            screen.Exit();
            return true;
        }
        return false;
    });

    // REFRESH THREAD
    bool keep_running = true;
    std::thread refresh_thread([&] {
        while (keep_running) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            reload_file(ast_file, ast_lines);
            reload_file(out_file, out_lines);
            screen.PostEvent(Event::Custom); 
        }
    });

    screen.Loop(component);

    keep_running = false;
    refresh_thread.join();

    return 0;
}
