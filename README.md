# sysmon

*A lightweight Linux system monitor written in C++, reading directly from /proc*

`C++17` · `CMake` · `Linux` · `ncurses`

## Preview

```
SYSMON | Cycle: 42
CPU  : [################-----------] 23.4%
MEM  : [##########------------------] 30.1%
Status: OK

PROCESSES (20 shown of 312 total) [Sorted by: CPU%]
PID       NAME                 CPU%      MEM(MB)
-------   ------------------   -------   -------
18934     antigravity          5.23      558.2
1536      chrome               3.65      628.1
882       Hyprland             0.62      124.1

[↑↓] navigate   [k] kill   [m] sort   [q] quit
```

## Features

- Real-time CPU usage via `/proc/stat` delta calculation
- Real-time memory monitoring via `/proc/meminfo` (correct formula: Total − Free − Buffers − Cached − SReclaimable)
- Per-process monitoring: PID, name, CPU %, RSS memory — sourced from `/proc/[pid]/stat` and `/proc/[pid]/status`
- Sorting by CPU or Memory with live toggle (`m` key)
- Flicker-free ncurses TUI with color coding and ASCII bar graphs
- Arrow key navigation through process list
- Process termination via `SIGTERM` with confirmation dialog
- CPU sparkline history graph (last 60 seconds, 8-level UTF-8 block chars)
- Anomaly detection: CPU spike (3 consecutive cycles > 85%) and memory leak trend (5 monotonically increasing cycles)
- Persistent logging to `sysmon.log` with timestamps and anomaly records

## How It Works

**CPU calculation:** sysmon uses the two-snapshot delta method to calculate usage. It reads total and idle jiffies from `/proc/stat` at two points in time. Comparing these snapshots ensures accuracy, as a single reading only provides an average since boot.

**Process CPU %:** per-process CPU usage is calculated as `(delta utime + stime) / delta global jiffies`. Process state is cached in an `unordered_map` keyed by PID to maintain tracking across collection cycles. This ensures that only active and identifiable processes are monitored.

**Memory calculation:** sysmon uses the kernel-recommended formula for calculating "used" memory. It subtracts `Free`, `Buffers`, `Cached`, and `SReclaimable` values from `MemTotal`. Relying solely on `Total − Free` provides a misleadingly high usage value by including disk-buffered memory.

## Prerequisites

**Debian / Ubuntu:**
```bash
sudo apt update
sudo apt install build-essential cmake libncurses-dev
```

**Arch Linux:**
```bash
sudo pacman -S base-devel cmake ncurses
```

**Fedora / RHEL:**
```bash
sudo dnf install gcc-c++ cmake ncurses-devel
```

**Minimum Versions:**
- GCC 8+ or Clang 7+ (C++17 required)
- CMake 3.15+
- ncurses 6.x

## Installation & Build

```bash
# 1. Clone the repository
git clone https://github.com/yourusername/sysmon.git
cd sysmon

# 2. Create build directory
mkdir build && cd build

# 3. Configure with CMake
cmake ..

# 4. Build (replace 4 with your core count)
make -j4

# 5. Run
./sysmon
```

**Release build (optimized):**
```bash
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j4
```

## Usage & Controls

| Key | Action |
|-----|--------|
| `↑` / `↓` | Navigate process list |
| `m` | Toggle sort: CPU% ↔ Memory |
| `k` | Kill selected process (shows confirmation) |
| `y` | Confirm kill |
| `n` | Cancel kill |
| `q` | Quit |

The monitor updates every 1 second and logs data to `sysmon.log` in the current directory from which the application is launched.

## Log File Format

The log entries are machine-parseable and include timestamps:
```
[2026-04-23 17:12:52] cycle=0 cpu=0.00% mem_used_mb=5595 mem_total_mb=15673 anomalies=0
[2026-04-23 17:12:53] cycle=1 cpu=4.27% mem_used_mb=5600 mem_total_mb=15673 anomalies=0
[2026-04-23 17:14:01] cycle=69 cpu=91.20% mem_used_mb=5841 mem_total_mb=15673 anomalies=1 ANOMALY:CPU_SPIKE(91.2%)
```
The log file is created in whichever directory you run `./sysmon` from.

## Anomaly Detection

- **CPU Spike:** triggers after system CPU usage stays above 85% for 3 or more consecutive cycles. The detection resets immediately if usage drops below the threshold. Alerts are displayed in bold red in the TUI header.
- **Memory Leak Trend:** triggers when used memory increases monotonically for 5 consecutive 1-second cycles. This heuristic identifies potential allocation leaks rather than static high memory consumption.

## Architecture Overview

```
/proc filesystem
      │
      ▼
┌─────────────┐
│  Collectors │  cpu_collector, memory_collector, process_collector
│             │  — raw data only, no computation
└──────┬──────┘
       │
       ▼
┌─────────────┐
│   Engines   │  cpu_engine, process_engine
│             │  — pure computation, no I/O
└──────┬──────┘
       │
       ├──────────────────────┐
       ▼                      ▼
┌─────────────┐        ┌─────────────┐
│  Analytics  │        │     TUI     │  ncurses rendering
│  history,   │        │             │
│  anomalies  │        └─────────────┘
└─────────────┘
       │
       ▼
┌─────────────┐
│   Logger    │  sysmon.log
└─────────────┘
```

## Known Limitations

- Linux only — relies exclusively on the `/proc` filesystem and is not portable to macOS or BSD.
- Single-threaded — collection and rendering share the same execution thread, which may cause minor jitter at extremely high process counts (1000+).
- Permission constraints — reading detailed stats for some kernel threads requires root privileges.
- Safe termination — the kill command sends `SIGTERM` for graceful exit and does not currently implement a `SIGKILL` fallback.

## License

MIT License — see LICENSE for details
