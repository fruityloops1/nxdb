#pragma once

#include "imgui.h"

namespace nxdb {

    void packImDrawData(void* out, size_t* outSize, ImDrawData* data);

} // namespace nxdb
