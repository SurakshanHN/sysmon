cmake_minimum_required(VERSION 3.15)
project(sysmon)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Curses REQUIRED)

add_executable(sysmon
    src/main.cpp
    src/collector/cpu_collector.cpp
    src/collector/memory_collector.cpp
    src/collector/process_collector.cpp
    src/engine/cpu_engine.cpp
    src/engine/process_engine.cpp
    src/ui/tui.cpp
    src/analytics/history_buffer.cpp
    src/analytics/anomaly_detector.cpp
    src/logger/logger.cpp
)

target_link_libraries(sysmon ${CURSES_LIBRARIES})
target_include_directories(sysmon PRIVATE ${CURSES_INCLUDE_DIR})
