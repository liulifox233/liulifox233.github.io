#include "table.h"
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <immintrin.h>
#include <iostream>
#include <omp.h>
#include <stdexcept>

using __reg = __m512i;
class RndGen {
public:
  RndGen(uint64_t s0, uint64_t s1, uint64_t s2) {
    uint64_t base_s0 = s0;
    uint64_t base_s1 = s1;
    uint64_t base_s2 = s2;
    for (int i = 0; i < 16; i++) {
      if (i != 0) {
        s0 = apply_jump_to_state_scalar(base_s0, TRANSPOSED_JUMPS[i]);
        s1 = apply_jump_to_state_scalar(base_s1, TRANSPOSED_JUMPS[i]);
        s2 = apply_jump_to_state_scalar(base_s2, TRANSPOSED_JUMPS[i]);
      }
      // 应用跳转

      // 存储状态
      s0_states[i] = s0;
      s1_states[i] = s1;
      s2_states[i] = s2;
    }

    // 将状态加载到AVX512向量中
    load_states_to_vectors();
  }

  // 重载版本：使用预定义的跳转索引
  void jump_all_states_with_index(int jump_index) {
    if (jump_index < 0)
      return;

    // 将向量状态保存回数组，以便标量处理
    save_vectors_to_states();

    // 对每个状态应用指定的跳转
    for (int i = 0; i < 16; i++) {
      s0_states[i] = apply_jump_to_state_scalar(s0_states[i],
                                                TRANSPOSED_JUMPS[jump_index]);
      s1_states[i] = apply_jump_to_state_scalar(s1_states[i],
                                                TRANSPOSED_JUMPS[jump_index]);
      s2_states[i] = apply_jump_to_state_scalar(s2_states[i],
                                                TRANSPOSED_JUMPS[jump_index]);
    }

    // 重新加载到向量中
    load_states_to_vectors();
  }

  // 存储16个状态的数组
  uint64_t s0_states[16];
  uint64_t s1_states[16];
  uint64_t s2_states[16];

  // AVX512向量存储状态（使用两个向量处理16个状态）
  __m512i vec_s0_lo, vec_s0_hi;
  __m512i vec_s1_lo, vec_s1_hi;
  __m512i vec_s2_lo, vec_s2_hi;

  // 将状态数组加载到AVX512向量中
  void load_states_to_vectors() {
    vec_s0_lo = _mm512_loadu_si512((const void *)s0_states);
    vec_s0_hi = _mm512_loadu_si512((const void *)(s0_states + 8));
    vec_s1_lo = _mm512_loadu_si512((const void *)s1_states);
    vec_s1_hi = _mm512_loadu_si512((const void *)(s1_states + 8));
    vec_s2_lo = _mm512_loadu_si512((const void *)s2_states);
    vec_s2_hi = _mm512_loadu_si512((const void *)(s2_states + 8));
  }

  // 将AVX512向量保存回状态数组
  void save_vectors_to_states() {
    _mm512_storeu_si512((void *)s0_states, vec_s0_lo);
    _mm512_storeu_si512((void *)(s0_states + 8), vec_s0_hi);
    _mm512_storeu_si512((void *)s1_states, vec_s1_lo);
    _mm512_storeu_si512((void *)(s1_states + 8), vec_s1_hi);
    _mm512_storeu_si512((void *)s2_states, vec_s2_lo);
    _mm512_storeu_si512((void *)(s2_states + 8), vec_s2_hi);
  }

  // AVX512版本的状态更新函数
  void update_states_avx512() {
    // xorshift64操作 - 使用AVX512指令
    __m512i shift13 = _mm512_set1_epi64(13);
    __m512i shift7 = _mm512_set1_epi64(7);
    __m512i shift17 = _mm512_set1_epi64(17);

    // 更新 vec_s0_lo
    vec_s0_lo =
        _mm512_xor_si512(vec_s0_lo, _mm512_sllv_epi64(vec_s0_lo, shift13));
    vec_s0_lo =
        _mm512_xor_si512(vec_s0_lo, _mm512_srlv_epi64(vec_s0_lo, shift7));
    vec_s0_lo =
        _mm512_xor_si512(vec_s0_lo, _mm512_sllv_epi64(vec_s0_lo, shift17));

    // 更新 vec_s0_hi
    vec_s0_hi =
        _mm512_xor_si512(vec_s0_hi, _mm512_sllv_epi64(vec_s0_hi, shift13));
    vec_s0_hi =
        _mm512_xor_si512(vec_s0_hi, _mm512_srlv_epi64(vec_s0_hi, shift7));
    vec_s0_hi =
        _mm512_xor_si512(vec_s0_hi, _mm512_sllv_epi64(vec_s0_hi, shift17));

    // 更新 vec_s1_lo
    vec_s1_lo =
        _mm512_xor_si512(vec_s1_lo, _mm512_sllv_epi64(vec_s1_lo, shift13));
    vec_s1_lo =
        _mm512_xor_si512(vec_s1_lo, _mm512_srlv_epi64(vec_s1_lo, shift7));
    vec_s1_lo =
        _mm512_xor_si512(vec_s1_lo, _mm512_sllv_epi64(vec_s1_lo, shift17));

    // 更新 vec_s1_hi
    vec_s1_hi =
        _mm512_xor_si512(vec_s1_hi, _mm512_sllv_epi64(vec_s1_hi, shift13));
    vec_s1_hi =
        _mm512_xor_si512(vec_s1_hi, _mm512_srlv_epi64(vec_s1_hi, shift7));
    vec_s1_hi =
        _mm512_xor_si512(vec_s1_hi, _mm512_sllv_epi64(vec_s1_hi, shift17));

    // 更新 vec_s2_lo
    vec_s2_lo =
        _mm512_xor_si512(vec_s2_lo, _mm512_sllv_epi64(vec_s2_lo, shift13));
    vec_s2_lo =
        _mm512_xor_si512(vec_s2_lo, _mm512_srlv_epi64(vec_s2_lo, shift7));
    vec_s2_lo =
        _mm512_xor_si512(vec_s2_lo, _mm512_sllv_epi64(vec_s2_lo, shift17));

    // 更新 vec_s2_hi
    vec_s2_hi =
        _mm512_xor_si512(vec_s2_hi, _mm512_sllv_epi64(vec_s2_hi, shift13));
    vec_s2_hi =
        _mm512_xor_si512(vec_s2_hi, _mm512_srlv_epi64(vec_s2_hi, shift7));
    vec_s2_hi =
        _mm512_xor_si512(vec_s2_hi, _mm512_sllv_epi64(vec_s2_hi, shift17));
  }

  // 标量版本的跳转应用函数
  uint64_t apply_jump_to_state_scalar(uint64_t state,
                                      const TransposedBitMaskJump &jump_mask) {
    uint64_t result = 0;

    // 处理8个列块（每个块8列）
    for (int block = 0; block < 8; ++block) {
      uint64_t block_result = 0;

      // 处理块内的8列
      for (int col_in_block = 0; col_in_block < 8; ++col_in_block) {
        int col = block * 8 + col_in_block;
        uint64_t col_mask = jump_mask.T[col];

        // 计算 parity (popcount % 2)
        int parity = __builtin_popcountll(col_mask & state) & 1;
        block_result |= (static_cast<uint64_t>(parity) << col_in_block);
      }

      result |= (block_result << (block * 8));
    }

    return result;
  }
};
namespace md5_avx512_soa {

class MD5 {
public:
  static constexpr int HASH_COUNT = 16;
  static constexpr int BLOCK_SIZE = 64;
  static constexpr int WORDS_PER_BLOCK = BLOCK_SIZE / sizeof(uint32_t);

  struct InputSoA {
    __reg words[WORDS_PER_BLOCK];
  };

  MD5() {
    init();
    reset();
    target_digest_a = _mm512_setzero_si512();
    target_digest_b = _mm512_setzero_si512();
    target_digest_c = _mm512_setzero_si512();
    target_digest_d = _mm512_setzero_si512();
  }

  MD5 operator=(const MD5 &) = delete;
  MD5(MD5 &) = delete;
  MD5(const MD5 &) = delete;
  MD5(MD5 &&) = delete;
  MD5(const MD5 &&) = delete;

  void set_target_digest(const uint32_t target[4]) {
    target_digest_a = _mm512_set1_epi32((int)target[0]);
    target_digest_b = _mm512_set1_epi32((int)target[1]);
    target_digest_c = _mm512_set1_epi32((int)target[2]);
    target_digest_d = _mm512_set1_epi32((int)target[3]);
  }

  int find_match_index() const {
    if (!finalized)
      return -1;
    __mmask16 match_a = _mm512_cmpeq_epi32_mask(digest_a, target_digest_a);
    __mmask16 match_b = _mm512_cmpeq_epi32_mask(digest_b, target_digest_b);
    __mmask16 match_c = _mm512_cmpeq_epi32_mask(digest_c, target_digest_c);
    __mmask16 match_d = _mm512_cmpeq_epi32_mask(digest_d, target_digest_d);
    __mmask16 all_match = match_a & match_b & match_c & match_d;
    if (all_match != 0) {
      uint32_t bitmap = (uint32_t)all_match;
      return (int)_tzcnt_u32(bitmap);
    }
    return -1;
  }

  void reset() {
    finalized = false;
    state[0] = _mm512_set1_epi32(0x67452301);
    state[1] = _mm512_set1_epi32(0xefcdab89);
    state[2] = _mm512_set1_epi32(0x98badcfe);
    state[3] = _mm512_set1_epi32(0x10325476);
    digest_a = _mm512_setzero_si512();
    digest_b = _mm512_setzero_si512();
    digest_c = _mm512_setzero_si512();
    digest_d = _mm512_setzero_si512();
  }

  void calculate(RndGen &generator) {
    reset();
    transform(generator);
    finalize();
  }

private:
  void init() {
    reset();
    for (int i = 0; i < (int)(sizeof(k) / sizeof(k[0])); i++) {
      kv[i] = _mm512_set1_epi32((int)k[i]);
    }
  }

  void transform(RndGen &generator) {
    // 更新随机数生成器状态
    generator.update_states_avx512();

    // 直接提取并使用随机数，避免临时数组存储
    __m256i s0_lo_low = _mm512_cvtepi64_epi32(generator.vec_s0_lo);
    __m256i s0_hi_low = _mm512_cvtepi64_epi32(generator.vec_s0_hi);
    __reg x0 =
        _mm512_inserti32x8(_mm512_castsi256_si512(s0_lo_low), s0_hi_low, 1);

    __m256i s0_lo_high =
        _mm512_cvtepi64_epi32(_mm512_srli_epi64(generator.vec_s0_lo, 32));
    __m256i s0_hi_high =
        _mm512_cvtepi64_epi32(_mm512_srli_epi64(generator.vec_s0_hi, 32));
    __reg x1 =
        _mm512_inserti32x8(_mm512_castsi256_si512(s0_lo_high), s0_hi_high, 1);

    __m256i s1_lo_low = _mm512_cvtepi64_epi32(generator.vec_s1_lo);
    __m256i s1_hi_low = _mm512_cvtepi64_epi32(generator.vec_s1_hi);
    __reg x4 =
        _mm512_inserti32x8(_mm512_castsi256_si512(s1_lo_low), s1_hi_low, 1);

    __m256i s1_lo_high =
        _mm512_cvtepi64_epi32(_mm512_srli_epi64(generator.vec_s1_lo, 32));
    __m256i s1_hi_high =
        _mm512_cvtepi64_epi32(_mm512_srli_epi64(generator.vec_s1_hi, 32));
    __reg x5 =
        _mm512_inserti32x8(_mm512_castsi256_si512(s1_lo_high), s1_hi_high, 1);

    __m256i s2_lo_low = _mm512_cvtepi64_epi32(generator.vec_s2_lo);
    __m256i s2_hi_low = _mm512_cvtepi64_epi32(generator.vec_s2_hi);
    __reg x8 =
        _mm512_inserti32x8(_mm512_castsi256_si512(s2_lo_low), s2_hi_low, 1);

    __m256i s2_lo_high =
        _mm512_cvtepi64_epi32(_mm512_srli_epi64(generator.vec_s2_lo, 32));
    __m256i s2_hi_high =
        _mm512_cvtepi64_epi32(_mm512_srli_epi64(generator.vec_s2_hi, 32));
    __reg x9 =
        _mm512_inserti32x8(_mm512_castsi256_si512(s2_lo_high), s2_hi_high, 1);

    // 预先计算常量，避免重复设置
    const __reg x2 = _mm512_setzero_si512();
    const __reg x3 = _mm512_setzero_si512();
    const __reg x6 = _mm512_setzero_si512();
    const __reg x7 = _mm512_setzero_si512();
    const __reg x10 = _mm512_setzero_si512();
    const __reg x11 = _mm512_setzero_si512();
    const __reg x12 = _mm512_set1_epi32(0x80u);
    const __reg x13 = _mm512_setzero_si512();
    const __reg x14 = _mm512_set1_epi32(384u);
    const __reg x15 = _mm512_setzero_si512();

    __reg a = state[0], b = state[1], c = state[2], d = state[3];

    /* Round 1 - 保持原有逻辑 */
    FF<7>(a, b, c, d, x0, kv[0]);
    FF<12>(d, a, b, c, x1, kv[1]);
    FF<17>(c, d, a, b, x2, kv[2]);
    FF<22>(b, c, d, a, x3, kv[3]);
    FF<7>(a, b, c, d, x4, kv[4]);
    FF<12>(d, a, b, c, x5, kv[5]);
    FF<17>(c, d, a, b, x6, kv[6]);
    FF<22>(b, c, d, a, x7, kv[7]);
    FF<7>(a, b, c, d, x8, kv[8]);
    FF<12>(d, a, b, c, x9, kv[9]);
    FF<17>(c, d, a, b, x10, kv[10]);
    FF<22>(b, c, d, a, x11, kv[11]);
    FF<7>(a, b, c, d, x12, kv[12]);
    FF<12>(d, a, b, c, x13, kv[13]);
    FF<17>(c, d, a, b, x14, kv[14]);
    FF<22>(b, c, d, a, x15, kv[15]);

    /* Round 2 */
    GG<5>(a, b, c, d, x1, kv[16]);
    GG<9>(d, a, b, c, x6, kv[17]);
    GG<14>(c, d, a, b, x11, kv[18]);
    GG<20>(b, c, d, a, x0, kv[19]);
    GG<5>(a, b, c, d, x5, kv[20]);
    GG<9>(d, a, b, c, x10, kv[21]);
    GG<14>(c, d, a, b, x15, kv[22]);
    GG<20>(b, c, d, a, x4, kv[23]);
    GG<5>(a, b, c, d, x9, kv[24]);
    GG<9>(d, a, b, c, x14, kv[25]);
    GG<14>(c, d, a, b, x3, kv[26]);
    GG<20>(b, c, d, a, x8, kv[27]);
    GG<5>(a, b, c, d, x13, kv[28]);
    GG<9>(d, a, b, c, x2, kv[29]);
    GG<14>(c, d, a, b, x7, kv[30]);
    GG<20>(b, c, d, a, x12, kv[31]);

    /* Round 3 */
    HH<4>(a, b, c, d, x5, kv[32]);
    HH<11>(d, a, b, c, x8, kv[33]);
    HH<16>(c, d, a, b, x11, kv[34]);
    HH<23>(b, c, d, a, x14, kv[35]);
    HH<4>(a, b, c, d, x1, kv[36]);
    HH<11>(d, a, b, c, x4, kv[37]);
    HH<16>(c, d, a, b, x7, kv[38]);
    HH<23>(b, c, d, a, x10, kv[39]);
    HH<4>(a, b, c, d, x13, kv[40]);
    HH<11>(d, a, b, c, x0, kv[41]);
    HH<16>(c, d, a, b, x3, kv[42]);
    HH<23>(b, c, d, a, x6, kv[43]);
    HH<4>(a, b, c, d, x9, kv[44]);
    HH<11>(d, a, b, c, x12, kv[45]);
    HH<16>(c, d, a, b, x15, kv[46]);
    HH<23>(b, c, d, a, x2, kv[47]);

    /* Round 4 */
    II<6>(a, b, c, d, x0, kv[48]);
    II<10>(d, a, b, c, x7, kv[49]);
    II<15>(c, d, a, b, x14, kv[50]);
    II<21>(b, c, d, a, x5, kv[51]);
    II<6>(a, b, c, d, x12, kv[52]);
    II<10>(d, a, b, c, x3, kv[53]);
    II<15>(c, d, a, b, x10, kv[54]);
    II<21>(b, c, d, a, x1, kv[55]);
    II<6>(a, b, c, d, x8, kv[56]);
    II<10>(d, a, b, c, x15, kv[57]);
    II<15>(c, d, a, b, x6, kv[58]);
    II<21>(b, c, d, a, x13, kv[59]);
    II<6>(a, b, c, d, x4, kv[60]);
    II<10>(d, a, b, c, x11, kv[61]);
    II<15>(c, d, a, b, x2, kv[62]);
    II<21>(b, c, d, a, x9, kv[63]);

    state[0] = _mm512_add_epi32(state[0], a);
    state[1] = _mm512_add_epi32(state[1], b);
    state[2] = _mm512_add_epi32(state[2], c);
    state[3] = _mm512_add_epi32(state[3], d);
  }

  void finalize() {
    if (!finalized) {
      digest_a = state[0];
      digest_b = state[1];
      digest_c = state[2];
      digest_d = state[3];
      finalized = true;
    }
  }

  static inline __reg F(__reg x, __reg y, __reg z) {
    return _mm512_xor_si512(z, _mm512_and_si512(x, _mm512_xor_si512(y, z)));
  }

  static inline __reg G(__reg x, __reg y, __reg z) {
    return _mm512_xor_si512(y, _mm512_and_si512(z, _mm512_xor_si512(x, y)));
  }

  static inline __reg H(__reg x, __reg y, __reg z) {
    return _mm512_xor_si512(_mm512_xor_si512(x, y), z);
  }

  static inline __reg I(__reg x, __reg y, __reg z) {
    return _mm512_xor_si512(
        y,
        _mm512_or_si512(x, _mm512_xor_si512(z, _mm512_set1_epi32(0xFFFFFFFF))));
  }

  template <int N> static inline __reg rotate_left(__reg x) {
    return _mm512_or_si512(_mm512_slli_epi32(x, N),
                           _mm512_srli_epi32(x, 32 - N));
  }

  template <int S>
  static inline void FF(__reg &a, __reg b, __reg c, __reg d, __reg x,
                        __reg ac) {
    __reg f = F(b, c, d);
    __reg sum =
        _mm512_add_epi32(_mm512_add_epi32(a, f), _mm512_add_epi32(x, ac));
    a = _mm512_add_epi32(b, rotate_left<S>(sum));
  }

  template <int S>
  static inline void GG(__reg &a, __reg b, __reg c, __reg d, __reg x,
                        __reg ac) {
    __reg g = G(b, c, d);
    __reg sum =
        _mm512_add_epi32(_mm512_add_epi32(a, g), _mm512_add_epi32(x, ac));
    a = _mm512_add_epi32(b, rotate_left<S>(sum));
  }

  template <int S>
  static inline void HH(__reg &a, __reg b, __reg c, __reg d, __reg x,
                        __reg ac) {
    __reg h = H(b, c, d);
    __reg sum =
        _mm512_add_epi32(_mm512_add_epi32(a, h), _mm512_add_epi32(x, ac));
    a = _mm512_add_epi32(b, rotate_left<S>(sum));
  }

  template <int S>
  static inline void II(__reg &a, __reg b, __reg c, __reg d, __reg x,
                        __reg ac) {
    __reg i = I(b, c, d);
    __reg sum =
        _mm512_add_epi32(_mm512_add_epi32(a, i), _mm512_add_epi32(x, ac));
    a = _mm512_add_epi32(b, rotate_left<S>(sum));
  }

  __reg state[4];
  __reg digest_a, digest_b, digest_c, digest_d;
  __reg target_digest_a, target_digest_b, target_digest_c, target_digest_d;
  bool finalized;

  inline static constexpr uint32_t r[] = {7, 12, 17, 22, 5, 9,  14, 20,
                                          4, 11, 16, 23, 6, 10, 15, 21};

  inline static constexpr uint32_t k[] = {
      0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee, 0xf57c0faf, 0x4787c62a,
      0xa8304613, 0xfd469501, 0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
      0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821, 0xf61e2562, 0xc040b340,
      0x265e5a51, 0xe9b6c7aa, 0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
      0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed, 0xa9e3e905, 0xfcefa3f8,
      0x676f02d9, 0x8d2a4c8a, 0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
      0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70, 0x289b7ec6, 0xeaa127fa,
      0xd4ef3085, 0x04881d05, 0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
      0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039, 0x655b59c3, 0x8f0ccc92,
      0xffeff47d, 0x85845dd1, 0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
      0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391};

  __reg kv[sizeof(k) / sizeof(*k)];
};

} // namespace md5_avx512_soa

// 解析 hex digest（保持原来字节翻转处理）
void parse_hex_digest(const std::string &hex, uint32_t digest[4]) {
  if (hex.length() != 32) {
    throw std::runtime_error("Invalid hex digest length.");
  }
  for (int i = 0; i < 4; ++i) {
    uint64_t part = std::stoull(hex.substr(i * 8, 8), nullptr, 16);
    digest[i] = ((part & 0xFF000000) >> 24) | ((part & 0x00FF0000) >> 8) |
                ((part & 0x0000FF00) << 8) | ((part & 0x000000FF) << 24);
  }
}
int main() {
  std::ios_base::sync_with_stdio(false);
  std::cin.tie(NULL);

  const int num_threads = 40;
  omp_set_num_threads(num_threads);

  constexpr int PHYSICAL_BATCH_SIZE = 16;         // SIMD lane 数
  constexpr long long SIMD_BATCH_SIZE = 1024 * 8; // 8192
  constexpr long long STEPS_PER_JUMP = 8192;      // 每个 jump 单位跳 8192 步
  constexpr long long THREAD_BATCH_SIZE =
      SIMD_BATCH_SIZE * PHYSICAL_BATCH_SIZE; // 131072

  static_assert(THREAD_BATCH_SIZE % STEPS_PER_JUMP == 0,
                "THREAD_BATCH_SIZE must be multiple of STEPS_PER_JUMP");
  constexpr int JUMP_UNITS_PER_BATCH =
      THREAD_BATCH_SIZE / STEPS_PER_JUMP; // 应为 16

  for (int test_case = 0; test_case < 5; ++test_case) {
    std::string s0_hex, s1_hex, s2_hex;
    if (!(std::cin >> s0_hex >> s1_hex >> s2_hex))
      return 0;
    std::string target_hex;
    if (!(std::cin >> target_hex))
      return 0;

    uint64_t s0 = std::stoull(s0_hex, nullptr, 16);
    uint64_t s1 = std::stoull(s1_hex, nullptr, 16);
    uint64_t s2 = std::stoull(s2_hex, nullptr, 16);

    uint32_t target_digest[4];
    try {
      parse_hex_digest(target_hex, target_digest);
    } catch (const std::exception &e) {
      std::cerr << "Error parsing target hash: " << e.what() << std::endl;
      return 1;
    }

    volatile long long found_n = -1;

#pragma omp parallel
    {
      int thread_id = omp_get_thread_num();

      RndGen generator(s0, s1, s2);
      if (thread_id > 0) {
        generator.jump_all_states_with_index(thread_id * PHYSICAL_BATCH_SIZE);
      }

      md5_avx512_soa::MD5 md5_calc;
      md5_calc.set_target_digest(target_digest);

      long long base_n = (long long)thread_id * THREAD_BATCH_SIZE + 1;

      while (found_n == -1) {
        for (long long simd_iter = 0; simd_iter < SIMD_BATCH_SIZE;
             ++simd_iter) {
          md5_calc.calculate(generator);

          int match_index = md5_calc.find_match_index();
          if (match_index != -1) {
            // 计算真实 n 值
            long long current_n =
                base_n + simd_iter + match_index * SIMD_BATCH_SIZE;
            long long expected = -1;
            if (__atomic_compare_exchange(&found_n, &expected, &current_n,
                                          false, __ATOMIC_SEQ_CST,
                                          __ATOMIC_SEQ_CST)) {
            }
            break;
          }
        }

        if (found_n != -1) {
          break;
        }

        generator.jump_all_states_with_index(40 * 16 - 1);

        base_n += (long long)40 * THREAD_BATCH_SIZE;
      }
    }

    if (found_n != -1) {
      std::cout << found_n << std::endl;
    } else {
      std::cout << -1 << std::endl;
    }
  }

  return 0;
}