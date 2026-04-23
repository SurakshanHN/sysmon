#ifndef SYSMON_MEMORY_COLLECTOR_HPP
#define SYSMON_MEMORY_COLLECTOR_HPP

struct MemoryRawData {
    long long mem_total_kb     = 0;
    long long mem_free_kb      = 0;
    long long buffers_kb       = 0;
    long long cached_kb        = 0;
    long long s_reclaimable_kb = 0;
};

struct MemoryMetrics {
    long long total_kb   = 0;
    long long used_kb    = 0;
    double usage_percent = 0.0;
};

class MemoryCollector {
public:
    MemoryRawData read() const;

    static MemoryMetrics compute(const MemoryRawData& raw);

private:
    static constexpr const char* PROC_MEMINFO_PATH = "/proc/meminfo";
};

#endif
