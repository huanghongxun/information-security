#include "des.h"
#include "permutation.h"
#include "binary.h"
#include <string.h>

/**
 * @brief Feistel 轮函数
 * @param r 长度为 32 位的串 R[i - 1]
 * @param k 长度为 48 位的子密钥 k[i]
 * @return 32 位输出
 */
static des_value_t feistel(des_value_t r, des_value_t k)
{
    // step1: 将长度为32位的串Ri-1 作E-扩展，成为48位的串E(Ri-1)
    des_value_t er = do_permutation(&PERM_E_EXTENSION, r);
    // step2: 将E(Ri-1) 和长度为48位的子密钥Ki 作48位二进制串按位异或运算，Ki 由密钥K生成
    des_value_t g = er ^ k;
    // step3: 将(2) 得到的结果平均分成8个分组，每个分组长度6位。各个
    // 分组分别经过8个不同的S-盒进行6-4 转换，得到8个长度分别为4位的分组
    // step4: 将(3) 得到的分组结果顺序连接得到长度为32位的串；
    // clang-format off
    des_value_t s =
        (do_sbox(S1_BOX, (g >> 42) & 0x3F) << 28) |
        (do_sbox(S2_BOX, (g >> 36) & 0x3F) << 24) |
        (do_sbox(S3_BOX, (g >> 30) & 0x3F) << 20) |
        (do_sbox(S4_BOX, (g >> 24) & 0x3F) << 16) |
        (do_sbox(S5_BOX, (g >> 18) & 0x3F) << 12) |
        (do_sbox(S6_BOX, (g >> 12) & 0x3F) <<  8) |
        (do_sbox(S7_BOX, (g >>  6) & 0x3F) <<  4) |
        (do_sbox(S8_BOX, (g >>  0) & 0x3F) <<  0);
    // clang-format on
    // step5: 将(4)的结果经过P-置换，得到的结果作为轮函数f(Ri-1, Ki) 的最终32位输出
    return do_permutation(&PERM_P, s);
}

/**
 * @brief 生成子密钥
 * @param key 64 位密钥 K
 * @param subkeys 生成的 16 个 48 位子密钥
 */
static void calc_subkey(des_value_t key, des_value_t subkeys[16])
{
    static int SHIFT_BITS[] = {1, 1, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 1};

    key = do_permutation(&PERM_REMOVE_PARITY, key);
    key = do_permutation(&PERM_PC1, key);
    des_value_t c = (key >> 28) & 0xFFFFFFF, d = key & 0xFFFFFFF;
    for (int i = 0; i < 16; ++i)
    {
        c = loop_shl(c, 28, SHIFT_BITS[i]);
        d = loop_shl(d, 28, SHIFT_BITS[i]);
        des_value_t cd = (c << 28) | d;
        subkeys[i] = do_permutation(&PERM_PC2, cd);
    }
}

/**
 * @param m 上一次 T 迭代的结果 M[i-1]=L[i-1]R[i-1]
 * @param k 长度为 48 位的子密钥 k[i]
 * @return L[i]R[i]
 */
static des_value_t t_iteration(des_value_t m, des_value_t k)
{
    des_value_t l = (m >> 32) & 0xFFFFFFFF, r = m & 0xFFFFFFFF;
    return (r << 32) | (l ^ feistel(r, k));
}

static des_value_t des_chunk(des_value_t m, des_value_t key, int mode)
{
    // C = E_k(M) = IP^(-1)·W·T_16·...·T_1·IP(M)
    des_value_t subkeys[16];
    m = do_permutation(&PERM_IP, m);
    calc_subkey(key, subkeys);
    for (int i = 0; i < 16; ++i)
        if (mode == DES_ENCRYPT)
            m = t_iteration(m, subkeys[i]);
        else
            m = t_iteration(m, subkeys[15 - i]);
    m = do_permutation(&PERM_SWITCH, m);
    m = do_permutation(&PERM_IPINV, m);
    return m;
}

static void des_block(char m[], des_value_t key, char c[], int mode)
{
    des_value_t mm = join_uint64(m);
    des_value_t res = des_chunk(mm, key, mode);
    // clang-format off
    c[0] = (res >> 56) & 0xFF;
    c[1] = (res >> 48) & 0xFF;
    c[2] = (res >> 40) & 0xFF;
    c[3] = (res >> 32) & 0xFF;
    c[4] = (res >> 24) & 0xFF;
    c[5] = (res >> 16) & 0xFF;
    c[6] = (res >>  8) & 0xFF;
    c[7] = (res >>  0) & 0xFF;
    // clang-format on
}

uint64_t des_generate_key()
{
    char key[8];
    for (int i = 0; i < 8; ++i)
    {
        key[i] = rand() & 0xFE;
        key[i] |= count_bits(key[i], 0xFF) % 2 == 1 ? 0 : 1; // odd parity
    }
    return join_uint64(key);
}

bool des_verify_key(uint64_t key)
{
    char skey[8];
    split_uint64(key, skey);
    for (int i = 0; i < 8; ++i)
        if (count_bits(skey[i], 0xFF) % 2 != 1) // odd parity
            return false;
    return true;
}

int des(struct des_stream *stream, char m[], int len, uint64_t key, char c[], int clen)
{
    int olen = 0;

    if (stream->has_buf && stream->mode == DES_DECRYPT)
    {
        if (clen - olen < 8)
            return -1;
        memcpy(c + olen, stream->buf, 8);
        olen += 8;
        stream->has_buf = false;
    }

    while (stream->len + len >= 8)
    {
        memcpy(stream->m + stream->len, m, 8 - stream->len);
        m += 8 - stream->len;
        len -= 8 - stream->len;
        if (clen - olen < 8)
            return -1;
        des_block(stream->m, key, c + olen, stream->mode);
        olen += 8;
        stream->len = 0;
    }

    if (olen >= 8 && stream->mode == DES_DECRYPT)
    {
        olen -= 8;
        memcpy(stream->buf, c + olen, 8);
        stream->has_buf = true;
    }

    {
        memcpy(stream->m + stream->len, m, len);
        stream->len += len;
    }

    return olen;
}

struct des_stream *des_open(int mode)
{
    struct des_stream *des = (struct des_stream *)malloc(sizeof(struct des_stream));
    des->len = 0;
    des->mode = mode;
    des->has_buf = false;
    return des;
}

int des_close(struct des_stream *stream, uint64_t key, char c[], int clen)
{
    int ret = 0;
    if (stream->mode == DES_ENCRYPT)
    {
        ret = 8;
        for (int i = stream->len; i < 8; ++i)
            stream->m[i] = 8 - stream->len;
        if (clen < ret)
        {
            ret = -1;
            goto end;
        }
        des_block(stream->m, key, c, stream->mode);
    }
    else
    {
        if (stream->len != 0) // 密文长度必须为 64-bit 的倍数
        {
            ret = -1;
            goto end;
        }
        if (stream->has_buf)
        {
            int nlen = stream->buf[7];
            ret = 8 - nlen;
            if (clen < ret)
            {
                ret = -1;
                goto end;
            }
            memcpy(c, stream->buf, ret);
        }
    }
end:
    free(stream);
    return ret;
}
