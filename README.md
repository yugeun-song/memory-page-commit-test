# Memory Page Commit Test

This project demonstrates the differences in memory allocation and paging strategies between Windows (VirtualAlloc) and Linux (mmap).

## Supported Platforms
- **Windows**: x64 Architecture, MSVC (Microsoft Visual C++)
- **Linux**: x64 Architecture, GCC

## Features
- **Lazy Allocation Test**: Allocates 4GB of virtual memory and monitors physical memory usage (RSS/Working Set) before access.
- **Force Commit Test**: Accesses each memory page to trigger page faults and force physical memory commitment.

## How to Build and Run

### Windows
1. Open `MemoryPageCommitTest.slnx` or `MemoryPageCommitTest.vcxproj` in Visual Studio.
2. Set the configuration to **Debug or Release** and platform to **x64**.
3. Build and run the solution.

### Linux
1. **Build**: Run the following command in the project root:
   ```bash
   make all
   ```
2. **Binary Location**: The executable is generated in the `x64-linux` directory:
   ```bash
   ./x64-linux/MemoryPageCommitTest_linux
   ```
3. **Clean**: To remove the `x64-linux` directory and the compiled binary:
   ```bash
   make clean
   ```
