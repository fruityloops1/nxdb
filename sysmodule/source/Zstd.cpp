#include "Zstd.h"
#include "LogServer.h"
#include "nxdb_imgui_config.h"
#include <cstdlib>

namespace nxdb {

    static ZSTD_DDict* sDDict = nullptr;
    static ZSTD_CDict* sCDict = nullptr;
    constexpr char sDictFile[] = "dictionary.zsdic";

    ZSTD_DDict* getZstdDDict() {
        if (sDDict == nullptr) {
            nxdb::log("loading dictionary %s\n", sDictFile);

            FILE* f = fopen(sDictFile, "rb");
            fseek(f, 0L, SEEK_END);
            size_t dictSize = ftell(f);
            fseek(f, 0L, SEEK_SET);
            void* const dictBuffer = malloc(dictSize);
            fclose(f);

            sDDict = ZSTD_createDDict(dictBuffer, dictSize);
            IM_ASSERT(sDDict != nullptr);
            free(dictBuffer);
        }
        return sDDict;
    }

    ZSTD_CDict* getZstdCDict() {
        if (sCDict == nullptr) {
            nxdb::log("loading dictionary %s\n", sDictFile);

            FILE* f = fopen(sDictFile, "rb");
            fseek(f, 0L, SEEK_END);
            size_t dictSize = ftell(f);
            fseek(f, 0L, SEEK_SET);
            void* const dictBuffer = malloc(dictSize);
            fclose(f);

            sCDict = ZSTD_createCDict(dictBuffer, dictSize, 3);
            IM_ASSERT(sCDict != nullptr);
            free(dictBuffer);
        }
        return sCDict;
    }

} // namespace nxdb
