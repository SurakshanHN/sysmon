#ifndef SYSMON_TUI_HPP
#define SYSMON_TUI_HPP

#include "../engine/cpu_engine.hpp"
#include "../engine/process_engine.hpp"
#include "../collector/memory_collector.hpp"
#include <vector>
#include <string>

#include "../analytics/history_buffer.hpp"
#include "../analytics/anomaly_detector.hpp"

class TUI {
public:
    TUI();                   
    ~TUI();     


    void draw(
        const CpuMetrics& cpu,
        const MemoryMetrics& mem,
        const std::vector<ProcessMetrics>& processes,
        size_t total_process_count,
        SortMode sort_mode,
        int cycle,
        int selected_index,
        bool confirm_mode,
        const std::vector<HistoryEntry>& history,
        const std::vector<Anomaly>& anomalies
    );


    int poll_key();

private:
    void draw_header(const CpuMetrics& cpu, const MemoryMetrics& mem, int cycle);
    void draw_process_table(const std::vector<ProcessMetrics>& processes,
                            size_t total_count, SortMode sort_mode,
                            int selected_index);
    void draw_footer(SortMode sort_mode, bool confirm_mode);
    void draw_confirm_dialog(const ProcessMetrics& proc);

    std::string make_sparkline(const std::vector<HistoryEntry>& history, size_t width);
    std::string make_bar(double percent, int bar_width);
    void safe_mvprintw(int row, int col, int max_col, const char* fmt, ...);

    int term_rows_;
    int term_cols_;
};

#endif
