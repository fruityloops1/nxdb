#pragma once

#include "imgui.h"

namespace pe {

    ImDrawData* unpackDrawData(void* data, size_t size, ImGuiMouseCursor* outCursor);

} // namespace pe
