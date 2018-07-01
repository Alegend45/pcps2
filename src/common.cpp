#include "common.h"

u128::u128()
{
    lo = hi = 0;
}

u128::u128(const u128& other)
{
    lo = other.lo;
    hi = other.hi;
}