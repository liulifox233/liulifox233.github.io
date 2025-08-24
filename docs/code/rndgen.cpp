class RndGen {
public:
    RndGen(uint64_t s0, uint64_t s1, uint64_t s2) {
        state_vec = _mm512_setr_epi64(s0, s1, s2, 0, 0, 0, 0, 0);
    }

    void generate(uint64_t out[6]) {
        update_state();
        alignas(64) uint64_t temp[8];
        _mm512_store_epi64(temp, state_vec);
        out[0] = temp[0]; out[1] = 0;
        out[2] = temp[1]; out[3] = 0;
        out[4] = temp[2]; out[5] = 0;
    }

    void generate_batch(int n, uint64_t out[][6]) {
        for (int i = 0; i < n; ++i) {
            update_state();
            alignas(64) uint64_t temp[8];
            _mm512_store_epi64(temp, state_vec);
            out[i][0] = temp[0]; out[i][1] = 0;
            out[i][2] = temp[1]; out[i][3] = 0;
            out[i][4] = temp[2]; out[i][5] = 0;
        }
    }

    void apply_bitmask_jump(const BitMaskJump& jump_mask) {
        alignas(64) uint64_t temp[8];
        _mm512_store_epi64(temp, state_vec);

        for (int lane = 0; lane < 8; ++lane) {
            uint64_t new_val = 0;
            for (int bit = 0; bit < 64; ++bit) {
                if (temp[lane] & (1ULL << bit)) {
                    new_val ^= jump_mask.masks[bit];
                }
            }
            temp[lane] = new_val;
        }

        state_vec = _mm512_load_epi64(temp);
    }

private:
    __m512i state_vec;

    inline void update_state() {
        __m512i x = state_vec;
        x = _mm512_xor_si512(x, _mm512_slli_epi64(x, 13));
        x = _mm512_xor_si512(x, _mm512_srli_epi64(x, 7));
        x = _mm512_xor_si512(x, _mm512_slli_epi64(x, 17));
        state_vec = x;
    }
};

void apply_precomputed_jump(RndGen& generator, int jump_index) {
    if (jump_index >= 0 && jump_index < 40) {
        generator.apply_bitmask_jump(PRECOMPUTED_JUMPS[jump_index]);
    }
}