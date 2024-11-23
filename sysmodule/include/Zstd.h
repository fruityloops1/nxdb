#pragma once

#include "zstd.h"

namespace nxdb {

    ZSTD_DDict* getZstdDDict();
    ZSTD_CDict* getZstdCDict();

} // namespace nxdb
