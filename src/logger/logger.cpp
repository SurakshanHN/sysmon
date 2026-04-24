#include "logger.hpp"
#include <chrono>
#include <iomanip>
#include <ctime>

Logger::Logger(const std::string& filepath) : file_(filepath, std::ios::app) {
    if (!file_.is_open()) {
        throw std::runtime_error("Could not open log file: " + filepath);
    }
}

Logger::~Logger() {
    if (file_.is_open()) {
        file_.close();
    }
}

void Logger::log_cycle(int cycle,
                       const CpuMetrics& cpu,
                       const MemoryMetrics& mem,
                       const std::vector<Anomaly>& anomalies) {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    
    struct tm* tm_ptr = std::localtime(&t);
    
    char time_buf[32];
    std::strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", tm_ptr);

    file_ << "[" << time_buf << "] cycle=" << cycle 
          << " cpu=" << std::fixed << std::setprecision(2) << cpu.usage_percent << "%"
          << " mem_used_mb=" << (mem.used_kb / 1024)
          << " mem_total_mb=" << (mem.total_kb / 1024)
          << " anomalies=" << anomalies.size();

    for (const auto& a : anomalies) {
        file_ << " ANOMALY:" << (a.type == AnomalyType::CPU_SPIKE ? "CPU_SPIKE" : "MEMORY_LEAK")
              << "(" << std::fixed << std::setprecision(1) << a.value << ")";
    }

    file_ << std::endl; 
}
