#ifndef BINARY_H
#define BINARY_H

#include <stdint.h>

/**
 * @brief 获取 chunk 的第 bit 位的值
 * @param chunk
 * @param bit 范围在 0~63 内
 * @return chunk 的第 bit 位的值
 */
int get_bit(uint64_t chunk, int bit);

/**
 * @brief 设置 chunk 的第 bit 位的值为 num
 * @param chunk
 * @param bit 范围在 0~63 内
 * @param num 范围在 0~1 内
 * @return 修改后的 chunk
 */
uint64_t set_bit(uint64_t chunk, int bit, int num);

/**
 * @brief 将 chunk 循环左移 bits 位
 * @param chunk 被循环左移的数字
 * @param len chunk 的位数
 * @param bits 左移的位数
 */
uint64_t loop_shl(uint64_t chunk, int len, int bits);

/**
 * @brief 计算 chunk 二进制表示下中 1 的个数
 * @param chunk
 * @param mask 选择哪些位统计
 * @return 1 的个数
 */
int count_bits(uint64_t chunk, uint64_t mask);

/**
 * @brief 将字符数组合成 uint64
 * @note 注意不可以直接强制类型转换，x86 是小端模式。
 * @return 合成的 uint64
 */
uint64_t join_uint64(char c[]);

void split_uint64(uint64_t i, char c[]);

#endif