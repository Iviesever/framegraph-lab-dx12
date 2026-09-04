#include "framegraph/graph.hpp"
#include <iomanip>
#include <locale>
#include <sstream>

namespace framegraph {
std::string json_quote(const std::string& text) {
    constexpr char hex[] = "0123456789abcdef";
    std::string out = "\"";
    for (std::size_t i = 0; i < text.size(); ++i) {
        const auto c = static_cast<unsigned char>(text[i]);
        if (c >= 128) {
            const unsigned count = c >= 0xc2 && c <= 0xdf ? 2 : c >= 0xe0 && c <= 0xef ? 3 : c >= 0xf0 && c <= 0xf4 ? 4 : 0;
            bool valid = count && count <= text.size() - i;
            std::uint32_t point = c & (count == 2 ? 31u : count == 3 ? 15u : 7u);
            for (unsigned j = 1; valid && j < count; ++j) {
                const auto continuation = static_cast<unsigned char>(text[i + j]);
                valid = (continuation & 0xc0) == 0x80;
                point = (point << 6) | (continuation & 0x3f);
            }
            valid = valid && point <= 0x10ffff && !(point >= 0xd800 && point <= 0xdfff)
                && (count != 3 || point >= 0x800) && (count != 4 || point >= 0x10000);
            if (valid) { out.append(text, i, count); i += count - 1; continue; }
        }
        switch (c) {
        case '"': out += "\\\""; break;
        case '\\': out += "\\\\"; break;
        case '\n': out += "\\n"; break;
        case '\r': out += "\\r"; break;
        case '\t': out += "\\t"; break;
        default:
            // Preserve valid UTF-8 above; escape individual malformed bytes for diagnostics.
            if (c < 32 || c >= 127) { out += "\\u00"; out += hex[c >> 4]; out += hex[c & 15]; }
            else out += static_cast<char>(c);
        }
    }
    return out + '"';
}
std::string stable_hash(const std::string& bytes) {
    std::uint64_t hash = 14695981039346656037ull;
    for (const unsigned char c : bytes) { hash ^= c; hash *= 1099511628211ull; }
    std::ostringstream out;
    out.imbue(std::locale::classic());
    out << std::hex << std::setfill('0') << std::setw(16) << hash;
    return out.str();
}
namespace {
std::string payload(const CompiledGraph& g) {
    std::ostringstream out;
    out.imbue(std::locale::classic());
    out << "{\"schema_version\":1,\"scene_seed\":" << g.description.scene_seed << ",\"resources\":[";
    for (std::size_t i = 0; i < g.description.resources.size(); ++i) {
        if (i) out << ',';
        const auto& r = g.description.resources[i];
        out << "{\"id\":" << i << ",\"name\":" << json_quote(r.name);
        if (const auto* t = std::get_if<TextureDesc>(&r.descriptor)) {
            out << ",\"kind\":\"texture\",\"width\":" << t->width << ",\"height\":" << t->height
                << ",\"format\":" << static_cast<unsigned>(t->format) << ",\"render_target\":" << t->render_target
                << ",\"depth_stencil\":" << t->depth_stencil << ",\"unordered_access\":" << t->unordered_access;
        } else {
            const auto& b = std::get<BufferDesc>(r.descriptor);
            out << ",\"kind\":\"buffer\",\"bytes\":" << b.bytes << ",\"unordered_access\":" << b.unordered_access;
        }
        out << ",\"imported\":" << r.imported << ",\"initialized\":" << r.initialized << ",\"exported\":" << r.exported
            << ",\"initial_state\":" << static_cast<unsigned>(r.initial_state) << ",\"final_state\":";
        if (r.final_state) out << static_cast<unsigned>(*r.final_state); else out << "null";
        out << ",\"lifetime\":";
        if (i < g.lifetimes.size() && g.lifetimes[i]) out << "{\"first\":" << g.lifetimes[i]->first << ",\"last\":" << g.lifetimes[i]->last << '}';
        else out << "null";
        out << '}';
    }
    out << "],\"passes\":[";
    for (std::size_t i = 0; i < g.description.passes.size(); ++i) {
        if (i) out << ',';
        const auto& p = g.description.passes[i];
        out << "{\"id\":" << i << ",\"name\":" << json_quote(p.name) << ",\"side_effect\":" << p.side_effect << ",\"usages\":[";
        for (std::size_t j = 0; j < p.usages.size(); ++j) {
            if (j) out << ',';
            const auto& u = p.usages[j];
            out << "{\"resource\":" << u.resource.value << ",\"access\":" << static_cast<unsigned>(u.access) << ",\"usage\":" << static_cast<unsigned>(u.usage) << '}';
        }
        out << "]}";
    }
    out << "],\"order\":[";
    for (std::size_t i = 0; i < g.passes.size(); ++i) { if (i) out << ','; out << g.passes[i].id.value; }
    out << "],\"culled\":[";
    for (std::size_t i = 0; i < g.culled.size(); ++i) { if (i) out << ','; out << g.culled[i].value; }
    out << "],\"dependencies\":[";
    for (std::size_t i = 0; i < g.dependencies.size(); ++i) {
        if (i) out << ',';
        const auto& e = g.dependencies[i];
        out << "{\"before\":" << e.before.value << ",\"after\":" << e.after.value << ",\"resource\":";
        if (e.resource.value == invalid_index) out << "null"; else out << e.resource.value;
        out << ",\"hazard\":" << static_cast<unsigned>(e.hazard) << '}';
    }
    out << "],\"declared_ordering\":[";
    for (std::size_t i = 0; i < g.description.ordering.size(); ++i) {
        if (i) out << ',';
        const auto& e = g.description.ordering[i]; out << '[' << e.before.value << ',' << e.after.value << ']';
    }
    out << "]}";
    return out.str();
}
}
std::string plan_identity(const CompiledGraph& graph) { return stable_hash(payload(graph)); }
std::string canonical_json(const CompiledGraph& graph) {
    auto value = payload(graph);
    const auto identity = stable_hash(value);
    value.pop_back();
    return value + ",\"plan_identity\":\"" + identity + "\"}";
}
}
