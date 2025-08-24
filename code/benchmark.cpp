#include <iostream>
#include <vector>
#include <cstdint>
#include <cmath>
#include <algorithm>
#include <cblas.h>
#include <omp.h>
#include <chrono>

using namespace std;
using namespace std::chrono;

// 方案1：逐点计算
void brute_force(uint32_t N, uint32_t D, const vector<float>& data) {
    #pragma omp parallel for
    for (uint32_t i = 0; i < N; ++i) {
        const float* vec_i = data.data() + (size_t)i * D;
        float a=-1,b=-1,c=-1,d=-1;
        for (uint32_t j = 0; j < N; ++j) {
            if (i == j) continue;
            const float* vec_j = data.data() + (size_t)j * D;
            float sim = cblas_sdot((int)D, vec_i, 1, vec_j, 1);
            if (sim > a){ d=c; c=b; b=a; a=sim; }
            else if(sim > b){ d=c; c=b; b=sim; }
            else if(sim > c){ d=c; c=sim; }
            else if(sim > d){ d=sim; }
        }
    }
}

// 方案2：矩阵乘法
void gemm_based(uint32_t N, uint32_t D, const vector<float>& data) {
    vector<float> cos_matrix((size_t)N * N);
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
        float* row = cos_matrix.data() + (size_t)i * N;
        row[i] = -1.0f;
        float a=-1,b=-1,c=-1,d=-1;
        for(uint32_t j=0;j<N;j++){
            float sim = row[j];
            if(sim > a){ d=c; c=b; b=a; a=sim; }
            else if(sim > b){ d=c; c=b; b=sim; }
            else if(sim > c){ d=c; c=sim; }
            else if(sim > d){ d=sim; }
        }
    }
}

int main() {
    uint32_t D = 4096;
    vector<int> testNs = {256, 512, 1024, 1500, 2000, 3000, 4000};

    cout << "Benchmarking brute_force vs gemm...\n";
    for (auto N : testNs) {
        vector<float> data((size_t)N * D);
        // 随机初始化
        for (size_t i = 0; i < data.size(); i++) {
            data[i] = (float)rand() / RAND_MAX;
        }
        // 归一化
        for (uint32_t i = 0; i < N; i++) {
            float sum=0;
            for(uint32_t j=0;j<D;j++) sum += data[i*D+j]*data[i*D+j];
            float inv = 1.0f/(sqrt(sum)+1e-12f);
            for(uint32_t j=0;j<D;j++) data[i*D+j] *= inv;
        }

        auto start = high_resolution_clock::now();
        brute_force(N, D, data);
        auto end = high_resolution_clock::now();
        double t1 = duration_cast<milliseconds>(end - start).count();

        start = high_resolution_clock::now();
        gemm_based(N, D, data);
        end = high_resolution_clock::now();
        double t2 = duration_cast<milliseconds>(end - start).count();

        cout << "N="<<N<<" brute="<<t1<<" ms  gemm="<<t2<<" ms  -> faster: "
             << (t1<t2?"brute":"gemm") << "\n";
    }

    return 0;
}
