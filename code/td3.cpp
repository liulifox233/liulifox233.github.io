// mmap_minimal_change.cpp
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <cstring>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <omp.h>
#include <cstdint>
#include <mutex>
#include <array>
#include "absl/container/flat_hash_map.h"
#include "absl/strings/str_format.h"

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

inline uint32_t make_sort_key(uint32_t ip) {
    uint8_t a = (ip >> 24) & 0xFF;
    uint8_t b = (ip >> 16) & 0xFF;
    uint8_t c = (ip >>  8) & 0xFF;
    uint8_t d = (ip      ) & 0xFF;
    return (static_cast<uint32_t>(g_lex_lut[a]) << 24) |
           (static_cast<uint32_t>(g_lex_lut[b]) << 16) |
           (static_cast<uint32_t>(g_lex_lut[c]) <<  8) |
           (static_cast<uint32_t>(g_lex_lut[d]));
}

struct FiveTuple {
    uint32_t src_ip, dst_ip;
    int16_t src_port, dst_port;

    bool operator==(const FiveTuple& o) const {
        return src_ip == o.src_ip && dst_ip == o.dst_ip &&
               src_port == o.src_port && dst_port == o.dst_port;
    }
};

struct FastFiveTupleHash {
    ssize_t operator()(const FiveTuple& t) const noexcept {
        return static_cast<ssize_t>(t.src_ip ^ t.dst_ip ^
               static_cast<uint32_t>(t.src_port) ^
               static_cast<uint32_t>(t.dst_port));
    }
};

struct FastIPHash {
    ssize_t operator()(uint32_t ip) const noexcept {
        return static_cast<ssize_t>(ip);
    }
};

template <typename H>
H AbslHashValue(H h, const FiveTuple& t) {
    return H::combine(std::move(h), t.src_ip, t.dst_ip, t.src_port, t.dst_port);
}

inline int parse_int(const char*& p) {
    int res = 0;
    while (*p >= '0' && *p <= '9') {
        res = res * 10 + (*p - '0');
        ++p;
    }
    return res;
}

inline void skip_ws(const char*& p) {
    while (*p == ' ') ++p;
}

inline uint32_t parse_ip_inline(const char*& p) {
    uint32_t a = 0, b = 0, c = 0, d = 0;
    while (*p >= '0' && *p <= '9') {
        a = a * 10 + (*p - '0');
        p++;
    }
    if (*p == '.') p++;
    while (*p >= '0' && *p <= '9') {
        b = b * 10 + (*p - '0');
        p++;
    }
    if (*p == '.') p++;
    while (*p >= '0' && *p <= '9') {
        c = c * 10 + (*p - '0');
        p++;
    }
    if (*p == '.') p++;
    while (*p >= '0' && *p <= '9') {
        d = d * 10 + (*p - '0');
        p++;
    }
    return (a << 24) | (b << 16) | (c << 8) | d;
}

std::string ipv4_to_string(uint32_t ip) {
    return absl::StrFormat("%d.%d.%d.%d",
        (ip >> 24) & 0xFF,
        (ip >> 16) & 0xFF,
        (ip >> 8) & 0xFF,
        ip & 0xFF);
}

// ------------------ main ------------------
int main() {
    // I/O: Assume stdin is a regular file and use mmap
    struct stat st;
    if (fstat(fileno(stdin), &st) != 0 || !S_ISREG(st.st_mode)) {
        std::cerr << "输入必须是一个常规文件。\n";
        return 1;
    }

    size_t file_size = st.st_size;
    if (file_size == 0) {
        return 0; // Empty file, nothing to do
    }

    // mmap the file
    int fd = fileno(stdin);
    posix_fadvise(fd, 0, 0, POSIX_FADV_SEQUENTIAL);
    void* mapped = mmap(nullptr, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (mapped == MAP_FAILED) {
        perror("mmap failed");
        return 1;
    }
    const char* data = static_cast<const char*>(mapped);

    // --- Start of processing logic ---

    int num_threads = omp_get_max_threads();
    if (num_threads > 40) num_threads = 40;

    std::vector<std::pair<size_t, size_t>> chunks;
    size_t chunk_size_val = (file_size + num_threads - 1) / num_threads;
    size_t start = 0;
    for (int i = 0; i < num_threads; ++i) {
        size_t end = start + chunk_size_val;
        if (end > file_size) end = file_size;

        // Align end to the next newline to avoid splitting lines
        size_t pos = end;
        while (pos < file_size && data[pos] != '\n' && data[pos] != '\r') ++pos;
        if (pos < file_size) {
            ++pos;
        }
        end = pos;

        // Align start to the previous newline (for chunks > 0)
        if (start > 0) {
            size_t pos2 = start;
            while (pos2 > 0 && data[pos2-1] != '\n' && data[pos2-1] != '\r') --pos2;
            start = pos2;
        }

        if (start < end && start < file_size) {
            chunks.push_back({start, end});
        }

        start = end;
        if (start >= file_size) break;
    }

    const int num_shards = 1024;
    std::vector<absl::flat_hash_map<FiveTuple, int, FastFiveTupleHash>> global_syn_shards(num_shards);
    std::vector<absl::flat_hash_map<uint32_t, int, FastIPHash>> global_dns_shards(num_shards);
    std::vector<std::mutex> syn_mutexes(num_shards);
    std::vector<std::mutex> dns_mutexes(num_shards);

    #pragma omp parallel for schedule(static)
    for (int i = 0; i < static_cast<int>(chunks.size()); ++i) {
        thread_local std::vector<absl::flat_hash_map<FiveTuple, int, FastFiveTupleHash>> tls_syn_shards(num_shards);
        thread_local std::vector<absl::flat_hash_map<uint32_t, int, FastIPHash>> tls_dns_shards(num_shards);

        size_t chunk_start = chunks[i].first;
        size_t chunk_end = chunks[i].second;

        const char* p = data + chunk_start;
        const char* end_ptr = data + chunk_end;

        while (p < end_ptr) {
            while (p < end_ptr && (*p == '\r' || *p == '\n')) ++p;
            if (p >= end_ptr) break;

            // Skip timestamp
            while (p < end_ptr && *p != ' ') ++p;
            if (p >= end_ptr) break;
            skip_ws(p);

            // Parse protocol
            const char* prot_start = p;
            while (p < end_ptr && *p != ' ') ++p;
            if (p == prot_start) continue;
            int prot_len = p - prot_start;
            skip_ws(p);

            if (prot_len == 3 && memcmp(prot_start, "TCP", 3) == 0) {
                uint32_t src_ip = parse_ip_inline(p);
                skip_ws(p);

                uint32_t dst_ip = parse_ip_inline(p);
                skip_ws(p);

                int src_port = parse_int(p);
                skip_ws(p);

                int dst_port = parse_int(p);
                skip_ws(p);

                const char* flag_start = p;
                while (p < end_ptr && *p != ' ' && *p != '\r' && *p != '\n') ++p;
                bool is_syn = (p - flag_start == 3 && memcmp(flag_start, "SYN", 3) == 0);

                FiveTuple key{src_ip, dst_ip, static_cast<int16_t>(src_port), static_cast<int16_t>(dst_port)};

                size_t shard_idx = absl::HashOf(key) % num_shards;
                if (is_syn) {
                    tls_syn_shards[shard_idx].try_emplace(key, 1);
                } else {
                    tls_syn_shards[shard_idx][key] = -1;
                }

            } else if (prot_len == 3 && memcmp(prot_start, "DNS", 3) == 0) {
                uint32_t src_ip = parse_ip_inline(p);
                skip_ws(p);

                // Skip fields to get to domain name
                for (int j = 0; j < 4; ++j) {
                    while (p < end_ptr && *p != ' ') ++p;
                    skip_ws(p);
                }

                const char* domain_start = p;
                while (p < end_ptr && *p != '\r' && *p != '\n' && *p != ' ') ++p;

                const char* dot = domain_start;
                while (dot < p && *dot != '.') ++dot;
                int prefix_len = dot - domain_start;

                if (prefix_len >= 30) {
                    size_t shard_idx = absl::HashOf(src_ip) % num_shards;
                    tls_dns_shards[shard_idx][src_ip] += prefix_len;
                }
            }
            
            // Go to the next line
            while (p < end_ptr && *p != '\n') ++p;
            if (p < end_ptr) ++p;
        } // end parse loop

        // Merge thread-local shards into global shards
        for (int shard_idx = 0; shard_idx < num_shards; ++shard_idx) {
            if (!tls_syn_shards[shard_idx].empty()) {
                std::lock_guard<std::mutex> lock(syn_mutexes[shard_idx]);
                for (const auto& [flow, value] : tls_syn_shards[shard_idx]) {
                    auto it = global_syn_shards[shard_idx].find(flow);
                    if (it == global_syn_shards[shard_idx].end()) {
                        global_syn_shards[shard_idx][flow] = value;
                    } else {
                        if (value == -1) {
                            it->second = -1;
                        }
                    }
                }
                tls_syn_shards[shard_idx].clear();
            }

            if (!tls_dns_shards[shard_idx].empty()) {
                std::lock_guard<std::mutex> lock(dns_mutexes[shard_idx]);
                for (const auto& [ip, count] : tls_dns_shards[shard_idx]) {
                    global_dns_shards[shard_idx][ip] += count;
                }
                tls_dns_shards[shard_idx].clear();
            }
        }
    } // end parallel for

    // --- Aggregate and output results ---
    absl::flat_hash_map<uint32_t, int> portscan_merged;
    absl::flat_hash_map<uint32_t, int> tunnelling_merged;

    #pragma omp parallel for schedule(static)
    for (int shard_idx = 0; shard_idx < num_shards; ++shard_idx) {
        absl::flat_hash_map<uint32_t, int> local_portscan;
        for (const auto& [flow, value] : global_syn_shards[shard_idx]) {
            if (value == 1) {
                local_portscan[flow.src_ip]++;
            }
        }
        #pragma omp critical
        for (const auto& [ip, count] : local_portscan) {
            portscan_merged[ip] += count;
        }
    }

    #pragma omp parallel for schedule(static)
    for (int shard_idx = 0; shard_idx < num_shards; ++shard_idx) {
        absl::flat_hash_map<uint32_t, int> local_tunnelling;
        for (const auto& [ip, count] : global_dns_shards[shard_idx]) {
            if (count > 0) {
                local_tunnelling[ip] += count;
            }
        }
        #pragma omp critical
        for (const auto& [ip, count] : local_tunnelling) {
            tunnelling_merged[ip] += count;
        }
    }

    // Sort and print portscan results
    std::vector<std::pair<uint32_t, std::pair<uint32_t, int>>> portscan_with_key;
    portscan_with_key.reserve(portscan_merged.size());
    for (const auto& [ip, count] : portscan_merged) {
        uint32_t sort_key = make_sort_key(ip);
        portscan_with_key.emplace_back(sort_key, std::make_pair(ip, count));
    }
    std::sort(portscan_with_key.begin(), portscan_with_key.end());

    // Sort and print tunnelling results
    std::vector<std::pair<uint32_t, std::pair<uint32_t, int>>> tunnelling_with_key;
    tunnelling_with_key.reserve(tunnelling_merged.size());
    for (const auto& [ip, count] : tunnelling_merged) {
        uint32_t sort_key = make_sort_key(ip);
        tunnelling_with_key.emplace_back(sort_key, std::make_pair(ip, count));
    }
    std::sort(tunnelling_with_key.begin(), tunnelling_with_key.end());

    for (const auto& p : portscan_with_key) {
        std::cout << ipv4_to_string(p.second.first) << " portscan " << p.second.second << '\n';
    }
    for (const auto& p : tunnelling_with_key) {
        std::cout << ipv4_to_string(p.second.first) << " tunnelling " << p.second.second << '\n';
    }

    // --- Cleanup ---
    munmap(mapped, file_size);

    return 0;
}