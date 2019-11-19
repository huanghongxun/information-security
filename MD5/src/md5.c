#include "md5.h"
#include <stdlib.h>
#include <string.h>
#include "binary.h"

static const uint32_t T[4][16] = {
    {0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
     0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
     0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
     0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821},
    {0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
     0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
     0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
     0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a},
    {0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
     0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
     0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
     0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665},
    {0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
     0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
     0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
     0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391}};

static const uint32_t S[4][16] = {
    {7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22},
    {5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20},
    {4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23},
    {6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21},
};

static const int X[4][16] = {
    {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
    {1, 6, 11, 0, 5, 10, 15, 4, 9, 14, 3, 8, 13, 2, 7, 12},
    {5, 8, 11, 14, 1, 4, 7, 10, 13, 0, 3, 6, 9, 12, 15, 2},
    {0, 7, 14, 5, 12, 3, 10, 1, 8, 15, 6, 13, 4, 11, 2, 9}};

static const uint8_t PADDING[64] = {
    0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

static uint32_t F(uint32_t b, uint32_t c, uint32_t d) { return (b & c) | (~b & d); }
static uint32_t G(uint32_t b, uint32_t c, uint32_t d) { return (b & d) | (c & ~d); }
static uint32_t H(uint32_t b, uint32_t c, uint32_t d) { return b ^ c ^ d; }
static uint32_t I(uint32_t b, uint32_t c, uint32_t d) { return c ^ (b | ~d); }
static uint32_t (*GEN[4])(uint32_t, uint32_t, uint32_t) = {F, G, H, I};

static void md5_compress(uint32_t abcd[4], uint32_t (*g)(uint32_t, uint32_t, uint32_t), uint32_t xk, uint32_t ti, uint32_t s)
{
    uint32_t a = abcd[0], b = abcd[1], c = abcd[2], d = abcd[3];
    abcd[0] = d;
    abcd[1] = b + loop_shl(a + g(b, c, d) + xk + ti, 32, s);
    abcd[2] = b;
    abcd[3] = c;
}

static void md5_block(uint32_t vq[4], const uint8_t y[64])
{
    uint32_t abcd[4] = {vq[0], vq[1], vq[2], vq[3]};
    for (int round = 0; round < 4; ++round)
        for (int i = 0; i < 16; ++i)
            md5_compress(abcd, GEN[round], join_uint32(y + X[round][i] * 4), T[round][i], S[round][i]);
    for (int i = 0; i < 4; ++i)
        vq[i] += abcd[i];
}

void md5_stream_begin(struct md5_stream *stream)
{
    stream->vector[0] = 0x67452301;
    stream->vector[1] = 0xEFCDAB89;
    stream->vector[2] = 0x98BADCFE;
    stream->vector[3] = 0x10325476;
    stream->len = 0;
    stream->total_len = 0;
}

void md5_stream_data(struct md5_stream *stream, uint8_t data[], int len)
{
    while (len > 0)
    {
        int delta = 64 - stream->len < len ? 64 - stream->len : len;
        memcpy(stream->data + stream->len, data, delta);
        data += delta;
        len -= delta;
        stream->len += delta;
        stream->total_len += delta;
        if (stream->len >= 64)
        {
            md5_block(stream->vector, stream->data);
            stream->len = 0;
        }
    }
}

void md5_stream_end(struct md5_stream *stream, uint8_t result[16])
{
    int filled = 0;
    if (stream->len >= 56)
    {
        // 此时 512 位块仅剩不多于 64 位的空闲位置，不足以同时存放至少一位的填充位和消息位数，需要新增一块
        while (stream->len < 64)
            stream->data[stream->len++] = PADDING[filled++];
        md5_block(stream->vector, stream->data);
        stream->len = 0;
    }
    while (stream->len < 56)
        stream->data[stream->len++] = PADDING[filled++];
    split_uint64(stream->total_len * 8, stream->data + 56);
    md5_block(stream->vector, stream->data);
    stream->len = 0;

    for (int i = 0; i < 4; ++i)
        split_uint32(stream->vector[i], result + i * 4);
}
