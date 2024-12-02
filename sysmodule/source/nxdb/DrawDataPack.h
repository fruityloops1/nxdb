#pragma once

#include "imgui.h"
#include "types.h"

namespace nxdb {

    size packImDrawData(void* out, ImDrawData* data);
    u32 hashImDrawList(ImDrawList* list);

} // namespace nxdb
