#pragma once

#include <cassert>
#include <climits>
#include <cmath>
#include <cinttypes>
#include <cstdlib>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <functional>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

typedef uint8_t u8;
typedef int8_t s8;
typedef uint16_t u16;
typedef int16_t s16;
typedef uint32_t u32;
typedef int32_t s32;
typedef uint64_t u64;
typedef int64_t s64;

struct u128
{
    u64 lo;
    u64 hi;

    u128();
    u128(const u128& other);
};