#ifndef SYSMON_CPU_ENGINE_HPP
#define SYSMON_CPU_ENGINE_HPP

#include "../collector/cpu_collector.hpp"

struct CpuMetrics {
    double usage_percent      = 0.0;  // 0.0 – 100.0
    double elapsed_seconds    = 0.0;
    long long system_delta_jiffies = 0;
};

class CpuEngine {
public:
    static CpuMetrics compute(const CpuRawData& prev, const CpuRawData& curr);
};

#endif
