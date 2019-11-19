#ifndef MD5_H
#define MD5_H
#include <stdint.h>

struct md5_stream
{
    // 缓存的 512 位块，MD5 要求将数据切割为数个 512-bit 分组
    // 如果数据无法填满，则填充，若无足够空位，则填充至下一个块
    uint8_t data[64]; // 512-bits
    int len;
    uint64_t total_len;

    uint32_t vector[4];
};

void md5_stream_begin(struct md5_stream *stream);

void md5_stream_data(struct md5_stream *stream, uint8_t data[], int len);

void md5_stream_end(struct md5_stream *stream, uint8_t result[16]);

#endif