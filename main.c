#if defined(_WIN64)
#include <windows.h>
#include <psapi.h>
#pragma comment(lib, "psapi.lib")
#elif defined(__linux__)
#include <sys/resource.h>
#include <sys/mman.h>
#include <unistd.h>
#else
#error "This project only supports x64 Windows and Linux."
#endif

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#if defined(_WIN64)
#define BYTES_TO_GB (1024.0 * 1024.0 * 1024.0)
#elif defined(__linux__)
#define KB_TO_GB (1024.0 * 1024.0)
#endif

#if defined(_WIN64)
#define RAW_ALLOC(ptr, size) \
        do { \
            (ptr) = (volatile unsigned char*)VirtualAlloc(NULL, (size), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE); \
        } while (0)

#define RAW_FREE(ptr, size) \
        do { \
            VirtualFree((ptr), 0, MEM_RELEASE); \
        } while (0)

#define RAW_ALLOC_FAILED(ptr) ((ptr) == NULL)

#elif defined(__linux__)
#define RAW_ALLOC(ptr, size) \
        do { \
            (ptr) = (volatile unsigned char*)mmap(NULL, (size), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0); \
        } while (0)

#define RAW_FREE(ptr, size) \
        do { \
            munmap((ptr), (size)); \
        } while (0)

#define RAW_ALLOC_FAILED(ptr) ((ptr) == MAP_FAILED)
#endif

size_t g_ResourceSize = 1024ULL * 1024ULL * 1024ULL * 4ULL; /* 4GB */

void printPhysicalMemoryUsage(const char* str)
{
#if defined(_WIN64)
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)))
    {
        printf("[%s] Physical Memory Usage: %lf GB\n", str, (double)pmc.WorkingSetSize / BYTES_TO_GB);
    }
#elif defined(__linux__)
    struct rusage r_usage;
    if (getrusage(RUSAGE_SELF, &r_usage) == 0)
    {
        printf("[%s] Physical Memory Usage: %lf GB\n", str, (double)r_usage.ru_maxrss / KB_TO_GB);
    }
#endif
}

void doNotPageUpdate(void)
{
    printPhysicalMemoryUsage("doNotPageUpdate() - Before Malloc");

    volatile unsigned char* largeDatas = NULL;
    RAW_ALLOC(largeDatas, g_ResourceSize);
    if (RAW_ALLOC_FAILED(largeDatas))
    {
        perror("[ERROR] doNotPageUpdate");
#if defined(_MSC_VER)
        if (IsDebuggerPresent())
        {
            __debugbreak();
        }
#elif defined(__GNUC__)
        __builtin_trap();
#endif
        exit(EXIT_FAILURE);
    }

#if defined(_WIN64)
    Sleep(5000);
#elif defined(__linux__)
    sleep(5);
#endif

    printPhysicalMemoryUsage("doNotPageUpdate() - After Malloc");

    RAW_FREE((void*)largeDatas, g_ResourceSize);
    largeDatas = NULL;
}

void doPageUpdateForce(void)
{
    printPhysicalMemoryUsage("doPageUpdateForce() - Before Malloc");

    volatile unsigned char* largeDatas = NULL;
    RAW_ALLOC(largeDatas, g_ResourceSize);
    if (RAW_ALLOC_FAILED(largeDatas))
    {
        perror("[ERROR] doPageUpdateForce");
#if defined(_MSC_VER)
        if (IsDebuggerPresent())
        {
            __debugbreak();
        }
#elif defined(__GNUC__)
        __builtin_trap();
#endif
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < g_ResourceSize; ++i)
    {
        largeDatas[i] = 0xAB; /* dummy access to force page faults and commit physical memory */
    }

#if defined(_WIN64)
    Sleep(5000);
#elif defined(__linux__)
    sleep(5);
#endif

    printPhysicalMemoryUsage("doPageUpdateForce() - After Malloc");

    RAW_FREE((void*)largeDatas, g_ResourceSize);
    largeDatas = NULL;
}

int main(void)
{
    printf("--------------------------------------------------------------------------------\n");
    doNotPageUpdate();
    printf("--------------------------------------------------------------------------------\n");

    printf("press enter to continue test");
    (void)getchar(); /* void casting to prevent unused return value warning in MSVC (C6031) */
    
    printf("--------------------------------------------------------------------------------\n");
    doPageUpdateForce();
    printf("--------------------------------------------------------------------------------\n");
    
    return 0;
}