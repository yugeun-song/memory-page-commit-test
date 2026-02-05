#if defined(_WIN64)
    #include <windows.h>
    #include <psapi.h>
    #pragma comment(lib, "psapi.lib")
#elif defined(__linux__)
    #define _DEFAULT_SOURCE
    #include <sys/resource.h>
    #include <sys/mman.h>
    #include <unistd.h>
    #include <sys/types.h>
    #include <fcntl.h>
#else
    #error "This project only supports x64 Windows and Linux."
#endif

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#define GIB_SIZE (1024.0 * 1024.0 * 1024.0)

#if defined(_WIN64)
    #define RAW_ALLOC(ptr, size) \
        do { \
            (ptr) = (volatile unsigned char*)VirtualAlloc(NULL, (size), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE); \
        } while (0)

    #define RAW_FREE(ptr, size) \
        do { \
            VirtualFree((void*)(ptr), 0, MEM_RELEASE); \
        } while (0)

    #define RAW_ALLOC_FAILED(ptr) ((ptr) == NULL)

    void PrintError(const char* message)
    {
        fprintf(stderr, "[ERROR] %s (Win32 Error Code: %lu)\n", message, GetLastError());
    }

#elif defined(__linux__)
    #define RAW_ALLOC(ptr, size) \
        do { \
            (ptr) = (volatile unsigned char*)mmap(NULL, (size), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0); \
        } while (0)

    #define RAW_FREE(ptr, size) \
        do { \
            munmap((void*)(ptr), (size)); \
        } while (0)

    #define RAW_ALLOC_FAILED(ptr) ((ptr) == MAP_FAILED)

    void PrintError(const char* message)
    {
        perror(message);
    }
#endif

size_t g_ResourceSize = 1024ULL * 1024ULL * 1024ULL * 4ULL;

void PrintPhysicalMemoryUsage(const char* label)
{
#if defined(_WIN64)
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)))
    {
        printf("[%s] Physical Memory Usage (Working Set): %.4lf GB\n", label, (double)pmc.WorkingSetSize / GIB_SIZE);
    }
#elif defined(__linux__)
    int fd = open("/proc/self/statm", O_RDONLY);
    if (fd != -1)
    {
        char buffer[128];
        if (read(fd, buffer, sizeof(buffer)) > 0)
        {
            long rssPages;
            if (sscanf(buffer, "%*s %ld", &rssPages) == 1)
            {
                double rssGb = (double)rssPages * sysconf(_SC_PAGESIZE) / GIB_SIZE;
                printf("[%s] Physical Memory Usage (RSS): %.4lf GB\n", label, rssGb);
            }
        }
        close(fd);
    }
#endif
}

void RunTest(const char* testName, int forceTouch)
{
    printf("\n--- Starting Test: %s ---\n", testName);
    PrintPhysicalMemoryUsage("Before Allocation");

    volatile unsigned char* largeData = NULL;
    RAW_ALLOC(largeData, g_ResourceSize);

    if (RAW_ALLOC_FAILED(largeData))
    {
        PrintError(testName);
        exit(EXIT_FAILURE);
    }

    printf("[INFO] Successfully allocated 4GB virtual memory.\n");
    PrintPhysicalMemoryUsage("After Allocation (Before Touch)");

    if (forceTouch)
    {
        printf("[INFO] Touching all pages to force commit...\n");
        for (size_t i = 0; i < g_ResourceSize; i += 4096)
        {
            largeData[i] = 0xAB;
        }
        PrintPhysicalMemoryUsage("After Touching Pages");
    }

    printf("[INFO] Sleeping for 2 seconds...\n");
#if defined(_WIN64)
    Sleep(2000);
#else
    sleep(2);
#endif

    RAW_FREE(largeData, g_ResourceSize);
    printf("[INFO] Memory released.\n");
    PrintPhysicalMemoryUsage("After Free");
    printf("--- Test Finished ---\n");
}

int main(void)
{
    printf("Memory Paging Strategy Test (Win: VirtualAlloc vs Linux: mmap)\n--- Starting Test: Lazy Allocation Test ---\n");
    PrintPhysicalMemoryUsage("Before Allocation");

    volatile unsigned char* lazyData = NULL;
    RAW_ALLOC(lazyData, g_ResourceSize);

    if (RAW_ALLOC_FAILED(lazyData))
    {
        PrintError("Lazy Allocation Test");
        exit(EXIT_FAILURE);
    }

    printf("[INFO] Successfully allocated 4GB virtual memory.\n");
    PrintPhysicalMemoryUsage("After Allocation (Before Touch)");

    printf("[INFO] Sleeping for 2 seconds...\n");
#if defined(_WIN64)
    Sleep(2000);
#else
    sleep(2);
#endif

    RAW_FREE(lazyData, g_ResourceSize);
    printf("[INFO] Memory released.\n");
    PrintPhysicalMemoryUsage("After Free");
    printf("--- Test Finished ---\n\nPress Enter to start Force Commit Test...");

    (void)getchar();

    RunTest("Force Commit Test", 1);

    return 0;
}
