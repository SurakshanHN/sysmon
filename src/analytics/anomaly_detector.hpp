#ifndef SYSMON_ANOMALY_DETECTOR_HPP
#define SYSMON_ANOMALY_DETECTOR_HPP

#include "../engine/cpu_engine.hpp"
#include "../collector/memory_collector.hpp"
#include <vector>
#include <string>
#include <deque>

enum class AnomalyType {
    CPU_SPIKE,      
    MEMORY_LEAK 
};

struct Anomaly {
    AnomalyType type;
    std::string message; 
    double value;  
};

class AnomalyDetector {
public:
    explicit AnomalyDetector(double cpu_threshold = 85.0, int window = 3);

    std::vector<Anomaly> analyze(const CpuMetrics& cpu, const MemoryMetrics& mem);

private:
    double cpu_threshold_;
    int    spike_window_; 
    int    cpu_spike_streak_;  

    std::deque<long long> mem_history_;
    int    leak_window_ = 5;  
};

#endif