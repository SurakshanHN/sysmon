#include "cpu_collector.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>

CpuRawData CpuCollector::read() const {
    std::ifstream file(PROC_STAT_PATH);
    if (!file.is_open()) {
        throw std::runtime_error(
            std::string("CpuCollector: failed to open ") + PROC_STAT_PATH);
    }

    std::string line;
    if (!std::getline(file, line)) {
        throw std::runtime_error(
            "CpuCollector: failed to read first line from /proc/stat");
    }

    std::istringstream iss(line);
    std::string label;
    iss >> label;

    if (label != "cpu") {
        throw std::runtime_error(
            "CpuCollector: unexpected label in /proc/stat: " + label);
    }

    CpuRawData data;
    data.timestamp = std::chrono::steady_clock::now();

    if (!(iss >> data.user >> data.nice >> data.system >> data.idle
              >> data.iowait >> data.irq >> data.softirq >> data.steal)) {
        throw std::runtime_error(
            "CpuCollector: failed to parse CPU fields from /proc/stat");
    }


    if (data.user < 0 || data.nice < 0 || data.system < 0 || data.idle < 0 ||
        data.iowait < 0 || data.irq < 0 || data.softirq < 0 || data.steal < 0) {
        throw std::runtime_error(
            "CpuCollector: parsed negative value from /proc/stat");
    }

    return data;
}
