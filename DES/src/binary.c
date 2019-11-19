#include "binary.h"

int get_bit(uint64_t chunk, int bit)
{
    return (chunk >> bit) & 1;
}

uint64_t set_bit(uint64_t chunk, int bit, int num)
{
    if (num)
        return chunk | (1 << bit);
    else
        return chunk & ~(1 << bit);
}

uint64_t loop_shl(uint64_t chunk, int len, int bits)
{
    for (int i = 0; i < bits; ++i)
    {
        chunk = (chunk << 1) | ((chunk >> (len - 1)) & 1);
    }
    return chunk & ~(~0 << len);
}

int count_bits(uint64_t chunk, uint64_t mask)
{
    int bits = 0;
    chunk &= mask;
    for (; chunk; chunk -= chunk & -chunk)
        ++bits;
    return bits;
}

uint64_t join_uint64(char c[])
{
    uint64_t res = 0;
    for (int i = 0; i < 8; ++i)
        res |= ((uint64_t)c[i] & 0xFF) << ((7 - i) * 8);
    return res;
}

void split_uint64(uint64_t x, char c[])
{
    for (int i = 0; i < 8; ++i)
        c[i] = (x >> ((7 - i) * 8)) & 0xFF;
}
