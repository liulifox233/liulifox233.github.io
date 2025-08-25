#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <cstring>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <omp.h>  // 添加OpenMP头文件

#include "absl/hash/hash.h"
#include "absl/container/flat_hash_map.h"
#include "absl/strings/str_format.h"

// 比较两个0-255整数的字符串表示（字典序）
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

// 五元组结构（只关心 TCP 五元组）
struct FiveTuple {
    uint32_t src_ip, dst_ip;
    int16_t src_port, dst_port;

    bool operator==(const FiveTuple& o) const {
        return src_ip == o.src_ip && dst_ip == o.dst_ip &&
               src_port == o.src_port && dst_port == o.dst_port;
    }
};

// 哈希函数
template <typename H>
H AbslHashValue(H h, const FiveTuple& t) {
    return H::combine(std::move(h), t.src_ip, t.dst_ip, t.src_port, t.dst_port);
}

// 手动解析整数
inline int parse_int(const char*& p) {
    int res = 0;
    while (*p >= '0' && *p <= '9') {
        res = res * 10 + (*p - '0');
        ++p;
    }
    return res;
}

// 跳过空格
inline void skip_ws(const char*& p) {
    while (*p == ' ') ++p;
}

// 解析 IPv4 地址为 uint32_t
uint32_t parse_ipv4(const char* start, const char* end) {
    uint32_t a = 0, b = 0, c = 0, d = 0;
    const char* p = start;
    while (p < end && *p != '.') a = a * 10 + (*p++ - '0'); if (p < end) ++p;
    while (p < end && *p != '.') b = b * 10 + (*p++ - '0'); if (p < end) ++p;
    while (p < end && *p != '.') c = c * 10 + (*p++ - '0'); if (p < end) ++p;
    while (p < end)              d = d * 10 + (*p++ - '0');
    return (a << 24) | (b << 16) | (c << 8) | d;
}

// 将 uint32_t IP 转为字符串（输出用）
std::string ipv4_to_string(uint32_t ip) {
    return absl::StrFormat("%d.%d.%d.%d",
        (ip >> 24) & 0xFF,
        (ip >> 16) & 0xFF,
        (ip >> 8) & 0xFF,
        ip & 0xFF);
}

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

    // 计算线程数（使用环境变量或默认8线程）
    int num_threads = omp_get_max_threads();
    if (num_threads > 16) num_threads = 16;  // 限制最大线程数

    // 计算分块边界（按行对齐）
    std::vector<std::pair<size_t, size_t>> chunks;
    size_t chunk_size = (file_size + num_threads - 1) / num_threads;
    size_t start = 0;

    for (int i = 0; i < num_threads; ++i) {
        size_t end = start + chunk_size;
        if (end > file_size) end = file_size;

        // 调整块结束位置到行尾
        size_t pos = end;
        while (pos < file_size && data[pos] != '\n' && data[pos] != '\r') {
            ++pos;
        }
        if (pos < file_size) {
            ++pos;  // 包含换行符
        }
        end = pos;

        // 调整块起始位置（如果非首块）
        if (start > 0) {
            size_t pos = start;
            while (pos > 0 && data[pos-1] != '\n' && data[pos-1] != '\r') {
                --pos;
            }
            start = pos;
        }

        // 有效块才添加
        if (start < end && start < file_size) {
            chunks.push_back({start, end});
        }

        // 下一个块起始
        start = end;
        if (start >= file_size) break;
    }

    // 创建线程局部存储
    std::vector<absl::flat_hash_map<FiveTuple, int>> thread_syn_flows(chunks.size());
    std::vector<absl::flat_hash_map<uint32_t, int>> thread_dnstunnel(chunks.size());

    // 多线程处理每个块
    #pragma omp parallel for schedule(static)
    for (int i = 0; i < chunks.size(); ++i) {
        size_t chunk_start = chunks[i].first;
        size_t chunk_end = chunks[i].second;

        // 预分配哈希表（根据块大小估算）
        size_t expected_flows = (chunk_end - chunk_start) / 100;
        thread_syn_flows[i].reserve(expected_flows);
        thread_dnstunnel[i].reserve(expected_flows / 10);

        const char* p = data + chunk_start;
        const char* end_ptr = data + chunk_end;

        while (p < end_ptr) {
            // 跳过空行
            while (p < end_ptr && (*p == '\r' || *p == '\n')) ++p;
            if (p >= end_ptr) break;

            const char* line_start = p;

            // 跳过时间戳（到第一个空格）
            while (p < end_ptr && *p != ' ') ++p;
            if (p >= end_ptr) break;
            skip_ws(p);

            // 解析协议
            const char* prot_start = p;
            while (p < end_ptr && *p != ' ') ++p;
            if (p == prot_start) continue; // 空行
            int prot_len = p - prot_start;
            skip_ws(p);

            if (prot_len == 3 && memcmp(prot_start, "TCP", 3) == 0) {
                // 解析 src_ip
                const char* src_ip_start = p;
                while (p < end_ptr && *p != ' ') ++p;
                uint32_t src_ip = parse_ipv4(src_ip_start, p);
                skip_ws(p);

                // dst_ip
                const char* dst_ip_start = p;
                while (p < end_ptr && *p != ' ') ++p;
                uint32_t dst_ip = parse_ipv4(dst_ip_start, p);
                skip_ws(p);

                // src_port
                int src_port = parse_int(p);
                skip_ws(p);

                // dst_port
                int dst_port = parse_int(p);
                skip_ws(p);

                // flags
                const char* flag_start = p;
                while (p < end_ptr && *p != ' ' && *p != '\r' && *p != '\n') ++p;
                bool is_syn = (p - flag_start == 3 && memcmp(flag_start, "SYN", 3) == 0);

                // 构造 key
                FiveTuple key{src_ip, dst_ip, static_cast<int16_t>(src_port), static_cast<int16_t>(dst_port)};

                // 修正后的TCP流统计逻辑
                if (is_syn) {
                    // 仅当流不存在时插入1，已存在则忽略
                    thread_syn_flows[i].try_emplace(key, 1);
                } else {
                    // 非SYN包：直接设置为-1（覆盖任何现有值）
                    thread_syn_flows[i][key] = -1;
                }

                // 跳到行尾
                while (p < end_ptr && *p != '\n') ++p;
                if (p < end_ptr) ++p;

            } else if (prot_len == 3 && memcmp(prot_start, "DNS", 3) == 0) {
                // 解析 src_ip
                const char* src_ip_start = p;
                while (p < end_ptr && *p != ' ') ++p;
                uint32_t src_ip = parse_ipv4(src_ip_start, p);
                skip_ws(p);

                // 跳过 dst_ip
                while (p < end_ptr && *p != ' ') ++p;
                skip_ws(p);

                // 跳过 src_port, dst_port, domain_length
                for (int i = 0; i < 3; ++i) {
                    while (p < end_ptr && *p != ' ') ++p;
                    skip_ws(p);
                }

                // 解析域名
                const char* domain_start = p;
                while (p < end_ptr && *p != '\r' && *p != '\n' && *p != ' ') ++p;
                int domain_len = p - domain_start;

                // 计算前缀长度（第一个 '.' 前）
                const char* dot = domain_start;
                while (dot < p && *dot != '.') ++dot;
                int prefix_len = dot - domain_start;

                if (prefix_len >= 30) {
                    thread_dnstunnel[i][src_ip] += prefix_len;
                }

                // 跳到下一行
                while (p < end_ptr && *p != '\n') ++p;
                if (p < end_ptr) ++p;
            } else {
                // 跳到行尾
                while (p < end_ptr && *p != '\n') ++p;
                if (p < end_ptr) ++p;
            }
        }
    }

    // 解除 mmap
    munmap(const_cast<char*>(data), file_size);
    close(fd);

    // 合并线程结果
    absl::flat_hash_map<FiveTuple, int> global_syn_flows;
    absl::flat_hash_map<uint32_t, int> global_dnstunnel;

    // 合并SYN流：只要任一线程标记为-1，全局即为-1
    for (int i = 0; i < chunks.size(); ++i) {
        for (const auto& [flow, value] : thread_syn_flows[i]) {
            auto it = global_syn_flows.find(flow);
            if (it == global_syn_flows.end()) {
                global_syn_flows[flow] = value;
            } else {
                // 优先保留-1（有后续包）
                if (value == -1) {
                    it->second = -1;
                }
            }
        }
    }

    // 合并DNS隧道
    for (int i = 0; i < chunks.size(); ++i) {
        for (const auto& [ip, count] : thread_dnstunnel[i]) {
            global_dnstunnel[ip] += count;
        }
    }

    // 收集 portscan 结果（只取值为1的流）
    std::vector<std::pair<uint32_t, int>> portscan_raw;
    for (const auto& [flow, value] : global_syn_flows) {
        if (value == 1) {
            portscan_raw.emplace_back(flow.src_ip, 1);
        }
    }

    // 按IP字典序排序（考虑点号）
    std::sort(portscan_raw.begin(), portscan_raw.end(), 
        [](const auto& a, const auto& b) {
            return ip_str_less(a.first, b.first);
        });

    // 合并相同 IP 的 portscan
    std::vector<std::pair<uint32_t, int>> final_portscan;
    for (const auto& item : portscan_raw) {
        if (!final_portscan.empty() && final_portscan.back().first == item.first) {
            final_portscan.back().second += item.second;
        } else {
            final_portscan.push_back(item);
        }
    }

    // 收集 tunnelling 并按字典序排序
    std::vector<std::pair<uint32_t, int>> tunnelling_raw(global_dnstunnel.begin(), global_dnstunnel.end());
    std::sort(tunnelling_raw.begin(), tunnelling_raw.end(), 
        [](const auto& a, const auto& b) {
            return ip_str_less(a.first, b.first);
        });

    // 输出结果
    for (const auto& p : final_portscan) {
        std::cout << ipv4_to_string(p.first) << " portscan " << p.second << '\n';
    }
    for (const auto& p : tunnelling_raw) {
        std::cout << ipv4_to_string(p.first) << " tunnelling " << p.second << '\n';
    }

    return 0;
}