#ifndef PERMUTATION_H
#define PERMUTATION_H

#include <stdint.h>

typedef struct
{
    int from_bits;
    int to_bits;
    int table[64];
} permutation_t;

/**
 * @brief 根据置换表 table，将 chunk 进行置换
 * @param perm 置换表，表中每个元素 e 表示对应位置 i 置换为 chunk[e]
 * @param chunk 将被置换的数组
 * @return 置换后的数组
 */
uint64_t do_permutation(const permutation_t *perm, uint64_t chunk);

/**
 * @brief 做 DES 的 S-Box 选择
 * @note S-Box 选择函数是 6 位转 4 位的变换。
 * 假设 Si=(abcdef)2，那么 n=(af)2 确定行号，m=(bcde)2 确定列号
 * @param chunk 二进制位数为 6 的 Feistel 轮函数分组
 * @return 二进制位数为 4 的选择后的分组
 */
uint64_t do_sbox(const int box[4][16], uint64_t chunk);

/**
 * IP 置换表
 */
extern const permutation_t PERM_IP;

/**
 * IP 逆置换表
 */
extern const permutation_t PERM_IPINV;

/**
 * 交换高低 32 位
 */
extern const permutation_t PERM_SWITCH;

/**
 * Feistel 轮函数的 E 扩展表
 */
extern const permutation_t PERM_E_EXTENSION;

extern const int S1_BOX[4][16];
extern const int S2_BOX[4][16];
extern const int S3_BOX[4][16];
extern const int S4_BOX[4][16];
extern const int S5_BOX[4][16];
extern const int S6_BOX[4][16];
extern const int S7_BOX[4][16];
extern const int S8_BOX[4][16];
extern const int (*S_BOX[8])[4][16];

/**
 * Feistel 轮函数的 P 置换表
 */
extern const permutation_t PERM_P;

/**
 * 子密钥生成时对密钥 K 使用的置换 PC-1
 */
extern const permutation_t PERM_PC1;

/**
 * 子密钥生成时对 CiDi 使用的置换 PC-2
 */
extern const permutation_t PERM_PC2;

/**
 * 生成子密钥时去除校验位的变换
 */
extern const permutation_t PERM_REMOVE_PARITY;

#endif // PERMUTATION_H