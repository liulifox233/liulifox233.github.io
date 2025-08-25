#include <iostream>
#include <vector>
#include <cstdint>
#include <cmath>
#include <algorithm>
#include <cblas.h>
#include <omp.h>

int main() {
    uint32_t N, D;
    std::cin.read(reinterpret_cast<char*>(&N), sizeof(N));
    std::cin.read(reinterpret_cast<char*>(&D), sizeof(D));

    std::vector<float> data(static_cast<size_t>(N) * D);
    std::cin.read(reinterpret_cast<char*>(data.data()), N * D * sizeof(float));

    std::vector<float> top4_sim(static_cast<size_t>(N) * 4);

    if (N < 1024) { // 1024 from benchmark.cpp
        #pragma omp parallel for 
        for (uint32_t i = 0; i < N; ++i) {
            float sum = 0.0f;
            const uint32_t base = i * D;
            for (uint32_t j = 0; j < D; ++j) {
                float v = data[base + j];
                sum += v * v;
            }
            float denom = std::sqrt(sum) + 1e-12f;
            float inv = 1.0f / denom;
            for (uint32_t j = 0; j < D; ++j) {
                data[base + j] *= inv;
            }
        }

        #pragma omp parallel for
        for (uint32_t i = 0; i < N; ++i) {
            const float* vec_i = data.data() + static_cast<size_t>(i) * D;

            float a = -1.0f, b = -1.0f, c = -1.0f, d = -1.0f;

            for (uint32_t j = 0; j < N; ++j) {
                if (i == j) continue;
                const float* vec_j = data.data() + static_cast<size_t>(j) * D;

                float sim = cblas_sdot((int)D, vec_i, 1, vec_j, 1);

                if (sim > a) { d=c; c=b; b=a; a=sim; }
                else if (sim > b) { d=c; c=b; b=sim; }
                else if (sim > c) { d=c; c=sim; }
                else if (sim > d) { d=sim; }
            }

            float* out = top4_sim.data() + static_cast<size_t>(i) * 4;
            out[0] = a; out[1] = b; out[2] = c; out[3] = d;
        }

    } else {
        std::vector<float> norms(N);
        #pragma omp parallel for
        for (uint32_t i = 0; i < N; ++i) {
            float sum = 0.0f;
            for (uint32_t j = 0; j < D; ++j) {
                float val = data[i * D + j];
                sum += val * val;
            }
            norms[i] = std::sqrt(sum) + 1e-12f;
            for (uint32_t j = 0; j < D; ++j) {
                data[i * D + j] /= norms[i];
            }
        }

        std::vector<float> cos_matrix(static_cast<size_t>(N) * N);
        cblas_sgemm(
            CblasRowMajor, CblasNoTrans, CblasTrans,
            N, N, D,
            1.0f,
            data.data(), D,
            data.data(), D,
            0.0f,
            cos_matrix.data(), N
        );

        #pragma omp parallel for
        for (uint32_t i = 0; i < N; ++i) {
            float* row = cos_matrix.data() + static_cast<size_t>(i) * N;
            row[i] = -1.0f;
            float a=-1,b=-1,c=-1,d=-1;
            for(uint32_t j=0;j<N;j++){
                float sim = row[j];
                if(sim > a){ d=c; c=b; b=a; a=sim; }
                else if(sim > b){ d=c; c=b; b=sim; }
                else if(sim > c){ d=c; c=sim; }
                else if(sim > d){ d=sim; }
            }
            float* out = top4_sim.data() + static_cast<size_t>(i) * 4;
            out[0]=a; out[1]=b; out[2]=c; out[3]=d;
        }
    }

    for (uint32_t i = 0; i < N; ++i) {
        std::cout.write(reinterpret_cast<const char*>(top4_sim.data() + static_cast<size_t>(i)*4),
                        4 * sizeof(float));
    }
    return 0;
}