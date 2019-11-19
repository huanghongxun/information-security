#ifndef DES_H
#define DES_H

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

/**
 * DES 数据格式，长度可能为 32、48、56、64 位。
 * 最低位为第 0 位，因此输入的密钥需要翻转后使用。
 */
typedef uint64_t des_value_t;

#define DES_KEY_SIZE 8
#define DES_ENCRYPT 0
#define DES_DECRYPT 1

struct des_stream
{
    // DES 64 位明文块/密文块
    char m[8];

    // 缓冲区，DES 解密时需要对最后一个密文块特殊操作
    // 这里总是缓存最新解密得到的明文块
    char buf[8];
    bool has_buf;

    // 已经存储的字节数
    int len;
    // 加密还是解密模式
    int mode;
};

struct des_stream *des_open(int mode);

/**
 * @brief 关闭 DES 加密流
 * @param c 输出的密文数组，需要确保该数组长度为 8 以上
 * @param clen 密文数组最大长度
 * @return 生成的密文长度
 */
int des_close(struct des_stream *stream, uint64_t key, char c[], int clen);

/**
 * @brief DES 加密算法
 * @param m 明文
 * @param key 64 位密钥
 * @param c 密文，需要确保该数组长度为 8 以上
 * @param clen 密文数组最大长度
 * @return 生成的密文长度
 */
int des(struct des_stream *stream, char m[], int len, uint64_t key, char c[], int clen);

/**
 * @brief 生成 DES 使用的随机密钥
 * @return DES 使用的随机密钥
 */
uint64_t des_generate_key();

/**
 * @brief 检查 DES 密钥是否合法
 */
bool des_verify_key(uint64_t key);

#endif // DES_H