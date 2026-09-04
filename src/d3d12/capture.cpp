#include "capture.hpp"
#include <wincodec.h>
#include <algorithm>
#include <cstring>
namespace fgl {
ReadbackLayout readback_layout(ID3D12Device* device, std::uint32_t width, std::uint32_t height) {
    D3D12_RESOURCE_DESC desc{}; desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D; desc.Width = width; desc.Height = height;
    desc.DepthOrArraySize = 1; desc.MipLevels = 1; desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; desc.SampleDesc.Count = 1;
    ReadbackLayout layout; layout.width = width; layout.height = height;
    device->GetCopyableFootprints(&desc, 0, 1, 0, &layout.footprint, nullptr, nullptr, &layout.bytes);
    if (!layout.bytes || layout.bytes == UINT64_MAX) throw GpuFailure("CaptureLayout", "invalid readback footprint");
    return layout;
}
ComPtr<ID3D12Resource> readback_buffer(ID3D12Device* device, std::uint64_t bytes) {
    D3D12_HEAP_PROPERTIES heap{}; heap.Type = D3D12_HEAP_TYPE_READBACK; heap.CreationNodeMask = heap.VisibleNodeMask = 1;
    D3D12_RESOURCE_DESC desc{}; desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER; desc.Width = bytes; desc.Height = 1;
    desc.DepthOrArraySize = 1; desc.MipLevels = 1; desc.SampleDesc.Count = 1; desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    ComPtr<ID3D12Resource> result;
    check_hr(device->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&result)), "CreateCommittedResource(readback)", device);
    return result;
}
void copy_to_readback(ID3D12GraphicsCommandList* list, ID3D12Resource* source, ID3D12Resource* destination, const ReadbackLayout& layout) {
    D3D12_TEXTURE_COPY_LOCATION from{}; from.pResource = source; from.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    D3D12_TEXTURE_COPY_LOCATION to{}; to.pResource = destination; to.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT; to.PlacedFootprint = layout.footprint;
    list->CopyTextureRegion(&to, 0, 0, 0, &from, nullptr);
}
std::vector<std::uint8_t> read_rgba(ID3D12Resource* buffer, const ReadbackLayout& layout) {
    std::vector<std::uint8_t> result(static_cast<std::size_t>(layout.width) * layout.height * 4);
    const D3D12_RANGE range{0, static_cast<SIZE_T>(layout.bytes)};
    void* mapped{}; check_hr(buffer->Map(0, &range, &mapped), "Map RGBA readback");
    for (std::uint32_t y = 0; y < layout.height; ++y)
        std::memcpy(result.data() + static_cast<std::size_t>(y) * layout.width * 4,
            static_cast<const std::uint8_t*>(mapped) + layout.footprint.Offset + static_cast<std::size_t>(y) * layout.footprint.Footprint.RowPitch,
            static_cast<std::size_t>(layout.width) * 4);
    const D3D12_RANGE written{0, 0}; buffer->Unmap(0, &written);
    return result;
}
void write_png(const std::filesystem::path& path, std::uint32_t width, std::uint32_t height, const std::vector<std::uint8_t>& rgba) {
    struct Apartment {
        HRESULT hr{CoInitializeEx(nullptr, COINIT_MULTITHREADED)};
        Apartment() { if (hr != RPC_E_CHANGED_MODE) check_hr(hr, "CoInitializeEx"); }
        ~Apartment() { if (SUCCEEDED(hr)) CoUninitialize(); }
    } apartment;
    if (rgba.size() != static_cast<std::size_t>(width) * height * 4 || rgba.size() > UINT32_MAX) throw GpuFailure("CaptureSize", "RGBA dimensions do not match");
    if (path.has_parent_path()) std::filesystem::create_directories(path.parent_path());
    ComPtr<IWICImagingFactory> factory;
    check_hr(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory)), "WIC factory");
    ComPtr<IWICStream> stream; check_hr(factory->CreateStream(&stream), "WIC stream");
    check_hr(stream->InitializeFromFilename(path.c_str(), GENERIC_WRITE), "WIC output file");
    ComPtr<IWICBitmapEncoder> encoder; check_hr(factory->CreateEncoder(GUID_ContainerFormatPng, nullptr, &encoder), "WIC PNG encoder");
    check_hr(encoder->Initialize(stream.Get(), WICBitmapEncoderNoCache), "WIC encoder Initialize");
    ComPtr<IWICBitmapFrameEncode> frame; check_hr(encoder->CreateNewFrame(&frame, nullptr), "WIC CreateNewFrame");
    check_hr(frame->Initialize(nullptr), "WIC frame Initialize"); check_hr(frame->SetSize(width, height), "WIC SetSize");
    WICPixelFormatGUID format = GUID_WICPixelFormat32bppRGBA; check_hr(frame->SetPixelFormat(&format), "WIC SetPixelFormat");
    const auto* pixels = rgba.data();
    std::vector<std::uint8_t> converted;
    if (IsEqualGUID(format, GUID_WICPixelFormat32bppBGRA)) {
        converted = rgba;
        for (std::size_t i = 0; i < converted.size(); i += 4) std::swap(converted[i], converted[i + 2]);
        pixels = converted.data();
    } else if (!IsEqualGUID(format, GUID_WICPixelFormat32bppRGBA)) throw GpuFailure("CaptureFormat", "WIC cannot encode a lossless 32-bit RGBA-compatible format");
    check_hr(frame->WritePixels(height, width * 4, static_cast<UINT>(rgba.size()), const_cast<BYTE*>(pixels)), "WIC WritePixels");
    check_hr(frame->Commit(), "WIC frame Commit"); check_hr(encoder->Commit(), "WIC encoder Commit");
}
}
