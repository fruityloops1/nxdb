#pragma once

#ifdef LIBNX
#include "LogServer.h"
#include "Zstd.h"
#include <cstdlib>
extern "C" {
#include "switch/runtime/diag.h"
}
#define PENET_ABORT(FMT, ...)        \
    {                                \
        nxdb::log(FMT, __VA_ARGS__); \
        diagAbortWithResult(0x402);  \
    }
#define PENET_WARN(FMT, ...) \
    { nxdb::log(FMT, __VA_ARGS__); }
#define PENET_MALLOC(SIZE) malloc(SIZE)
#define PENET_FREE(PTR) free(PTR)
#define PENET_GET_ZSTD_DDICT() ::nxdb::getZstdDDict()
#define PENET_GET_ZSTD_CDICT() ::nxdb::getZstdCDict()
#else
#include "main.h"
#include "pe/Render.h"
#include <cstdio>
#include <cstdlib>
#define PENET_MALLOC(SIZE) buddyMalloc(SIZE)
#define PENET_FREE(PTR) buddyFree(PTR)
#define PENET_ABORT(FMT, ...)                   \
    {                                           \
        fprintf(stderr, FMT "\n", __VA_ARGS__); \
        fflush(stderr);                         \
        abort();                                \
    }
#define PENET_WARN(FMT, ...) \
    { fprintf(stdout, FMT "\n", __VA_ARGS__); }
#define PENET_GET_ZSTD_DDICT() ::pe::getZstdDDict()
#define PENET_GET_ZSTD_CDICT() ::pe::getZstdCDict()
#endif
