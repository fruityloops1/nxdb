#pragma once

#ifdef LIBNX
#include "LogServer.h"
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
#else
#include "main.h"
#include <cstdio>
#include <cstdlib>
#define PENET_MALLOC(SIZE) buddyMalloc(SIZE)
#define PENET_FREE(PTR) buddyFree(PTR)
#define PENET_ABORT(FMT, ...)              \
    {                                      \
        fprintf(stderr, FMT, __VA_ARGS__); \
        fflush(stderr);                    \
        abort();                           \
    }
#define PENET_WARN(FMT, ...) \
    { fprintf(stdout, FMT, __VA_ARGS__); }
#endif
