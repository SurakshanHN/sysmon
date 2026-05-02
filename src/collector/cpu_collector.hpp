#ifndef SYSMON_CPU_COLLECTOR_HPP
#define SYSMON_CPU_COLLECTOR_HPP

#include <chrono>
#include <string>

/*Raw CPU time data format parsed directly from /proc/stat*/
struct CpuRawData {
    long long user      = 0;
    long long nice      = 0;
    long long system    = 0;
    long long idle      = 0;
    long long iowait    = 0;
    long long irq       = 0;
    long long softirq   = 0;
    long long steal     = 0;
    std::chrono::steady_clock::time_point timestamp;
};

class CpuCollector {
public:
    CpuRawData read() const;

private:
    static constexpr const char* PROC_STAT_PATH = "/proc/stat";
};

#endif 
