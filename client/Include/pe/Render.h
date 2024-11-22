#pragma once

#include "imgui.h"
#include "zstd.h"

namespace pe {

    void render(ImDrawData* data);
    ZSTD_DDict* getZstdDDict();
    inline ZSTD_CDict* getZstdCDict() { return nullptr; }

} // namespace pesDictFile
