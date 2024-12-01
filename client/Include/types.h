#pragma once

#include <cstddef>
#include <cstdint>

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef size_t size;

#define ALWAYSINLINE __attribute__((always_inline)) inline
#define UNROLL __attribute__((optimize("unroll-loops")))
#define NOINLINE __attribute__((noinline))
#define ALIGN_PTR(ptr, alignment) \
    (((ptr) + ((alignment) - 1)) & ~((alignment) - 1))

inline u64 constexpr operator""_B(unsigned long long b) { return b; }
inline u64 constexpr operator""_KB(unsigned long long kb) { return kb * 1024; }
inline u64 constexpr operator""_MB(unsigned long long mb) { return mb * 1024_KB; }
inline u64 constexpr operator""_GB(unsigned long long gb) { return gb * 1024_MB; }
