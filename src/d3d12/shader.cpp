#include "shader.hpp"
namespace fgl {
ComPtr<ID3DBlob> compile_shader(const std::filesystem::path& file, const char* entry, const char* target) {
    ComPtr<ID3DBlob> bytecode, diagnostics;
    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef NDEBUG
    flags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#else
    flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
    const auto result = D3DCompileFromFile(file.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        entry, target, flags, 0, &bytecode, &diagnostics);
    if (FAILED(result)) {
        const std::string detail = diagnostics ? std::string(static_cast<const char*>(diagnostics->GetBufferPointer()), diagnostics->GetBufferSize()) : "no compiler diagnostic";
        throw GpuFailure("ShaderCompile", "file=" + utf8(file.c_str()) + " entry=" + entry + " target=" + target + " diagnostic=" + detail, result);
    }
    return bytecode;
}
}
