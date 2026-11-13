# Memory Page Commit Test

This project demonstrates the differences in memory allocation and paging strategies between Windows (VirtualAlloc) and Linux (mmap).

## Supported Platforms
- **Windows**: x64 Architecture, MSVC (Microsoft Visual C++)
- **Linux**: x64 Architecture, GCC or Clang

## Features
- **Lazy Allocation Test**: Allocates 4GB of virtual memory and monitors physical memory usage (RSS/Working Set) before access.
- **Force Commit Test**: Accesses each memory page to trigger page faults and force physical memory commitment.

## How to Build and Run

### Windows
1. Open `MemoryPageCommitTest.slnx` or `MemoryPageCommitTest.vcxproj` in Visual Studio.
2. Set the configuration to **Debug or Release** and platform to **x64**.
3. Build and run the solution.

### Linux
1. **Build**: `make` builds the debug program and the same program with AddressSanitizer, on all cores. `make release` builds the optimized one, and `make CC=clang` uses Clang instead of GCC:
   ```bash
   make
   ```
2. **Binary Location**: Each mode and compiler builds into `bin/<arch>-linux-<cc>-<mode>/`:
   ```bash
   ./bin/x86_64-linux-gcc-debug/MemoryPageCommitTest
   ```
3. **Index**: `make index` writes `compile_commands.json`, `tags` and `cscope.out`; `make clean-index` removes them.
4. **Clean**: `make clean` removes `bin/` and `build/`; `make distclean` also removes the index, tool outputs such as `gmon.out` and `.cache/`.
   ```bash
   make clean
   ```
