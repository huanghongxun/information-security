#include "binary.h"

// 循环左移
uint32_t loop_shl(uint32_t chunk, uint32_t len, uint32_t bits)
{
    return (chunk << bits) | (chunk >> (len - bits));
}

// 该函数能确保是小端制读取数据
uint32_t join_uint32(const uint8_t c[])
{
    uint32_t res = 0;
    for (int i = 0; i < 4; ++i)
        res |= ((uint32_t)c[i] & 0xFF) << (i * 8);
    return res;
}

// 该函数能确保是小端制写入数据
void split_uint32(uint32_t x, uint8_t c[])
{
    for (int i = 0; i < 4; ++i)
        c[i] = (x >> (i * 8)) & 0xFF;
}

// 该函数能确保是小端制写入数据
void split_uint64(uint64_t x, uint8_t c[])
{
    for (int i = 0; i < 8; ++i)
        c[i] = (x >> (i * 8)) & 0xFF;
}
