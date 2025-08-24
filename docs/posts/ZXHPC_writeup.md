---
date: 
  created: 2025-08-22
  updated: 2025-08-25
categories:
  - Tech
authors:
  - ruri
hero: assets/posts/kotoha.webp
---
# ZXHPC 2025 Writeup
???+ TIP
    Kotoha かわすぎ😭

    都去听 Kotoha Cover 的 [#あくあ色ぱれっと](https://www.youtube.com/watch?v=UdXxY3kLdVc) 😡
    
<!-- more -->

因为是一边调整生物钟一边打比赛（虽然到最后也没成功把生物钟调整回来），凌晨上分应该很诡异吧www

最后拿到了 Rank 2 的成绩，其中 `md5-new` 和 `traffic-detector` 都拿到了 SOTA 的成绩，总体来说还是很满意的吧

![](../assets/posts/rank.webp)

## 环境信息

评测姬的CPU信息如下

```
Architecture:          x86_64
CPU op-mode(s):        32-bit, 64-bit
Byte Order:            Little Endian
CPU(s):                40
On-line CPU(s) list:   0-39
Thread(s) per core:    1
Core(s) per socket:    20
Socket(s):             2
NUMA node(s):          2
Vendor ID:             GenuineIntel
CPU family:            6
Model:                 85
Model name:            Intel(R) Xeon(R) Gold 6148 CPU @ 2.40GHz
Stepping:              4
CPU MHz:               2600.001
CPU max MHz:           2401.0000
CPU min MHz:           1000.0000
BogoMIPS:              4800.00
Virtualization:        VT-x
L1d cache:             32K
L1i cache:             32K
L2 cache:              1024K
L3 cache:              28160K
NUMA node0 CPU(s):     0-19
NUMA node1 CPU(s):     20-39
Flags:                 fpu vme de pse tsc msr pae mce cx8 apic sep mtrr pge mca cmov pat pse36 clflush dts acpi mmx fxsr sse sse2 ss ht tm pbe syscall nx pdpe1gb rdtscp lm constant_tsc art arch_perfmon pebs bts rep_good nopl xtopology nonstop_tsc cpuid aperfmperf pni pclmulqdq dtes64 ds_cpl vmx smx est tm2 ssse3 sdbg fma cx16 xtpr pdcm pcid dca sse4_1 sse4_2 x2apic movbe popcnt tsc_deadline_timer aes xsave avx f16c rdrand lahf_lm abm 3dnowprefetch cpuid_fault epb cat_l3 cdp_l3 invpcid_single pti intel_ppin ssbd mba ibrs ibpb stibp tpr_shadow vnmi flexpriority ept vpid ept_ad fsgsbase tsc_adjust bmi1 hle avx2 smep bmi2 erms invpcid rtm cqm mpx rdt_a avx512f avx512dq rdseed adx smap clflushopt clwb intel_pt avx512cd avx512bw avx512vl xsaveopt xsavec xgetbv1 xsaves cqm_llc cqm_occup_llc cqm_mbm_total cqm_mbm_local dtherm ida arat pln pts pku ospke md_clear flush_l1d arch_capabilities
```

编译器信息：

=== "`HPCI`"
    ```
    g++ (binhub) 10.1.0
    Copyright (C) 2020 Free Software Foundation, Inc.
    This is free software; see the source for copying conditions.  There is NO
    warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
    ```
=== "`Local`"
    ```
    g++ (Ubuntu 14.2.0-19ubuntu2) 14.2.0
    Copyright (C) 2024 Free Software Foundation, Inc.
    This is free software; see the source for copying conditions.  There is NO
    warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
    ```

`a+b`、`cos-sim`、`md5-bf`、`md5-new` 均使用集群版本为 10.1.0 的 g++ 编译器

`traffic-detector` 为本机编译静态链接，上传二进制文件

## cos-sim

这个题目的核心任务是，给我们 `N` 个 `D` 维的向量，要我们算出每个向量和其他所有向量的余弦相似度，然后找出最大的那四个。

最直观、最暴力的想法，就是老老实实地写两层循环：外层循环遍历向量 `i`，内层循环遍历向量 `j`，然后在最里面根据公式计算 `i` 和 `j` 的相似度。这个算法的复杂度大概是 `O(N² * D)`。考虑到 `N` 最大能到 20000，`D` 是 4096，这个计算量是天文数字，肯定会超时。

#### 优化思路

我们的优化主要分三步走：首先在数学上“偷个懒”，然后选择了两条不同的计算路径，并最终根据实际测试，将它们组合成了一个混合策略

**向量归一化**

我们先看一下余弦相似度的公式：


$$
\text{similarity}(A, B) = \frac{A \cdot B}{\|A\| \cdot \|B\|}
$$

分母部分的 $||A||$ 和 $||B||$ 是向量的模长，计算它需要开平方，这是一个非常耗时的操作。如果在 `N*N` 的循环里反复计算，性能损失会非常大

我们可以预先将所有向量都处理成单位向量，即归一化

归一化之后，所有向量的模长 $||V||$ 都变成了 1。这时公式就变成了：

$$
\text{similarity}(A', B') = \frac{A' \cdot B'}{1 \cdot 1} = A' \cdot B'
$$

计算余弦相似度被简化成了只计算点积。

这一步的开销是 `O(N * D)`，相比于后续 `N²` 级别的计算，这点预处理的开销完全可以忽略不计，但它带来的收益是巨大的

之后的问题就是如何高效计算 N*N 的点积矩阵，我首先想到了这个方式：

把 `N` 个 `D` 维向量看成一个 `N x D` 的矩阵 `M`。那么，`M` 乘以它自己的转置 `M^T`，得到的结果 `S = M * M^T` 是一个 `N x N` 的矩阵。这个结果矩阵 `S` 里面，`S_ij` 的值正好就是向量 `i` 和向量 `j` 的点积

这个操作可以通过 BLAS 库里的 `cblas_sgemm` 函数（通用矩阵乘法）一步完成

然后就能开香槟...不对，怎么小矩阵算得这么慢呢？暴力逐点计算在小规模反而更快！？

那就根据 N 的大小判断使用暴力计算还是矩阵乘法！

写了一个 `benchmark.cpp` 程序在不同的 `N` 值下对这两个方案进行了性能测试

??? note "benchmark.cpp"
    ```c++
    --8<-- "docs/code/benchmark.cpp"
    ```

结果：
```
Benchmarking brute_force vs gemm...
N=256 brute=34 ms  gemm=66 ms  -> faster: brute
N=512 brute=21 ms  gemm=55 ms  -> faster: brute
N=1024 brute=23 ms  gemm=22 ms  -> faster: gemm
N=1500 brute=48 ms  gemm=25 ms  -> faster: gemm
N=2000 brute=165 ms  gemm=49 ms  -> faster: gemm
N=3000 brute=556 ms  gemm=98 ms  -> faster: gemm
N=4000 brute=1020 ms  gemm=73 ms  -> faster: gemm
```

- 当 `N` 比较小的时候（比如几百），`cblas_sdot` 反而更快。因为 `sgemm` 虽然理论效率高，但它本身有一些固定的启动开销，而且内存分配也需要时间。数据量小的时候，这些额外开销就显得比较突出了
- 当 `N` 增大到某个临界点之后，`cblas_sgemm` 的优势就体现出来了，它的计算效率增长得更快

通过我们的测试，这个临界点大约在 `N = 1024` 左右

因此最终使用混合策略
- 如果 `N < 1024`，我们就用逐点计算的方案
- 如果 `N >= 1024`，我们就用矩阵乘法的方案

这样，无论输入数据的规模如何，程序都能自动选择当前最优的算法路径

最终得分 99.0 pts

~~1pts差在N=1000时，懒得凹分~~

## 	md5-bf

??? note "md5-bf"
    **MD5 Brute Force**

    **题目内容**

    MD5 是一种广泛使用的哈希函数，于1992年公开。它将任意长度的输入映射为128位的哈希值。尽管 MD5在密码学上已被证明不够安全，但它仍然在数据完整性校验等领域得到广泛应用。

    在本题中，你需要实现一个程序，加速MD5哈希计算。你的任务是找到一个给定的目标哈希值对应的原始输入。

    具体来说，你将使用一个简单的伪随机数生成器（PRNG）来生成一系列可能的输入，并计算它们的MD5哈希值。你的目标是找到第一个生成的输入，其MD5哈希值与给定的目标哈希值匹配，并输出该输入在生成序列中的位置。

    生成器由 3 个异或移位寄存器（Xorshift）组成，初始种子由 3 个 64 位无符号整数表示。每次调用生成器时，它会更新寄存器的状态并得到3个64位无符号整数的伪随机数，然后在每个整数之后插入 8 个 0 字节，形成一个 48 字节的输入。

    生成器的C++实现如下：

    ```cpp
    class RndGen
    {
    public:
        RndGen(uint64_t s0, uint64_t s1, uint64_t s2)
        {
            s_[0] = s0;
            s_[1] = s1;
            s_[2] = s2;
        }

        RndGen() = delete;

        void generate(uint64_t out[6])
        {
            out[0] = xorshift64(s_[0]);
            out[1] = 0;
            out[2] = xorshift64(s_[1]);
            out[3] = 0;
            out[4] = xorshift64(s_[2]);
            out[5] = 0;
        }

    private:
        uint64_t xorshift64(uint64_t &state)
        {
            uint64_t x = state;
            x ^= (x << 13);
            x ^= (x >> 7);
            x ^= (x << 17);
            state = x;
            return x;
        }
        uint64_t s_[4];
    };
    ```

    **输入格式**

    每组测试数据包含5个测试点，每个测试点2行，共10行。

    每个测试点的第一行包含 3 个以空格分隔的 16 进制字符串，表示生成器的初始种子。
    每个测试点第二行包含一个 16 进制字符串，表示目标哈希值。

    **输出格式**

    输出5行，每行输出一个整数 n，表示该测试点的目标哈希值是由生成器生成的第 n 个原始输入的 MD5 哈希值。

    **样例**

    **输入样例**

    ```text
    f266a54e3e47a23c fdaae989f0a7211c e6aa6f98e36deedf
    82d45fa866fa60b64d71164df84c75c9
    17a7cd855b73cdac cc4323d166e03be9 b00bc0b3bd5894fa
    7218bba8b911ab47c6327bb5dbe61b59
    dea8b42aaeadda81 ec4cc7cb402de9d9 ce3826dac217a356
    1a74d9f4a21cbe42d7eb6476505f8ca9
    ca1114b1a8399b94 4f22b32081209bca 3db2ed9884246d92
    38978e0a05d932b7c6abb0969ddf9d42
    8af2347938b775bf d03e9af34bb95deb 3542f798e87dc7b4
    edceface43f62178ada45fd121932c13
    ```

    **输出样例**

    ```text
    1
    12
    123
    1234
    12345
    ```

    **评分标准**

    10 个测试点，每个测试点的得分具体如下：

    - 输出结果必须正确。结尾的空格和换行将被忽略。结果不正确的，该测试点得0分。
    - 计该测试点的运行时间为 $t$ 秒、正确答案为 $n$，则该测试点得分为 $\dfrac{60}{t} - 1$，其中 $t$ 的最大值为 60 秒。如果运行时间超过 60 秒，则该测试点得 0 分。

    总分为所有测试点得分之和，并根据全部选手的最高分进行归一化，线性缩放到满分 100 分。

看到题目第一时间就想到先用 [SIMD](https://en.wikipedia.org/wiki/Single_instruction,_multiple_data) 加速，因为不管是给出的随机数生成器，还是md5值的生成，都是很容易向量化的算法

通过 `lscpu` 查看 CPU flags，集群支持我们使用 AVX2 和 AVX512 指令集进行 SIMD 优化

Github 搜索 `simd` 和 `md5`，第一个出现的就是 [minio/md5-simd](https://github.com/minio/md5-simd)

于是出于偷懒考虑(bushi)，使用了 Go 实现的第一版实验算法

当然实验是大☆失☆败了，实验下来消耗时间远高题目要求最高时间，我是Golang苦手不会优化真是太对不起了，因此切换到了传统的 C++ 方案

使用了 [ashbob999/md5-simd](https://github.com/ashbob999/md5-simd) 这个库只实现了最高8路的向量化 MD5 计算，但为了简化MD5算法实现，可以之后再优化到 AVX512 16路计算

题目在比赛期间更新了，标为`md5-bf`的代码为旧题目实现，标为`md5-new`的则为新版

??? note "参考代码 Ver1.0 ( md5-bf test1用时约21.703960709s )"
    ```c++
    --8<-- "docs/code/md5-bf.cpp"
    ```


Ver1.0是最基本的实现，分析一下可以发现有问题的部分有

1. MD5 没有用到 SIMD 加速，使用SIMD可以获得最高16x的加速
2. 每个线程都有自己的生成器实例，每个线程都会从第一个随机数开始一直生成到属于自己的md5计算部分，重复计算
3. 随机数生成器没有用到 SIMD 加速

经profiling，`RndGen`占用的时间和8路`SIMD`计算时间大致相当，`RndGen`不是可忽略的开销

`RndGen`实际上是Xorshift随机数生成器的变种，而`Xorshift`在`GF(2)`上是线性的

因此可以将这个随机数生成过程用二元域`GF(2)`上的矩阵乘法表示（`XOR`相当于乘法，`AND`相当于加法）

### RndGen的数学表示

- 状态空间：

$$
\mathcal{S} = (\mathrm{GF}(2)^{64})^3
$$

- 状态转移：

$$
(s_0,s_1,s_2) \;\mapsto\; (A s_0,\, A s_1,\, A s_2),
$$

其中:

$$
A=(I \oplus L_{17})(I \oplus R_{7})(I \oplus L_{13})
$$

- 输出函数：

$$
\text{Out}(s_0,s_1,s_2) = (A s_0,\,0,\,A s_1,\,0,\,A s_2,\,0).
$$


这个性质可以被利用于线程快速跳过属于其他线程的`MD5`计算任务，只需预先计算好

$$
T = A^{\text{BATCH}  \times  \text{thread\_num}}
$$

将$T$与当前状态相乘，即可在`O(1)`内完成跳到属于当前线程的`BATCH`

这个算法名字叫`Xorshift Jump`

??? note "RndGen"
    ```c++
    --8<-- "docs/code/rndgen.cpp"
    ```

其中`PRECOMPUTED_JUMPS`是40个已经在编译前计算好的用于跳转到对应起始位置的掩码

```chat
[left] [https://avatars.githubusercontent.com/u/191038400] 流离流离，你这个随机数生成器虽然好，但是不是有点浪费了？
[right] [https://avatars.githubusercontent.com/u/88608708] 确实确实，向量寄存器都还没满呢，而且还需要转置才能传递给 MD5 计算实例
```

我们同样可以使用 `Xorshift Jump` 对SIMD操作进行并行优化，AVX512 的向量寄存器，最多能存储8个 `u64` 的状态，而使用 AVX512 向量的 MD5 实例一次计算需要 $16 \times 3$ 个状态，因此我们通过 6 个向量在一个 `RndGen` 对象里储存 16 组状态，一组状态由三个 `u64 `状态共 24bytes 组成

如果我们之前是在同一个向量寄存器里储存同一组状态，那这个优化就相当于把同一组状态转置分散在三个不同向量寄存器里，刚好 MD5 实例计算所需要的就是这样转置后的 `SoA` 排列的状态，我们相当于同时做了两组优化：simd 加速随机数计算，同时节省了转置的操作开销

总结一下我们的并行策略:

- 我们设定一个计算最小单元 `BLOCK` 为 8192 次迭代
- 首先通过 40 个线程在线程级别并行，每个线程之间状态相差 $8192 \times 16$ 组状态，即 16 个 BLOCK
- 线程内部通过 SIMD 并行，同时计算 16 组状态，每组状态之间相差一个 BLOCK
- 当线程迭代 8192 次后，本批次任务就完成了，此时使用 Xorshift 跳过其他线程负责的 BLOCK，来到下一个自己负责的批次

多线程加上 8 路 SIMD 并行，理论上能比逐个计算快 `8 × 线程数` 倍

经过火焰图查看，我们的 MD5 计算占比从 50% 提升到了 90%，可喜可贺可喜可贺

??? note "最终优化代码 (270.2pts)"
    ```c++
    --8<-- "docs/code/md5-new.cpp"
    ```
