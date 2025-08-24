#include <iostream>
#include <string>
#include <vector>
#include <cstdint>
#include <stdexcept>
#include <omp.h>

#include "md5-simd/source/simd/md5-simd.h"

// 题目提供的伪随机数生成器 (PRNG)
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

// 生成48字节的输入数据
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
// s_[3] is unused padding to avoid false sharing
uint64_t s_[4];
};


// 辅助函数：将16进制字符串解析为16字节的MD5摘要 (4个 uint32_t)
// MD5的输出是小端序的，这个函数正确处理字节序
void parse_hex_digest(const std::string& hex, uint32_t digest[4]) {
if (hex.length() != 32) {
    throw std::runtime_error("Invalid hex digest length.");
}
for (int i = 0; i < 4; ++i) {
    uint64_t part = std::stoull(hex.substr(i * 8, 8), nullptr, 16);
    // MD5的每个32位字在输出时是小端字节序
    digest[i] = ((part & 0xFF000000) >> 24) |
                ((part & 0x00FF0000) >> 8)  |
                ((part & 0x0000FF00) << 8)  |
                ((part & 0x000000FF) << 24);
}
}


int main()
{
// 加速C++标准IO
std::ios_base::sync_with_stdio(false);
std::cin.tie(NULL);

// 1. 读取输入
std::string s0_hex, s1_hex, s2_hex;
std::cin >> s0_hex >> s1_hex >> s2_hex;

std::string target_hex;
std::cin >> target_hex;

// 2. 解析输入
uint64_t s0 = std::stoull(s0_hex, nullptr, 16);
uint64_t s1 = std::stoull(s1_hex, nullptr, 16);
uint64_t s2 = std::stoull(s2_hex, nullptr, 16);

uint32_t target_digest[4];
try {
    parse_hex_digest(target_hex, target_digest);
} catch (const std::exception& e) {
    std::cerr << "Error parsing target hash: " << e.what() << std::endl;
    return 1;
}

// 共享变量，用于存储结果。volatile 确保所有线程能看到最新值。
volatile long long found_n = -1;

// 3. 使用OpenMP并行计算
#pragma omp parallel
{
    // 每个线程获取自己的ID和总线程数
    int thread_id = omp_get_thread_num();
    int num_threads = omp_get_num_threads();

    // 每个线程拥有自己的生成器实例，避免状态冲突
    RndGen generator(s0, s1, s2);
    
    // 每个线程拥有自己的MD5计算器实例
    md5_simd::MD5_SIMD md5_calc;

    // 准备计算所需的数据结构
    uint64_t input_buffer[6];
    const char* inputs[1] = { reinterpret_cast<const char*>(input_buffer) };
    uint64_t lengths[1] = { 48 }; // 输入长度固定为48字节

    // "快进"生成器状态，使每个线程从正确的交错起点开始
    uint64_t dummy_buffer[6];
    for (int i = 0; i < thread_id; ++i) {
        generator.generate(dummy_buffer);
    }

    // 4. 开始并行搜索循环
    // 每个线程的步长是总线程数
    for (long long n = thread_id + 1; found_n == -1; n += num_threads) {
        
        // 生成第n个输入
        generator.generate(input_buffer);

        // 计算MD5哈希
        // 使用<1, false>模板参数表示一次计算1个哈希，并且不做长度检查（因为长度固定）
        md5_calc.calculate<1, false>(inputs, lengths);

        // 比较结果
        std::string result_hex = md5_calc.hexdigest(0);
        uint32_t result_digest[4];
        try {
            parse_hex_digest(result_hex, result_digest);
            bool match = true;
            for (int i = 0; i < 4; ++i) {
                if (result_digest[i] != target_digest[i]) {
                    match = false;
                    break;
                }
            }
            
            if (match) {
                // 如果找到，就更新共享变量
                // 即使多个线程同时找到，这也能确保我们得到的是最小的n
                #pragma omp critical
                {
                    if (found_n == -1 || n < found_n) {
                        found_n = n;
                    }
                }
            }
        } catch (const std::exception& e) {
            // 忽略解析错误
        }
        
        // 为了跳到下一个属于本线程的任务，需要将生成器状态推进 (num_threads - 1) 次
        if (num_threads > 1) {
            for (int i = 0; i < num_threads - 1; ++i) {
                generator.generate(dummy_buffer);
            }
        }
    }
}

// 5. 输出结果
if (found_n != -1) {
    std::cout << found_n << std::endl;
}

return 0;
}