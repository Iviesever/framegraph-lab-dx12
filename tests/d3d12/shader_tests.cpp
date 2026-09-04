#include "d3d12/shader.hpp"
#include "../unit/test_support.hpp"
namespace { std::filesystem::path root; }
CASE(valid_shader_compiles) {
    const auto blob = fgl::compile_shader(root / "tests/d3d12/fixtures/valid.hlsl", "PSMain", "ps_5_0");
    CHECK(blob && blob->GetBufferSize() > 0);
}
CASE(shader_error_preserves_context) {
    try { (void)fgl::compile_shader(root / "tests/d3d12/fixtures/invalid.hlsl", "PSMain", "ps_5_0"); CHECK(false); }
    catch (const fgl::GpuFailure& error) {
        CHECK(error.category == "ShaderCompile"); const std::string text = error.what();
        CHECK(text.find("invalid.hlsl") != std::string::npos); CHECK(text.find("PSMain") != std::string::npos);
        CHECK(text.find("ps_5_0") != std::string::npos); CHECK(text.find("unknown_colour") != std::string::npos);
    }
}
CASE(missing_shader_is_typed) {
    try { (void)fgl::compile_shader(root / "tests/d3d12/fixtures/missing.hlsl", "PSMain", "ps_5_0"); CHECK(false); }
    catch (const fgl::GpuFailure& error) { CHECK(error.category == "ShaderCompile"); CHECK(std::string(error.what()).find("missing.hlsl") != std::string::npos); }
}
CASE(missing_entry_is_typed) {
    try { (void)fgl::compile_shader(root / "tests/d3d12/fixtures/valid.hlsl", "MissingEntry", "ps_5_0"); CHECK(false); }
    catch (const fgl::GpuFailure& error) { CHECK(error.category == "ShaderCompile"); CHECK(std::string(error.what()).find("MissingEntry") != std::string::npos); }
}
int main(int argc, char** argv) { root = argc > 1 ? std::filesystem::path(argv[1]) : std::filesystem::current_path(); return test::run(); }
