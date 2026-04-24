#include "process_engine.hpp"
#include <algorithm>

std::vector<ProcessMetrics> ProcessEngine::compute_all(
    const std::unordered_map<int, ProcessRawData>& prev_map,
    const std::unordered_map<int, ProcessRawData>& curr_map,
    long long system_delta) {
    
    std::vector<ProcessMetrics> metrics_list;
    metrics_list.reserve(curr_map.size());

    for (const auto& [pid, curr_proc] : curr_map) {
        std::optional<ProcessRawData> prev_proc = std::nullopt;
        auto it = prev_map.find(pid);
        if (it != prev_map.end()) {
            prev_proc = it->second;
        }

        metrics_list.push_back(compute(prev_proc, curr_proc, system_delta));
    }

    return metrics_list;
}

std::vector<ProcessMetrics> ProcessEngine::get_top_n(
    std::vector<ProcessMetrics>& all,
    size_t n,
    SortMode mode) {
    
    size_t mid = std::min(n, all.size());

    if (mode == SortMode::CPU) {
        std::partial_sort(all.begin(), all.begin() + mid, all.end(),
            [](const ProcessMetrics& a, const ProcessMetrics& b) {
                return a.cpu_percent > b.cpu_percent; 
            });
    } else {
        std::partial_sort(all.begin(), all.begin() + mid, all.end(),
            [](const ProcessMetrics& a, const ProcessMetrics& b) {
                return a.memory_kb > b.memory_kb;
            });
    }

    return std::vector<ProcessMetrics>(all.begin(), all.begin() + mid);
}

ProcessMetrics ProcessEngine::compute(const std::optional<ProcessRawData>& prev,
                                      const ProcessRawData& curr,
                                      long long system_delta) {
    ProcessMetrics metrics;
    metrics.pid = curr.pid;
    metrics.name = curr.name;
    metrics.memory_kb = curr.memory_kb;

    if (!prev || system_delta <= 0) {
        metrics.cpu_percent = 0.0;
    } else {
        long long prev_total = prev->utime + prev->stime;
        long long curr_total = curr.utime + curr.stime;
        long long delta_process = curr_total - prev_total;

        if (delta_process <= 0) {
            metrics.cpu_percent = 0.0;
        } else {
            metrics.cpu_percent = (static_cast<double>(delta_process) / system_delta) * 100.0;
        }
    }

    return metrics;
}
