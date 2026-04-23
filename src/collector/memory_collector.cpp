#include "memory_collector.hpp"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

MemoryRawData MemoryCollector::read() const {
    std::ifstream file(PROC_MEMINFO_PATH);
    if (!file.is_open()) {
        throw std::runtime_error(
            std::string("MemoryCollector: failed to open ") + PROC_MEMINFO_PATH);
    }

    MemoryRawData data;

    int fields_found = 0;
    constexpr int FIELDS_NEEDED = 5;

    std::string line;
    while (std::getline(file, line) && fields_found < FIELDS_NEEDED) {
        std::istringstream iss(line);
        std::string key;
        long long value;
        iss >> key >> value;

        if (key == "MemTotal:")      { data.mem_total_kb     = value; ++fields_found; }
        else if (key == "MemFree:")  { data.mem_free_kb      = value; ++fields_found; }
        else if (key == "Buffers:")  { data.buffers_kb       = value; ++fields_found; }
        else if (key == "Cached:")   { data.cached_kb        = value; ++fields_found; }
        else if (key == "SReclaimable:") { data.s_reclaimable_kb = value; ++fields_found; }
    }

    if (fields_found < FIELDS_NEEDED) {
        throw std::runtime_error(
            "MemoryCollector: could not find all required fields in /proc/meminfo "
            "(found " + std::to_string(fields_found) + "/" +
            std::to_string(FIELDS_NEEDED) + ")");
    }

    if (data.mem_total_kb < 0 || data.mem_free_kb < 0 ||
        data.buffers_kb < 0 || data.cached_kb < 0 ||
        data.s_reclaimable_kb < 0) {
        throw std::runtime_error(
            "MemoryCollector: parsed negative value from /proc/meminfo");
    }

    return data;
}

MemoryMetrics MemoryCollector::compute(const MemoryRawData& raw) {
    MemoryMetrics metrics;
    metrics.total_kb = raw.mem_total_kb;

    metrics.used_kb = raw.mem_total_kb
                    - raw.mem_free_kb
                    - raw.buffers_kb
                    - raw.cached_kb
                    - raw.s_reclaimable_kb;

    metrics.used_kb = std::max(metrics.used_kb, 0LL);

    if (metrics.total_kb > 0) {
        metrics.usage_percent =
            (static_cast<double>(metrics.used_kb) / metrics.total_kb) * 100.0;
    } else {
        metrics.usage_percent = 0.0;
    }

    return metrics;
}
