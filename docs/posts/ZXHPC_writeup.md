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
    Kotoha かわすぎ😭[^1]

    都去听 Kotoha Cover 的 [#あくあ色ぱれっと](https://www.youtube.com/watch?v=UdXxY3kLdVc)
    

因为是一边调整生物钟一边打比赛（虽然到最后也没成功把生物钟调整回来），凌晨上分应该很诡异吧www

最后拿到了 Rank 2 的成绩，其中 `md5-new` 和 `traffic-detector` 都拿到了 SOTA 的成绩，总体来说还是很满意的吧

<!-- more -->

![](../assets/posts/rank.webp)

本次比赛我使用的代码均可在[https://git.byr.moe/LiuliFox/zxhpc-writeup](https://git.byr.moe/LiuliFox/zxhpc-writeup)查看

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

## a-plus-b

??? quote "题面 a-plus-b"
    **A+B Problem**

    **题目内容**

    为了帮助选手熟悉比赛环境和评分器，我们准备了一道简单的题目。通过这道题，你可以熟悉 slurm 环境和评分、提交流程。

    给定两个整数 A 和 B，请计算它们的和。

    **输入格式**

    输入包含一行，包含两个整数 A 和 B ($$1 \le A, B \le 10^9$$)。

    **输出格式**

    输出一个整数，表示 A 和 B 的和。

    **样例**

    **输入样例**
    ```text
    3 5
    ```
    **输出样例**
    ```text
    8
    ```
    **评分标准**
    两个测试点，每个 5 分，共 10 分。

    **其他**

    关于评分器的使用方法，见[评分器文档](/pages/scorer)。

签到一下

```c++
#include <iostream>
using namespace std;

int main() {
    int a, b;
    cin >> a >> b;
    cout << a + b;
    return 0;
}
```

## cos-sim

??? quote "题面 cos-sim"
    **Cosine Similarity**

    **注意：本题只能在 Linux x86 系统上运行**

    **题目内容**

    余弦相似度是衡量两个向量在高维空间中方向相似性的指标，它通过计算两个向量夹角的余弦值来判断它们的相似程度，数值范围在 $[-1, 1]$ 之间。其定义为：

    $$
    \text{cosine\_similarity}(A, B) = \frac{A \cdot B}{\|A\| \|B\|} = \frac{\sum_{i=1}^{D} A_i B_i}{\sqrt{\sum_{i=1}^{D} A_i^2} \sqrt{\sum_{i=1}^{D} B_i^2}}
    $$

    现在给定 $N$ 个 $D$ 维向量，求这些向量两两之间的余弦相似度。

    为了减少输出数据量，你只需要输出每个向量除其自身以外余弦相似度最大的四个向量对应的值。

    你的答案被视为正确，当且仅当每个输出数值与标准结果的相对或绝对误差小于 $10^{-6}$。

    **输入格式**

    通过标准输入输入小端序二进制串，可参考示例代码。

    首先是两个 `int32`，分别表示 $N$ 和 $D$。

    接下来是 $N \times D$ 个 `float32`，表示 $N$ 个 $D$ 维向量。

    **输出格式**

    通过标准输出输出小端序二进制串，可参考示例代码。

    输出 $4N$ 个 `float32`，表示每个向量除其自身以外余弦相似度最大的四个向量对应的值。

    **评分标准**

    共四个测试点。

    | 测试点 | 分值 | $N$ | $D$ | $T_\text{std}$ | $T_\text{max}$ |
    |--------|------|-----|-----|---------|---------|
    | 1      | 25   | 1000 | 4096  | 0.1s      | 10s      |
    | 2      | 25   | 5000 | 4096  | 0.25s     | 120s      |
    | 3      | 25   | 10000| 4096  | 0.6s      | 120s      |
    | 4      | 25   | 20000| 4096  | 1.8s      | 120s      |

    若一个测试点运行时间为 $T$，则该测试点分值为

    $$
    \text{score} = \begin{cases}
    25 &, T \leq T_\text{std} \\
    25\left({\log\left(\frac{T}{T_\text{max}}\right)}/{\log\left(\frac{T_\text{std}}{T_\text{max}}\right)}\right) &, T_\text{std} < T < T_\text{max} \\
    0 &, T \geq T_\text{max}
    \end{cases}
    $$

    **示例程序**

    ```cpp
    #include <algorithm>
    #include <cmath>
    #include <iostream>
    #include <vector>
    #include <cstdint>

    float cosine_similarity(const float *a, const float *b, int D) {
        float dot_product = 0.0f;
        float sum_a2 = 0.0f;
        float sum_b2 = 0.0f;
        for (int i = 0; i < D; ++i) {
            dot_product += a[i] * b[i];
            sum_a2 += a[i] * a[i];
            sum_b2 += b[i] * b[i];
        }
        return dot_product / (std::sqrt(sum_a2) * std::sqrt(sum_b2) + 1e-12);
    }

    int main() {
        uint32_t N, D;
        std::cin.read(reinterpret_cast<char *>(&N), sizeof(N));
        std::cin.read(reinterpret_cast<char *>(&D), sizeof(D));
        std::vector<float> data(N * D);
        std::cin.read(reinterpret_cast<char *>(data.data()), N * D * sizeof(float));

        for (int i = 0; i < N; ++i) {
            std::vector<float> cosine_sim(N);
            for (int j = 0; j < N; ++j) {
                cosine_sim[j] = cosine_similarity(data.data() + i * D, data.data() + j * D, D);
            }
            std::partial_sort(cosine_sim.begin(), cosine_sim.begin() + 5, cosine_sim.begin() + N, std::greater<float>());
            std::cout.write(reinterpret_cast<char *>(cosine_sim.data() + 1), 4 * sizeof(float));
        }

        return 0;
    }
    ```

    该代码仅支持小端序设备。

这个题目的核心任务是，给我们 `N` 个 `D` 维的向量，要我们算出每个向量和其他所有向量的余弦相似度，然后找出最大的那四个。

最直观、最暴力的想法，就是老老实实地写两层循环：外层循环遍历向量 `i`，内层循环遍历向量 `j`，然后在最里面根据公式计算 `i` 和 `j` 的相似度。这个算法的复杂度大概是 `O(N² * D)`。考虑到 `N` 最大能到 20000，`D` 是 4096，这个计算量是天文数字，肯定会超时。

### 优化思路

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

### 小规模优化

然后就能开香槟...不对，怎么小矩阵算得这么慢呢？暴力逐点计算在小规模反而更快！？

那就根据 N 的大小判断使用暴力计算还是矩阵乘法！

写了一个 `benchmark.cpp` 程序在不同的 `N` 值下对这两个方案进行了性能测试

??? example "benchmark.cpp"
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

??? success "最终优化代码 (99.0pts)"
    ```c++
    --8<-- "docs/code/cos-sim.cpp"
    ```

## 	md5-bf

??? quote "md5-bf"
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

??? warning "参考代码 Ver1.0 ( md5-bf test1用时约21.703960709s )"
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

??? example "RndGen"
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

??? success "最终优化代码 (270.2pts)"
    ```c++
    --8<-- "docs/code/md5-new.cpp"
    ```

## llm-challenge

??? quote "题面 llm-challenge"
    **LLM挑战**

    **注意：本题只能在 Linux x86 系统上运行**

    **题目描述**

    请选择合适的 LLM 模型及推理系统，完成给定的100道测试题。你需要兼顾运行时间与正确率，以获得尽可能高的分数。

    你可以使用除了打表和调用外部LLM以外的各种方法优化运行时间与测试得分，例如：

    - 使用高效推理框架
    - 使用量化、稀疏化模型
    - 在问题基础上添加其他prompt
    - 使用思维链
    - 调整从模型回答中提取答案的方式
    - 只选择部分问题作答
    - 提前完成模型加载，仅在`zxscorer`中处理输入

    **输入格式**

    输入为一系列四选一的单选题，各题目之间以连续两个换行符分隔（保证题目内不存在连续两个换行符）。

    实际输入中每个题目的平均字符数为473。

    **输出格式**

    输出需按照题目输入的顺序，每行填写一个字符（A/B/C/D），以此表示对应题目的答案。

    **输入样例**

    示例仅包含5道题目，实际输入包含100道题目。

    ```text
    A person walked 3 miles to the east, then turned north and walked 10 miles, then turned west and walked 6 miles, and finally turned south and walked 16 miles. Approximately how far is the person from his starting point in miles?
    A. 3.4
    B. 6.7
    C. 9.2
    D. 12.8

    This question refers to the following information.
    "We conclude that, in the field of public education, the doctrine of "separate but equal" has no place. Separate educational facilities are inherently unequal. Therefore, we hold that the plaintiffs and others similarly situated for whom the actions have been brought are, by reason of the segregation complained of, deprived of the equal protection of the laws guaranteed by the Fourteenth Amendment."
    Brown v. Board of Education, 1954
    Desegregation of schools was, in part, a response to unfulfilled promises from which of the following initiatives?
    A. The Great Society
    B. The Square Deal
    C. The New Deal
    D. Reconstruction

    Statement 1| For any two variables x and y having joint distribution p(x, y), we always have H[x, y] ≥ H[x] + H[y] where H is entropy function. Statement 2| For some directed graphs, moralization decreases the number of edges present in the graph.
    A. True, True
    B. False, False
    C. True, False
    D. False, True

    This question refers to the following information.
    "To slacken the tempo would mean falling behind. And those who fall behind get beaten. But we do not want to be beaten. No, we refuse to be beaten! One feature of the history of old Russia was the continual beatings she suffered because of her backwardness. She was beaten by the Mongol khans. She was beaten by the Turkish beys. She was beaten by the Swedish feudal lords. She was beaten by the Polish and Lithuanian gentry. She was beaten by the British and French capitalists. She was beaten by the Japanese barons. All beat her––because of her backwardness, because of her military backwardness, cultural backwardness, political backwardness, industrial backwardness, agricultural backwardness. They beat her because it was profitable and could be done with impunity. You remember the words of the pre-revolutionary poet: "You are poor and abundant, mighty and impotent, Mother Russia." Those gentlemen were quite familiar with the verses of the old poet. They beat her, saying: "You are abundant," so one can enrich oneself at your expense. They beat her, saying: "You are poor and impotent," so you can be beaten and plundered with impunity. Such is the law of the exploiters––to beat the backward and the weak. It is the jungle law of capitalism. You are backward, you are weak––therefore you are wrong; hence you can be beaten and enslaved. You are mighty––therefore you are right; hence we must be wary of you.
    That is why we must no longer lag behind."
    Joseph Stalin, speech delivered at the first All-Union Conference of Leading Personnel of Socialist Industry, February 4, 1931
    Stalin's speech is most strongly influenced by which of the following?
    A. Appeasement
    B. Fascism
    C. Communism
    D. Secret treaties

    Which of the following forms of dementia has the highest prevalence rate?
    A. Alzheimer's
    B. Vascular
    C. Parkinson's
    D. Fronto-temporal lobe
    ```

    **输出样例**

    ```text
    B
    D
    B
    C
    A
    ```

    **评分标准**

    若运行时间为$T$分钟，则运行时间部分的得分为

    $$
    S_1 = \begin{cases}
    100 &, T \leq 0.5 \\
    100\left({\log\left(\frac{T}{30}\right)}/{\log\left(\frac{0.5}{30}\right)}\right) &, 0.5 < T < 30 \\
    0 &, T \geq 30
    \end{cases}
    $$

    若答案正确的问题数量为$C$，则正确率部分的得分为

    $$
    S_2 = \frac{100}{65}\max(C - 35, 0)
    $$

    最终得分为两部分得分的几何平均值，即$S=\sqrt{S_1S_2}$，并根据全部选手的最高分进行归一化，线性缩放到满分 100 分。

这一题大部分的难度其实是在配环境上，但我选择了一个取巧的方式：使用 [llamafile](https://github.com/Mozilla-Ocho/llamafile)

???+ quote "README.md"
    **llamafile lets you distribute and run LLMs with a single file. ([announcement blog post](https://hacks.mozilla.org/2023/11/introducing-llamafile/))**

    Our goal is to make open LLMs much more
    accessible to both developers and end users. We're doing that by
    combining [llama.cpp](https://github.com/ggerganov/llama.cpp) with [Cosmopolitan Libc](https://github.com/jart/cosmopolitan) into one
    framework that collapses all the complexity of LLMs down to
    a single-file executable (called a "llamafile") that runs
    locally on most computers, with no installation.

一键运行，在服务器上可以无须配置任何环境直接运行

对于模型，我尝试了这几个模型

- [https://huggingface.co/Mozilla/Qwen2.5-7B-Instruct-1M-llamafile](https://huggingface.co/Mozilla/Qwen2.5-7B-Instruct-1M-llamafile)

- [https://huggingface.co/Mozilla/Qwen3-4B-llamafile](https://huggingface.co/Mozilla/Qwen3-4B-llamafile)

- [https://huggingface.co/Mozilla/Qwen2.5-0.5B-Instruct-llamafile](https://huggingface.co/Mozilla/Qwen2.5-0.5B-Instruct-llamafile)

- [https://huggingface.co/Mozilla/Llama-3.2-1B-Instruct-llamafile](https://huggingface.co/Mozilla/Llama-3.2-1B-Instruct-llamafile)

- [https://huggingface.co/zai-org/glm-4-9b](https://huggingface.co/zai-org/glm-4-9b)

其中 glm-4-9b 的正确率最高，一般能到 80% 左右，但速度比 Qwen2.5-7B-Instruct-1M-llamafile 慢了非常多

Qwen3 系列因为思维链的关系，输出不易调整，综合考虑后排除了 Qwen3 系列模型

综合考虑，Qwen2.5-7B-Instruct-1M-llamafile 是得分最高的模型，取得了最高的成绩（58.16pts）

我考虑过限制 max token 为 1 并用 grammar 限制输出为 A/B/C/D，但正确率太低得不偿失，故没有继续深入调试

```
Run log:
Test case: Test 1
Duration: 158.023s
Diff result: 72
Tip: 
Score: 58.15746228385942
Passed
```

??? success "最终优化代码 (58.16pts)"
    ```c++
    --8<-- "docs/code/llm.py"
    ```

## traffic-detector

??? quote "题面 traffic-detector"
    **Traffic Detector**

    **注意：本题无法在本地测试，只能在集群环境测试或提交**

    **题目描述**

    恶意流量检测是网络空间安全中的重要研究课题，如何分析大量的网络数据是其中的一个重难点。现有纯文本格式的网络流量，包含有两种协议的流量数据：TCP/DNS。具体的格式如下：

    - TCP流量：`时间戳 TCP 源IP 目的IP 源端口 目的端口 flags 数据长度 十六进制数据`
    - DNS流量：`时间戳 DNS 源IP 目的IP 源端口 目的端口 域名长度 域名`

    输入数据按照时间戳增序给出。

    在正常的流量中，混杂有两种恶意流量：TCP端口扫描和DNS隧道。在这里我们使用最简单的模型，将所有“源IP只发送一个SYN包便没有后续流量”的流视为端口扫描，将所有域名前缀大于等于30的DNS查询视为DNS隧道。请你统计所有的恶意行为，并给出统计数据

    请不要根据输入数据的其他无关特征进行优化，如DNS隧道对应的主域名与正常DNS请求的主域名差异等。请严格按照以上的规则进行统计，并完成文件IO、算法相关的优化。

    **输出格式**

    `IP 恶意行为 发生次数`，使用一个空格作为分隔，其中恶意行为是`portscan`/`tunnelling`中的一个，发生次数对于端口扫描而言是所有端口扫描TCP流的个数，对于DNS隧道而言是所有通过DNS隧道传输的数据总量（即子域名前缀的长度，按字符计）。题目对输出数据的顺序有要求，需要先输出所有`portscan`统计，再输出所有`tunnelling`统计，同时每份统计按照字典序升序对IP进行排序（如`1.1.1.7` < `1.1.1.70` < `1.1.1.8`）。

    **样例**

    **输入样例**
    ```text
    0.01 TCP 114.5.1.4 192.168.1.100 1919 810 SYN 0
    0.02 DNS 192.168.1.1 8.8.8.8 61212 53 42 aaaaaaaaaaaaaaaaaaaaaaaaaaaaaa.example.com
    ```
    **输出样例**
    ```text
    114.5.1.4 portscan 1
    192.168.1.1 tunnelling 30
    ```

    **评分规则**

    $$
    \text{score} = 25\cdot\log_2\left(\frac{300\text{s}}{T}\right)
    $$

    **参考程序**

    ```C++
    #include <iostream>
    #include <fstream>
    #include <sstream>
    #include <map>
    #include <vector>
    #include <string>

    struct Packet {
        double timestamp;
        std::string protocol;
        std::string src_ip, dst_ip;
        int src_port = -1, dst_port = -1;
        std::string flags;
        int data_len = 0;
        std::string data;
    };

    Packet parse_line(const std::string& line) {
        Packet pkt;
        std::istringstream iss(line);
        iss >> pkt.timestamp >> pkt.protocol >> pkt.src_ip >> pkt.dst_ip;
        if (pkt.protocol == "TCP" || pkt.protocol == "DNS") {
            iss >> pkt.src_port >> pkt.dst_port;
            if (pkt.protocol == "TCP") {
                iss >> pkt.flags;
            }
            iss >> pkt.data_len;
            if (iss.peek() == ' ' || iss.peek() == '\t') iss.get();
            std::getline(iss, pkt.data);
            if (!pkt.data.empty() && pkt.data[0] == ' ') pkt.data.erase(0, 1);
        }
        return pkt;
    }

    std::string get_dns_prefix(const std::string& domain) {
        size_t dot = domain.find('.');
        if (dot != std::string::npos) return domain.substr(0, dot);
        return "";
    }

    int main() {
        std::ifstream fin("network_traffic.txt");
        if (!fin) {
            std::cerr << "读取network_traffic.txt失败!\n";
            return 1;
        }

        struct FiveTuple {
            std::string src_ip, dst_ip;
            int src_port, dst_port;
        };
        auto tuple_str = [](const FiveTuple& t) {
            return t.src_ip + "|" + t.dst_ip + "|" + std::to_string(t.src_port) + "|" + std::to_string(t.dst_port);
        };

        // 端口扫描
        std::map<std::string, std::vector<Packet>> syn_flows;

        // DNS隧道
        std::map<std::string, int> dnstunnel_count; // src_ip -> 次数

        std::string line;
        while (std::getline(fin, line)) {
            if (line.empty()) continue;
            Packet pkt = parse_line(line);

            // 端口扫描
            if (pkt.protocol == "TCP") {
                FiveTuple key{pkt.src_ip, pkt.dst_ip, pkt.src_port, pkt.dst_port};
                syn_flows[tuple_str(key)].push_back(pkt);
            }
            // DNS隧道
            else if (pkt.protocol == "DNS" && !pkt.data.empty()) {
                std::string prefix = get_dns_prefix(pkt.data);
                if (prefix.length() >= 30) {
                    dnstunnel_count[pkt.src_ip]+=prefix.length();
                }
            }
        }

        // 输出端口扫描
        std::map<std::string, int> portscan_ip_count;
        for (const auto& kv : syn_flows) {
            const std::vector<Packet>& pkts = kv.second;
            if (pkts.size() == 1 && pkts[0].flags == "SYN") {
                portscan_ip_count[pkts[0].src_ip]++;
            }
        }

        // 统一输出，优先端口扫描，后隧道
        for (const auto& kv : portscan_ip_count) {
            std::cout << kv.first << " portscan " << kv.second << std::endl;
        }
        for (const auto& kv : dnstunnel_count) {
            std::cout << kv.first << " tunnelling " << kv.second << std::endl;
        }

        return 0;
    }
    ```

这个程序的行为非常复杂，我们先看看 profile 结果

本次比赛我主要使用 [samply](https://github.com/mstange/samply) 作为 profile 工具，这个工具能采样程序运行数据，用 `profiler.firefox.com` 作为前端画出直观的火焰图与显示热点代码

![](../assets/posts/profile.webp)

可以发现大部分的时间都是花在 `std::map` 的插入与查找操作上，`std::map` 的内部实现是红黑树，对于大量插入查找的行为，hashmap 才是更合适的数据结构

### traffic-detector Ver1.0

查看这个 [Hashmaps Benchmarks](https://martin.ankerl.com/2019/04/01/hashmap-benchmarks-05-conclusion/) ，考虑到静态链接到程序中的难度，我选择了 `absl::flat_hash_map`

为了优化 hashmap 速度，我将原来由多个字符串拼接成的 `string key (src_ip + "|" + ...)` 替换为一个自定义的 FiveTuple 结构体，减少了字符串拼接操作

分析题目，其实我们不需要完整解析和储存每一行，只需要在协议是 "TCP" 或 "DNS" 才去解析后续的字段（如端口、Flags、域名等）。对于不关心的协议，解析工作在协议名之后就停止了

通过这些优化，我们的程序从需要 `233.565s` 加速到了只需要 `86.318s`，加速约 2.7倍

??? success "traffic-detector Ver1.0"
    ```c++
    --8<-- "docs/code/td1.cpp"
    ```

### traffic-detector Ver2.0

再次查看火焰图

![](../assets/posts/profile2.webp)

我们发现 `std::getline` 和解析占用了大量时间，因此我们可以考虑优化一下 IO 性能

我们可以使用更快的 `mmap` 将文件直接映射到进程的虚拟地址空间，代替传统的 std::ifstream 和 std::getline，此时操作系统会负责将文件内容按需载入内存

我们不使用 `std::string` 解析字符串，而是改成 `const char*` 指针进行手动解析，零内存拷贝

我们还发现，虽然题目要求将 ip 按照字符串顺序排序，可是我们完全可以自己实现一个“伪字符串排序”，将 ip 用 u32 格式储存，用这个自定义排序函数让 ip 按照字符串排序，这样可以极大加快 hashmap 的速度

??? example "ip_str_less"
    ```cpp hl_lines="2 3 7 11 15 17 21 25 27 31 33 37 39 43 45 64 65 68 69 72 73"
    inline int compare_num_str(int x, int y) {
        if (x < 10) {
            if (y < 10) {
                return x - y;
            } else if (y < 100) {
                int cmp = x - (y / 10);
                if (cmp != 0) return cmp;
                return -1;
            } else {
                int cmp = x - (y / 100);
                if (cmp != 0) return cmp;
                return -1;
            }
        } else if (x < 100) {
            if (y < 10) {
                int cmp = (x / 10) - y;
                if (cmp != 0) return cmp;
                return 1;
            } else if (y < 100) {
                int cmp = (x / 10) - (y / 10);
                if (cmp != 0) return cmp;
                return (x % 10) - (y % 10);
            } else {
                int cmp = (x / 10) - (y / 100);
                if (cmp != 0) return cmp;
                cmp = (x % 10) - (y / 10 % 10);
                if (cmp != 0) return cmp;
                return -1;
            }
        } else {
            if (y < 10) {
                int cmp = (x / 100) - y;
                if (cmp != 0) return cmp;
                return 1;
            } else if (y < 100) {
                int cmp = (x / 100) - (y / 10);
                if (cmp != 0) return cmp;
                cmp = (x / 10 % 10) - (y % 10);
                if (cmp != 0) return cmp;
                return 1;
            } else {
                int cmp = (x / 100) - (y / 100);
                if (cmp != 0) return cmp;
                cmp = (x / 10 % 10) - (y / 10 % 10);
                if (cmp != 0) return cmp;
                return (x % 10) - (y % 10);
            }
        }
    }

    // 比较两个IP的字典序（考虑点号）
    inline bool ip_str_less(uint32_t ip1, uint32_t ip2) {
        int a1 = (ip1 >> 24) & 0xFF;
        int b1 = (ip1 >> 16) & 0xFF;
        int c1 = (ip1 >> 8) & 0xFF;
        int d1 = ip1 & 0xFF;

        int a2 = (ip2 >> 24) & 0xFF;
        int b2 = (ip2 >> 16) & 0xFF;
        int c2 = (ip2 >> 8) & 0xFF;
        int d2 = ip2 & 0xFF;

        int cmp = compare_num_str(a1, a2);
        if (cmp < 0) return true;
        if (cmp > 0) return false;

        cmp = compare_num_str(b1, b2);
        if (cmp < 0) return true;
        if (cmp > 0) return false;

        cmp = compare_num_str(c1, c2);
        if (cmp < 0) return true;
        if (cmp > 0) return false;

        cmp = compare_num_str(d1, d2);
        return cmp < 0;
    }
    ```

除此之外，我们实际上不需要完整储存一个 TCP 流的全部 flags，只需要用一个 int 来标记状态：1 代表只看到了 SYN 包（潜在的 portscan），-1 代表看到了后续包（正常通信或已结束）

但是到目前为止，我们的程序还是一个单线程程序，并没有发挥集群的多核优势

于是我们又做了以下优化

- 代码不再逐行读取文件，而是将整个文件内容（通过 mmap 映射到内存）分割成多个块（chunks）。每个线程负责处理一个独立的块

- 每个线程拥有自己的哈希表 (thread_syn_flows, thread_dnstunnel) 来存储中间结果，避免了线程间因争抢锁而造成的性能瓶颈，直到所有线程处理完毕后才进行一次性的结果合并并排序结果

通过这些优化，我们的程序从需要 `86.318s` 加速到了只需需要 `9.473s`，加速约 9.1 倍

??? success "traffic-detector Ver2.0"
    ```c++
    --8<-- "docs/code/td2.cpp"
    ```

### traffic-detector Ver3.0

```chat
[left] [https://avatars.githubusercontent.com/u/191038400] 流离流离，你这个 ip_str_less 虽然好，但是不是分支有点太多了？
[right] [https://avatars.githubusercontent.com/u/88608708] 确实确实，这么多分支对现代 CPU 的分支预测太不友好了，但是没关系，LUT 会出手☝️🤓
```

在原来的 `ip_str_less` 函数中，每比较一个 ip 至少需要 8 个分支，这太慢了

为了解决这个问题，我们引入编译期生成查找表 (Look-Up Table, LUT)： 通过 constexpr 函数 generate_lex_lut 在编译时就生成一个全局数组 g_lex_lut， 将0-255的数字映射到它们的字典序排名。例如，g_lex_lut[10] 的值会大于 g_lex_lut[2]。这样，原本复杂的字符串比较就变成了 O(1) 的数组查询

??? example "ip_str_less"
    ```c++
    constexpr int compare_num_str(int x, int y) {
        if (x < 10) {
            if (y < 10) {
                return x - y;
            } else if (y < 100) {
                int cmp = x - (y / 10);
                if (cmp != 0) return cmp;
                return -1;
            } else {
                int cmp = x - (y / 100);
                if (cmp != 0) return cmp;
                return -1;
            }
        } else if (x < 100) {
            if (y < 10) {
                int cmp = (x / 10) - y;
                if (cmp != 0) return cmp;
                return 1;
            } else if (y < 100) {
                int cmp = (x / 10) - (y / 10);
                if (cmp != 0) return cmp;
                return (x % 10) - (y % 10);
            } else {
                int cmp = (x / 10) - (y / 100);
                if (cmp != 0) return cmp;
                cmp = (x % 10) - (y / 10 % 10);
                if (cmp != 0) return cmp;
                return -1;
            }
        } else {
            if (y < 10) {
                int cmp = (x / 100) - y;
                if (cmp != 0) return cmp;
                return 1;
            } else if (y < 100) {
                int cmp = (x / 100) - (y / 10);
                if (cmp != 0) return cmp;
                cmp = (x / 10 % 10) - (y % 10);
                if (cmp != 0) return cmp;
                return 1;
            } else {
                int cmp = (x / 100) - (y / 100);
                if (cmp != 0) return cmp;
                cmp = (x / 10 % 10) - (y / 10 % 10);
                if (cmp != 0) return cmp;
                return (x % 10) - (y % 10);
            }
        }
    }

    template<typename T>
    constexpr void constexpr_swap(T& a, T& b) {
        T temp = a;
        a = b;
        b = temp;
    }

    constexpr auto generate_lex_lut() {
        std::array<int, 256> nums{};
        std::array<int, 256> lut{};
        for (int i = 0; i < 256; ++i) nums[i] = i;
        for (int i = 0; i < 256; ++i) {
            int min_idx = i;
            for (int j = i + 1; j < 256; ++j) {
                if (compare_num_str(nums[j], nums[min_idx]) < 0) min_idx = j;
            }
            if (min_idx != i) constexpr_swap(nums[i], nums[min_idx]);
        }
        for (int rank = 0; rank < 256; ++rank) lut[nums[rank]] = rank;
        return lut;
    }
    constexpr auto g_lex_lut = generate_lex_lut();

    inline bool ip_str_less(uint32_t ip1, uint32_t ip2) {
        int a1 = (ip1 >> 24) & 0xFF, b1 = (ip1 >> 16) & 0xFF, c1 = (ip1 >> 8) & 0xFF, d1 = ip1 & 0xFF;
        int a2 = (ip2 >> 24) & 0xFF, b2 = (ip2 >> 16) & 0xFF, c2 = (ip2 >> 8) & 0xFF, d2 = ip2 & 0xFF;
        if (g_lex_lut[a1] != g_lex_lut[a2]) return g_lex_lut[a1] < g_lex_lut[a2];
        if (g_lex_lut[b1] != g_lex_lut[b2]) return g_lex_lut[b1] < g_lex_lut[b2];
        if (g_lex_lut[c1] != g_lex_lut[c2]) return g_lex_lut[c1] < g_lex_lut[c2];
        return g_lex_lut[d1] < g_lex_lut[d2];
    }
    ```

我们同时也可以替换掉原来的 `std::sort`，使用 “排序键”，这个键的构造方式保证了键的数值大小顺序是我们自定义的字典序

做完这个优化后我们发现，程序的主要瓶颈是并行部分结束后的合并阶段，这个合并阶段只能串行处理

因此我们将这个过程优化为增量式并行合并

- 创建了固定数量的全局“分片”（global_syn_shards, global_dns_shards），例如1024个。每个分片都是一个独立的 hash_map，并配有一个独立的 mutex 锁

- 在每个线程内部，使用 thread_local 关键字创建了与全局分片同样结构的本地分片。在核心的日志解析阶段，所有操作都在线程局部缓存上进行，完全无锁

- 当一个线程处理完自己的数据块后，它会遍历自己的本地分片。对于每个本地分片，它会获取对应全局分片的锁，然后将数据合并进去。因为有1024个分片和锁，不同线程同时需要锁定同一个分片的概率大大降低，锁竞争被降到了最低

- 使用 #pragma omp parallel for 来并行处理全局分片。每个线程负责一部分分片的统计工作，将结果存入线程自己的局部 hash_map。最后，通过 #pragma omp critical 临界区将各个线程的局部统计结果安全地合并到最终的总表中

通过这些优化，我们的程序从需要 `9.473` 加速到了只需需要 `1.721s`，加速约 5.5 倍

总结下来，我们所做的三阶段优化，将程序从需要 `233.565s` 加速到了只需要 `1.721s`，总共加速约 135.7 倍，可喜可贺可喜可贺

??? success "最终优化代码 (196.6pts)"
    ```c++
    --8<-- "docs/code/td3.cpp"
    ```

## 后记

我在比赛时**巨量**使用了 LLM，几乎所有代码都是我提供思路，由 LLM 实际编写的，感谢以下模型对我的鼎力支持

- Qwen3-235B-A22B-Thinking-2507

- DeepSeek-V3.1

- Gemini 2.5 pro

- GPT 5

- Grok 4

- Claude Sonnet 4



[^1]: kotoha ちゃん真的很可爱