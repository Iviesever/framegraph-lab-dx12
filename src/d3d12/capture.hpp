#pragma once
#include "platform.hpp"
#include <cstdint>
#include <filesystem>
#include <vector>
namespace fgl {
struct ReadbackLayout {
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint{};
    std::uint64_t bytes{};
    std::uint32_t width{}, height{};
};
ReadbackLayout readback_layout(ID3D12Device* device, std::uint32_t width, std::uint32_t height);
ComPtr<ID3D12Resource> readback_buffer(ID3D12Device* device, std::uint64_t bytes);
void copy_to_readback(ID3D12GraphicsCommandList* list, ID3D12Resource* source, ID3D12Resource* destination, const ReadbackLayout& layout);
std::vector<std::uint8_t> read_rgba(ID3D12Resource* buffer, const ReadbackLayout& layout);
void write_png(const std::filesystem::path& path, std::uint32_t width, std::uint32_t height, const std::vector<std::uint8_t>& rgba);
}
