#ifndef SYSMON_PROCESS_COLLECTOR_HPP
#define SYSMON_PROCESS_COLLECTOR_HPP

#include <chrono>
#include <string>
#include <vector>
#include <optional>

struct ProcessRawData {
    int pid;
    std::string name;
    long long utime;       
    long long stime;   
    long long memory_kb;   
    std::chrono::steady_clock::time_point timestamp;
};

class ProcessCollector {
public:
    std::vector<ProcessRawData> read_all() const;

private:
    std::optional<ProcessRawData> read_process(int pid) const;
    bool parse_stat_file(int pid, std::string& name, long long& utime, long long& stime) const;

    long long parse_status_file(int pid) const;
};

#endif 
