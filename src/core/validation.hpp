#pragma once
#include "framegraph/graph.hpp"

namespace framegraph::detail {
Result<void> validate(const GraphDescription& graph);
inline bool reads(ResourceAccess access) noexcept { return access != ResourceAccess::Write; }
inline bool writes(ResourceAccess access) noexcept { return access != ResourceAccess::Read; }
}
