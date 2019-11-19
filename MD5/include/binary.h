#ifndef BINARY_H
#define BINARY_H

#include <stdint.h>

/**
 * @brief 将 chunk 循环左移 bits 位
 * @param chunk 被循环左移的数字
 * @param len chunk 的位数
 * @param bits 左移的位数
 */
uint32_t loop_shl(uint32_t chunk, uint32_t len, uint32_t bits);

/**
 * @brief 将字符数组合成 uint32，确保字符数组为小端制
 * @param c 要合成的字符数组
 * @note 注意不可以直接强制类型转换
 * @return 合成的 uint632
 */
uint32_t join_uint32(const uint8_t c[]);

void split_uint32(uint32_t i, uint8_t c[]);

void split_uint64(uint64_t x, uint8_t c[]);

#endif