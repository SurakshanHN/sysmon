#ifndef SYSMON_HISTORY_BUFFER_HPP
#define SYSMON_HISTORY_BUFFER_HPP

#include "../engine/cpu_engine.hpp"
#include "../collector/memory_collector.hpp"
#include <deque>
#include <vector>
#include <chrono>

struct HistoryEntry {
    double       cpu_percent;
    long long    mem_used_kb;
    std::chrono::steady_clock::time_point timestamp;
};

class HistoryBuffer {
public:
    explicit HistoryBuffer(size_t capacity = 60);

    void push(const CpuMetrics& cpu, const MemoryMetrics& mem);

    std::vector<HistoryEntry> get_last(size_t n) const;

    size_t size() const;

private:
    std::deque<HistoryEntry> buffer_;
    size_t capacity_;
};

#endif 
