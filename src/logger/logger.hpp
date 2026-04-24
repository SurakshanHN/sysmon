#ifndef SYSMON_LOGGER_HPP
#define SYSMON_LOGGER_HPP

#include "../engine/cpu_engine.hpp"
#include "../collector/memory_collector.hpp"
#include "../analytics/anomaly_detector.hpp"
#include <string>
#include <fstream>
#include <vector>

class Logger {
public:
    explicit Logger(const std::string& filepath);
    ~Logger();

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    void log_cycle(int cycle,
                   const CpuMetrics& cpu,
                   const MemoryMetrics& mem,
                   const std::vector<Anomaly>& anomalies);

private:
    std::ofstream file_;
};

#endif 
