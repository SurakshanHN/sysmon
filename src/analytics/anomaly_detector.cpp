#include "anomaly_detector.hpp"

AnomalyDetector::AnomalyDetector(double cpu_threshold, int window)
    : cpu_threshold_(cpu_threshold), spike_window_(window), cpu_spike_streak_(0) {}

std::vector<Anomaly> AnomalyDetector::analyze(const CpuMetrics& cpu, const MemoryMetrics& mem) {
    std::vector<Anomaly> anomalies;

    /* CPU Spike Detection */
    if (cpu.usage_percent > cpu_threshold_) {
        cpu_spike_streak_++;
    } else {
        cpu_spike_streak_ = 0; 
    }

    if (cpu_spike_streak_ >= spike_window_) {
        anomalies.push_back({AnomalyType::CPU_SPIKE,
            "CPU spike: " + std::to_string(static_cast<int>(cpu.usage_percent)) + "% for "
            + std::to_string(cpu_spike_streak_) + " cycles",
            cpu.usage_percent});
    }

    /* Memory Leak Detection */
    mem_history_.push_back(mem.used_kb);
    if ((int)mem_history_.size() > leak_window_) {
        mem_history_.pop_front();
    }

    if ((int)mem_history_.size() == leak_window_) {
        bool is_leak = true;
        for (int i = 1; i < (int)mem_history_.size(); i++) {
            if (mem_history_[i] <= mem_history_[i-1]) {
                is_leak = false;
                break;
            }
        }
        if (is_leak) {
            anomalies.push_back({AnomalyType::MEMORY_LEAK,
                "Memory leak trend: increasing for "
                + std::to_string(leak_window_) + " consecutive cycles",
                static_cast<double>(mem.used_kb)});
        }
    }

    return anomalies;
}
