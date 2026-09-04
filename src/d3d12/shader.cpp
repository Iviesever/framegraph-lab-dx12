#include "shader.hpp"
namespace fgl {
std::filesystem::path executable_directory() {
    std::wstring buffer(32768, L'\0');
    const auto length = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
    if (!length || length >= buffer.size()) throw GpuFailure("ExecutablePath", "GetModuleFileNameW failed or exceeded the bounded path buffer", HRESULT_FROM_WIN32(GetLastError()));
    buffer.resize(length); return std::filesystem::path(buffer).parent_path();
}
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
