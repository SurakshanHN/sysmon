#include "cpu_engine.hpp"

CpuMetrics CpuEngine::compute(const CpuRawData& prev, const CpuRawData& curr) {
    CpuMetrics metrics;

    metrics.elapsed_seconds = std::chrono::duration<double>(
        curr.timestamp - prev.timestamp).count();

    auto total = [](const CpuRawData& d) -> long long {
        return d.user + d.nice + d.system + d.idle
             + d.iowait + d.irq + d.softirq + d.steal;
    };

    auto active = [&total](const CpuRawData& d) -> long long {
        return total(d) - d.idle - d.iowait;
    };

    long long delta_total  = total(curr)  - total(prev);
    long long delta_active = active(curr) - active(prev);

    metrics.system_delta_jiffies = delta_total;

    if (delta_total == 0) {
        metrics.usage_percent = 0.0;
    } else {
        metrics.usage_percent =
            (static_cast<double>(delta_active) / delta_total) * 100.0;
    }

    return metrics;
}
