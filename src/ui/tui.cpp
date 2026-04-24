#include "tui.hpp"
#include <ncurses.h>
#include <algorithm>
#include <cstdarg>
#include <iomanip>
#include <sstream>

TUI::TUI() {
    initscr();
    cbreak();           
    noecho();         
    curs_set(0);    
    keypad(stdscr, TRUE);   
    nodelay(stdscr, TRUE);
    start_color();
    use_default_colors();  

    init_pair(1, COLOR_GREEN,  -1);  
    init_pair(2, COLOR_CYAN,   -1);  
    init_pair(3, COLOR_YELLOW, -1);  
    init_pair(4, COLOR_RED,    -1);  
    init_pair(5, COLOR_WHITE,  -1); 

    getmaxyx(stdscr, term_rows_, term_cols_);
}

TUI::~TUI() {
    endwin();
}

void TUI::draw(
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
) {
    erase();
    getmaxyx(stdscr, term_rows_, term_cols_);

    draw_header(cpu, mem, cycle);
    
    
    if (anomalies.empty()) {
        attron(COLOR_PAIR(1));
        mvprintw(3, 0, "Status: OK");
        attroff(COLOR_PAIR(1));
    } else {
        attron(COLOR_PAIR(4) | A_BOLD);
        std::string status = "Status: !! ";
        for (const auto& a : anomalies) status += a.message + " !! ";
        mvprintw(3, 0, "%s", status.c_str());
        attroff(COLOR_PAIR(4) | A_BOLD);
    }

    draw_process_table(processes, total_process_count, sort_mode, selected_index);
    draw_footer(sort_mode, confirm_mode);

    if (confirm_mode && selected_index >= 0 && selected_index < (int)processes.size()) {
        draw_confirm_dialog(processes[selected_index]);
    }

    refresh(); 
}

int TUI::poll_key() {
    return getch();
}

void TUI::draw_header(const CpuMetrics& cpu, const MemoryMetrics& mem, int cycle) {
    int bar_width = std::max(10, term_cols_ - 30);
    
    attron(A_BOLD);
    mvprintw(0, 0, "SYSMON | Cycle: %d", cycle);
    attroff(A_BOLD);

    mvprintw(1, 0, "CPU  : [");
    attron(COLOR_PAIR(1));
    std::string cpu_bar = make_bar(cpu.usage_percent, bar_width);
    printw("%s", cpu_bar.c_str());
    attroff(COLOR_PAIR(1));
    printw("] %.1f%%", cpu.usage_percent);

    // Memory Bar
    long long used_mb = mem.used_kb / 1024;
    long long total_mb = mem.total_kb / 1024;
    mvprintw(2, 0, "MEM  : [");
    attron(COLOR_PAIR(2));
    std::string mem_bar = make_bar(mem.usage_percent, bar_width);
    printw("%s", mem_bar.c_str());
    attroff(COLOR_PAIR(2));
    printw("] %lld/%lld MB (%.1f%%)", used_mb, total_mb, mem.usage_percent);

    mvprintw(4, 0, "--------------------------------------------------------------------------------");
}



void TUI::draw_process_table(const std::vector<ProcessMetrics>& processes,
                            size_t total_count, SortMode sort_mode,
                            int selected_index) {
    attron(COLOR_PAIR(3) | A_BOLD);
    mvprintw(3, 0, "PROCESSES (%zu shown of %zu total) [Sorted by: %s]", 
             processes.size(), total_count, (sort_mode == SortMode::CPU ? "CPU%" : "MEM"));
    
    mvprintw(4, 0, "%-10s %-20s %-10s %-10s", "PID", "NAME", "CPU%", "MEM(MB)");
    attroff(COLOR_PAIR(3) | A_BOLD);

    int start_row = 5;
    int max_rows = term_rows_ - 2; // Subtract footer rows

    for (int i = 0; i < static_cast<int>(processes.size()) && (start_row + i) < max_rows; ++i) {
        const auto& pm = processes[i];
        bool is_selected = (i == selected_index);
        bool is_high_cpu = (pm.cpu_percent > 50.0);

        if (is_selected) {
            attron(A_REVERSE);
        } else if (is_high_cpu) {
            attron(COLOR_PAIR(4) | A_BOLD);
        } else {
            attron(COLOR_PAIR(5));
        }

        std::string name = pm.name;
        if (name.size() > 19) name = name.substr(0, 16) + "...";

        mvprintw(start_row + i, 0, "%-10d %-20s %-10.2f %-10.1f", 
                 pm.pid, name.c_str(), pm.cpu_percent, pm.memory_kb / 1024.0);
        
        if (is_selected) {
            attroff(A_REVERSE);
        } else if (is_high_cpu) {
            attroff(COLOR_PAIR(4) | A_BOLD);
        } else {
            attroff(COLOR_PAIR(5));
        }
    }
}

void TUI::draw_footer(SortMode sort_mode, bool confirm_mode) {
    if (confirm_mode) return; 

    attron(A_REVERSE);
    mvprintw(term_rows_ - 1, 0, "[UP/DN] navigate [k] kill [m] sort toggle [q] quit | Sorted by: %s ", 
             (sort_mode == SortMode::CPU ? "CPU%" : "MEM"));
    attroff(A_REVERSE);
}

void TUI::draw_confirm_dialog(const ProcessMetrics& proc) {
    int dialog_width  = 40;
    int dialog_height = 9;
    int start_row = (term_rows_ - dialog_height) / 2;
    int start_col = (term_cols_ - dialog_width)  / 2;

    attron(COLOR_PAIR(4) | A_BOLD); 
    for (int r = start_row; r < start_row + dialog_height; r++) {
        mvhline(r, start_col, ' ', dialog_width);  
    }

    // Draw border
    mvprintw(start_row,                      start_col, "+%s+", std::string(dialog_width - 2, '-').c_str());
    mvprintw(start_row + dialog_height - 1,  start_col, "+%s+", std::string(dialog_width - 2, '-').c_str());
    for (int r = start_row + 1; r < start_row + dialog_height - 1; r++) {
        mvprintw(r, start_col, "│");
        mvprintw(r, start_col + dialog_width - 1, "│");
    }

    // Dialog content
    mvprintw(start_row + 2, start_col + 4, "!! KILL PROCESS !!");
    mvprintw(start_row + 4, start_col + 4, "PID  : %d", proc.pid);
    
    std::string name = proc.name;
    if (name.size() > 25) name = name.substr(0, 22) + "...";
    mvprintw(start_row + 5, start_col + 4, "Name : %s", name.c_str());
    
    mvprintw(start_row + 7, start_col + 4, "[y] confirm   [n] cancel");
    attroff(COLOR_PAIR(4) | A_BOLD);
}

std::string TUI::make_bar(double percent, int bar_width) {
    int filled = static_cast<int>((percent / 100.0) * bar_width);
    filled = std::clamp(filled, 0, bar_width);
    return std::string(filled, '#') + std::string(bar_width - filled, '-');
}

void TUI::safe_mvprintw(int row, int col, int max_col, const char* fmt, ...) {
    if (row < 0 || row >= term_rows_ || col < 0 || col >= term_cols_) return;
    
    char buffer[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    std::string s(buffer);
    if (col + (int)s.size() > term_cols_) {
        s = s.substr(0, term_cols_ - col);
    }
    if (max_col > 0 && (int)s.size() > max_col) {
        s = s.substr(0, max_col);
    }

    mvprintw(row, col, "%s", s.c_str());
}
