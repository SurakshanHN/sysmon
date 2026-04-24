#include <chrono>
#include <iomanip>
#include <iostream>
#include <thread>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <csignal>
#include <sys/types.h>

#include "collector/cpu_collector.hpp"
#include "collector/memory_collector.hpp"
#include "collector/process_collector.hpp"
#include "engine/cpu_engine.hpp"
#include "ui/tui.hpp"
#include <ncurses.h>
#include "analytics/history_buffer.hpp"
#include "analytics/anomaly_detector.hpp"
#include "logger/logger.hpp"

void kill_selected_process(const ProcessMetrics& proc) {
    if (proc.pid <= 1) return;
    kill(proc.pid, SIGTERM);
}

int main() {
    std::string log_path = "sysmon.log";
    std::cerr << "Logging to: " << log_path << std::endl;

    TUI tui;
    HistoryBuffer history(60);
    AnomalyDetector detector(85.0, 3);
    Logger logger(log_path);

    CpuCollector cpu_collector;
    MemoryCollector memory_collector;
    ProcessCollector process_collector;

    std::unordered_map<int, ProcessRawData> prev_process_map;

    constexpr auto SAMPLE_INTERVAL = std::chrono::milliseconds(1000);
    int cycle = 0;
    SortMode current_sort_mode = SortMode::CPU;
    int selected_index = 0;
    bool confirm_mode = false;

    try {
        CpuRawData prev_cpu = cpu_collector.read();

        while (true) {
            CpuRawData curr_cpu = cpu_collector.read();
            MemoryRawData mem_raw = memory_collector.read();
            std::vector<ProcessRawData> curr_process_list = process_collector.read_all();

            std::unordered_map<int, ProcessRawData> curr_process_map;
            for (auto& proc : curr_process_list) {
                curr_process_map[proc.pid] = std::move(proc);
            }

            CpuMetrics cpu_metrics = CpuEngine::compute(prev_cpu, curr_cpu);
            MemoryMetrics mem_metrics = MemoryCollector::compute(mem_raw);
            
            std::vector<ProcessMetrics> all_metrics = std::move(ProcessEngine::compute_all(
                prev_process_map, curr_process_map, cpu_metrics.system_delta_jiffies));

            size_t total_processes = all_metrics.size();
            std::vector<ProcessMetrics> top_n = std::move(ProcessEngine::get_top_n(all_metrics, 200, current_sort_mode));

            
            history.push(cpu_metrics, mem_metrics);
            auto anomalies = detector.analyze(cpu_metrics, mem_metrics);
            logger.log_cycle(cycle, cpu_metrics, mem_metrics, anomalies);

            
            if (!top_n.empty()) {
                selected_index = std::clamp(selected_index, 0, (int)top_n.size() - 1);
            } else {
                selected_index = 0;
            }

            
            tui.draw(cpu_metrics, mem_metrics, top_n, total_processes, current_sort_mode, 
                     cycle, selected_index, confirm_mode, history.get_last(60), anomalies);

                     
            prev_cpu = curr_cpu;
            prev_process_map = std::move(curr_process_map);

            
            int key;
            while ((key = tui.poll_key()) != -1) {
                if (confirm_mode) {
                    if (key == 'y' || key == 'Y') {
                        if (selected_index < (int)top_n.size()) {
                            kill_selected_process(top_n[selected_index]);
                        }
                        confirm_mode = false;
                    } else if (key == 'n' || key == 'N') {
                        confirm_mode = false;
                    }
                } else {
                    if (key == 'q' || key == 'Q') {
                        return 0;
                    } else if (key == 'm' || key == 'M') {
                        current_sort_mode = (current_sort_mode == SortMode::CPU) ? SortMode::MEMORY : SortMode::CPU;
                    } else if (key == KEY_UP) {
                        selected_index = std::max(0, selected_index - 1);
                    } else if (key == KEY_DOWN) {
                        selected_index = std::min((int)top_n.size() - 1, selected_index + 1);
                    } else if (key == 'k' || key == 'K') {
                        if (!top_n.empty()) {
                            confirm_mode = true;
                        }
                    }
                }
            }

            std::this_thread::sleep_for(SAMPLE_INTERVAL);
            ++cycle;
        }
    } catch (const std::exception& e) {
        return 1;
    }

    return 0;
}
