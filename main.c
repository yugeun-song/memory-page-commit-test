#if defined(__linux__)
# define _DEFAULT_SOURCE
#endif

#if defined(_WIN64)
# include <windows.h>
# include <psapi.h>
# pragma comment(lib, "psapi.lib")
#elif defined(__linux__)
# include <fcntl.h>
# include <sys/mman.h>
# include <sys/resource.h>
# include <sys/types.h>
# include <unistd.h>
#else
# error "This project only supports x64 Windows and Linux."
#endif

#include <stdio.h>
#include <stdlib.h>

#define GIB_SIZE (1024.0 * 1024.0 * 1024.0)

static const size_t gs_resource_size = 1024ULL * 1024ULL * 1024ULL * 4ULL;

static void print_error(const char *const message)
{
#if defined(_WIN64)
    fprintf(stderr, "[ERROR] %s (Win32 Error Code: %lu)\n", message, GetLastError());
#elif defined(__linux__)
    perror(message);
#endif
}

static volatile unsigned char *allocate_pages(size_t size)
{
#if defined(_WIN64)
    return VirtualAlloc(NULL, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
#elif defined(__linux__)
    void *pages = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (pages == MAP_FAILED) {
        return NULL;
    }

    return pages;
#endif
}

static void free_pages(volatile unsigned char *const mut_pages, size_t size)
{
#if defined(_WIN64)
    VirtualFree((void *)mut_pages, 0, MEM_RELEASE);
#elif defined(__linux__)
    munmap((void *)mut_pages, size);
#endif
}

static void print_physical_memory_usage(const char *const label)
{
#if defined(_WIN64)
    PROCESS_MEMORY_COUNTERS counters;

    if (GetProcessMemoryInfo(GetCurrentProcess(), &counters, sizeof(counters)) != 0) {
        printf("[%s] Physical Memory Usage (Working Set): %.4lf GB\n", label,
               (double)counters.WorkingSetSize / GIB_SIZE);
    }
#elif defined(__linux__)
    int fd = open("/proc/self/statm", O_RDONLY);
    if (fd != -1) {
        char buffer[128];

        if (read(fd, buffer, sizeof(buffer)) > 0) {
            long rss_pages;

            if (sscanf(buffer, "%*s %ld", &rss_pages) == 1) {
                double rss_gib = (double)rss_pages * (double)sysconf(_SC_PAGESIZE) / GIB_SIZE;

                printf("[%s] Physical Memory Usage (RSS): %.4lf GB\n", label, rss_gib);
            }
        }
        close(fd);
    }
#endif /* defined(_WIN64) */
}

static void run_test(const char *const test_name, int should_touch_pages)
{
    printf("\n--- Starting Test: %s ---\n", test_name);
    print_physical_memory_usage("Before Allocation");

    volatile unsigned char *large_data = allocate_pages(gs_resource_size);
    if (large_data == NULL) {
        print_error(test_name);
        exit(EXIT_FAILURE);
    }

    printf("[INFO] Successfully allocated 4GB virtual memory.\n");
    print_physical_memory_usage("After Allocation (Before Touch)");

    if (should_touch_pages != 0) {
        printf("[INFO] Touching all pages to force commit...\n");
        for (size_t i = 0; i < gs_resource_size; i += 4096) {
            large_data[i] = 0xAB; /* dummy access to force page faults and commit physical memory */
        }
        print_physical_memory_usage("After Touching Pages");
    }

    printf("[INFO] Sleeping for 2 seconds...\n");
#if defined(_WIN64)
    Sleep(2000);
#else
    sleep(2);
#endif

    free_pages(large_data, gs_resource_size);
    large_data = NULL;
    printf("[INFO] Memory released.\n");
    print_physical_memory_usage("After Free");
    printf("--- Test Finished ---\n");
}

int main(void)
{
    printf("Memory Paging Strategy Test (Win: VirtualAlloc vs Linux: mmap)\n"
           "--- Starting Test: Lazy Allocation Test ---\n");
    print_physical_memory_usage("Before Allocation");

    volatile unsigned char *lazy_data = allocate_pages(gs_resource_size);
    if (lazy_data == NULL) {
        print_error("Lazy Allocation Test");
        return EXIT_FAILURE;
    }

    printf("[INFO] Successfully allocated 4GB virtual memory.\n");
    print_physical_memory_usage("After Allocation (Before Touch)");

    printf("[INFO] Sleeping for 2 seconds...\n");
#if defined(_WIN64)
    Sleep(2000);
#else
    sleep(2);
#endif

    free_pages(lazy_data, gs_resource_size);
    lazy_data = NULL;
    printf("[INFO] Memory released.\n");
    print_physical_memory_usage("After Free");
    printf("--- Test Finished ---\n\nPress Enter to start Force Commit Test...");

    getchar();

    run_test("Force Commit Test", 1);

    return 0;
}
