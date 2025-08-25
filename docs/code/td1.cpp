#include <iostream>
#include <fstream>
#include <string>
#include "absl/container/flat_hash_map.h"
#include <map>
#include <vector>

struct FiveTuple {
    std::string src_ip, dst_ip;
    int src_port, dst_port;

    bool operator==(const FiveTuple& o) const {
        return src_ip == o.src_ip && dst_ip == o.dst_ip &&
               src_port == o.src_port && dst_port == o.dst_port;
    }
};

struct HashFiveTuple {
    size_t operator()(const FiveTuple& t) const {
        size_t h1 = std::hash<std::string>{}(t.src_ip);
        size_t h2 = std::hash<std::string>{}(t.dst_ip);
        size_t h3 = std::hash<int>{}(t.src_port);
        size_t h4 = std::hash<int>{}(t.dst_port);
        return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3);
    }
};

std::string get_dns_prefix(const std::string& domain) {
    size_t pos = domain.find('.');
    if (pos != std::string::npos) return domain.substr(0, pos);
    return "";
}

int main() {
    absl::flat_hash_map<FiveTuple, std::vector<std::string>, HashFiveTuple> syn_flows;
    absl::flat_hash_map<std::string, int> dnstunnel_count;

    std::string line;
    while (std::getline(std::cin, line)) {
        if (line.empty()) continue;
        const char* p = line.c_str();

        // timestamp
        while (*p && *p != ' ') ++p; while (*p==' ') ++p;
        // protocol
        const char* prot_start = p;
        while (*p && *p != ' ') ++p; std::string protocol(prot_start, p - prot_start);
        while (*p==' ') ++p;
        // src_ip
        const char* src_start = p;
        while (*p && *p != ' ') ++p; std::string src_ip(src_start, p - src_start);
        while (*p==' ') ++p;
        // dst_ip
        const char* dst_start = p;
        while (*p && *p != ' ') ++p; std::string dst_ip(dst_start, p - dst_start);
        while (*p==' ') ++p;

        if (protocol == "TCP") {
            // src_port
            const char* s = p;
            while (*p && *p != ' ') ++p; int src_port = std::stoi(std::string(s, p-s));
            while (*p==' ') ++p;
            // dst_port
            s = p; while (*p && *p != ' ') ++p; int dst_port = std::stoi(std::string(s,p-s));
            while (*p==' ') ++p;
            // flags
            const char* fstart = p; while (*p && *p != ' ') ++p; std::string flags(fstart,p-fstart);

            FiveTuple key{src_ip,dst_ip,src_port,dst_port};
            syn_flows[key].push_back(flags);
        }
        else if (protocol == "DNS") {
            // skip src_port dst_port length
            for (int i=0;i<3;++i){
                while (*p && *p != ' ') ++p; while (*p==' ') ++p;
            }
            // domain
            std::string domain(p);
            std::string prefix = get_dns_prefix(domain);
            if (prefix.size() >= 30) dnstunnel_count[src_ip] += prefix.size();
        }
    }

    // portscan统计
    std::map<std::string,int> portscan_ip_count;
    for (auto& kv : syn_flows) {
        const auto& flags_vec = kv.second;
        if (flags_vec.size() == 1 && flags_vec[0]=="SYN")
            portscan_ip_count[kv.first.src_ip]++;
    }

    // 输出
    for (auto& kv : portscan_ip_count) std::cout << kv.first << " portscan " << kv.second << "\n";
    std::map<std::string,int> tunnelling_sorted(dnstunnel_count.begin(), dnstunnel_count.end());
    for (auto& kv : tunnelling_sorted) std::cout << kv.first << " tunnelling " << kv.second << "\n";

    return 0;
}