#ifndef SYSMON_PROCESS_ENGINE_HPP
#define SYSMON_PROCESS_ENGINE_HPP

#include "../collector/process_collector.hpp"
#include <unordered_map>
#include <vector>

enum class SortMode {
    CPU,   
    MEMORY
};

struct ProcessMetrics {
    int pid;
    std::string name;
    double cpu_percent;
    long long memory_kb;
};

class ProcessEngine {
public:
    static std::vector<ProcessMetrics> compute_all(
        const std::unordered_map<int, ProcessRawData>& prev_map,
        const std::unordered_map<int, ProcessRawData>& curr_map,
        long long system_delta);

    static std::vector<ProcessMetrics> get_top_n(
        std::vector<ProcessMetrics>& all,
        size_t n,
        SortMode mode);

    static ProcessMetrics compute(const std::optional<ProcessRawData>& prev,
                                  const ProcessRawData& curr,
                                  long long system_delta);
};

#endif
