# DES

## 概念

DES 标准是基于 56 位密钥（64 位密钥中包含 8 位校验位，实际有效位 56 位）。

DES 算法仅讨论一个块(64位)的加密方式。对于字节流，DES 允许通过分块的方式来进行加密。

## 模块

DES 算法可以分为以下模块：

1. 变换模块 —— 保存 DES 算法中的所有变换，包括 IP、IP-1、P、PC1、PC2
2. S盒模块 —— 保存并实现 DES 算法中的 S 盒变换
3. 主模块 —— 实现 DES 算法的 Feistel 轮函数、T 迭代、以及最终 DES 算法的对块加密流程模块
4. 密钥生成模块 —— 实现生成 DES 算法的对称加密密钥
5. 流模块 —— 实现 DES 算法的不定长明文字节流加密（包括对块的填充）

模块的依赖关系如下图所示：（然而并没有图）

## 数据结构

DES 每次仅对一个 64 位的块进行操作，操作也只有移位和置换，因此时用到的数据结构仅包含数组，用于保存置换表。而对块的操作都可以在 64 位整数中完成运算。

## 复杂度

DES 算法对于指定大小的块（8 个字节）进行加密的时间复杂度是 $O(1)$。

根据 DES 算法的块加解密过程：

1. 对 64 位块进行 IP 置换，而定长置换的时间复杂度为 $O(1)$
2. 计算子密钥，每计算一个子密钥需要进行 1 次 PC1 置换，并进行 16 次循环移位和 PC2 置换，时间复杂度为 $O(1)$，最后计算子密钥过程总时间复杂度为 $O(1)$
3. Feistel 轮函数先进行一次 E 扩展变换，再调用 8 次 S-盒变换，最后进行 P 置换，每个变换都是 $O(1)$ 的时间复杂度，因此 $Feistel$ 轮函数的时间复杂度是 $O(1)$
4. 16 次 T 迭代，每次 T 迭代调用一次 Feistel 轮函数，因此 T 迭代的时间复杂度为 $O(1)$
5. 最后做一次 LR 交换和 IP 逆置换，时间复杂度都是 $O(1)$

因此 DES 算法的时间复杂度是 $O(1)$。

同理，由于算法对一个块的操作也仅需 $O(1)$ 的空间复杂度。

## 测试

由于 DES 算法是纯函数，输入密钥和明文和加密模式，可以输出密文；或者输入密钥和密文和解密模式，输出明文。因此我们可以借助单元测试的手段对 DES 算法的正确性进行验证。

### 单元测试

单元测试是通过构造样例对源程序中的函数行为进行验证的方法。

1. 验证 Feistel 轮函数的正确性。通过构造 32 位串 R 和 48 位子密钥 Ki，并通过人工计算 32 位结果，通过添加单元测试函数，验证代码运行结果的正确性，
2. 验证子密钥生成的正确性。通过构造任意满足条件的 64 位（56 位有效）的 DES 密钥，通过人工构造多组计算结果，并使用单元测试代码验证子密钥生成的正确性。

3. 验证 T 迭代的正确性，通过构造 64 位输入 M 和 48 位子密钥 Ki，人工计算结果并通过单元测试代码比对验证 T 迭代的正确性。
4. 验证 DES 算法除 S-盒 之外的正确性：除了对子模块进行测试，还需要对 DES 算法整体进行测试，通过随机生成密钥和明文块，对明文块进行加密和解密，验证解密得到的块是否和明文一致。

### 攻击测试

除了正确性验证，我们还需验证算法是否会被溢出攻击。比如计算过程中是否会被构造特定的输入数据导致程序计算结果溢出，从而被攻击。

## 代码

```c
uint64_t do_permutation(const permutation_t *perm, uint64_t chunk)
{
    uint64_t result = 0;
    for (int i = 0; i < perm->to_bits; ++i)
    {
        result = (result << 1) | get_bit(chunk, perm->from_bits - perm->table[i] - 1);
    }
    return result;
}

uint64_t do_sbox(const int box[4][16], uint64_t chunk)
{
    assert(0 <= chunk && chunk < (1 << 6));
    return box[((chunk >> 4) & 0x2) | (chunk & 0x1)][(chunk >> 1) & 0xF];
}

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
```

