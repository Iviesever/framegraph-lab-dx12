#include "options.hpp"
#include "report.hpp"
#include "d3d12/runtime.hpp"
#include <fstream>
#include <iostream>
#include <vector>

int main(int argc, char** argv) {
    std::vector<std::string_view> arguments;
    for (int i = 1; i < argc; ++i) arguments.emplace_back(argv[i]);
    const auto options = fgl::parse_options(arguments);
    if (!options) { std::cerr << options.error().message << '\n'; return 2; }
    if (options->help) { std::cout << fgl::usage_text(); return 0; }
    try {
        auto report = fgl::run_clear_demo(*options);
        const auto json = report.json();
        if (!options->report.empty()) {
            if (options->report.has_parent_path()) std::filesystem::create_directories(options->report.parent_path());
            std::ofstream output(options->report, std::ios::binary);
            output << json << '\n'; output.close();
            if (!output) throw std::runtime_error("failed to write runtime report");
        }
        std::cout << json << '\n';
        return report.success ? 0 : 1;
    } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
