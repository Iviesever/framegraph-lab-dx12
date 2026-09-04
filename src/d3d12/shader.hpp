#pragma once
#include "platform.hpp"
#include <d3dcompiler.h>
#include <filesystem>
namespace fgl {
ComPtr<ID3DBlob> compile_shader(const std::filesystem::path& file, const char* entry, const char* target);
}
