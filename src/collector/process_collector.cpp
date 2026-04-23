#include "process_collector.hpp"

#include <dirent.h>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

std::vector<ProcessRawData> ProcessCollector::read_all() const {
    std::vector<ProcessRawData> processes;
    DIR* dir = opendir("/proc");
    if (!dir) return processes;

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        
        std::string name(entry->d_name);
        if (std::all_of(name.begin(), name.end(), ::isdigit)) {
            int pid = std::stoi(name);
            auto data = read_process(pid);
            if (data) {
                processes.push_back(std::move(*data));
            }
        }
    }
    closedir(dir);
    return processes;
}

std::optional<ProcessRawData> ProcessCollector::read_process(int pid) const {
    ProcessRawData data;
    data.pid = pid;
    data.timestamp = std::chrono::steady_clock::now();

    if (!parse_stat_file(pid, data.name, data.utime, data.stime)) {
        return std::nullopt; 
    }

    data.memory_kb = parse_status_file(pid);

    return data;
}

bool ProcessCollector::parse_stat_file(int pid, std::string& name, long long& utime, long long& stime) const {
    std::string path = "/proc/" + std::to_string(pid) + "/stat";
    std::ifstream file(path);
    if (!file.is_open()) return false;

    std::string content;
    std::getline(file, content);
    if (content.empty()) return false;

    size_t first_paren = content.find('(');
    size_t last_paren  = content.rfind(')');

    if (first_paren == std::string::npos || last_paren == std::string::npos || last_paren < first_paren) {
        return false;
    }

    name = content.substr(first_paren + 1, last_paren - first_paren - 1);

    std::string rest = content.substr(last_paren + 1);
    std::istringstream iss(rest);


    
    std::string dummy;
    for (int i = 0; i < 11; ++i) iss >> dummy;

    if (!(iss >> utime >> stime)) return false;

    return true;
}

long long ProcessCollector::parse_status_file(int pid) const {
    std::string path = "/proc/" + std::to_string(pid) + "/status";
    std::ifstream file(path);
    if (!file.is_open()) return 0;

    std::string line;
    while (std::getline(file, line)) {
        if (line.compare(0, 6, "VmRSS:") == 0) {
            std::istringstream iss(line.substr(6));
            long long value;
            if (iss >> value) return value;
        }
    }
    return 0; 
}
