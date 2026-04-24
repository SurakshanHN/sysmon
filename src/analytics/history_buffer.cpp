#include "history_buffer.hpp"
#include <algorithm>

HistoryBuffer::HistoryBuffer(size_t capacity) : capacity_(capacity) {}

void HistoryBuffer::push(const CpuMetrics& cpu, const MemoryMetrics& mem) {
    buffer_.push_back({cpu.usage_percent, mem.used_kb, 
                       std::chrono::steady_clock::now()});
    if (buffer_.size() > capacity_) {
        buffer_.pop_front();
    }
}

std::vector<HistoryEntry> HistoryBuffer::get_last(size_t n) const {
    size_t count = std::min(n, buffer_.size());
    std::vector<HistoryEntry> result;
    result.reserve(count);
    

    auto start = buffer_.end() - count;
    std::copy(start, buffer_.end(), std::back_inserter(result));
    
    return result;
}

size_t HistoryBuffer::size() const {
    return buffer_.size();
}
