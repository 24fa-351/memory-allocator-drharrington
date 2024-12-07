#ifdef USE_SYSTEM_ALLOCATOR
#include <stdio.h>   // Needed for fprintf and stderr
#include <stdlib.h>  // Use the system allocator
#define my_malloc malloc
#define my_free free
#define my_realloc realloc

// Define init/destroy as no-ops for the system allocator
#define init_memory_manager(size) \
    ((void)0)  // Accept the argument but do nothing
#define destroy_memory_manager() ((void)0)

#else
#include "my_malloc.h"  // Use the custom allocator
#endif

#include <pthread.h>  // For threading in multithreaded tests
#include <stdio.h>    // For fprintf and stderr
#include <stdlib.h>   // For system malloc/free in the system allocator
#include <string.h>   // For memcpy, if needed in tests

#define TEST_POOL_SIZE 1024 * 1024  // 1 MB memory pool

void* thread_test_function(void* arg) {
    int thread_id = *(int*)arg;

    // Allocate memory
    size_t alloc_size = 1024;  // 1 KB
    void* ptr = my_malloc(alloc_size);
    if (ptr == NULL) {
        printf("Thread %d: Allocation failed\n", thread_id);
        return NULL;
    }

    printf("Thread %d: Allocated %zu bytes at %p\n", thread_id, alloc_size,
           ptr);

    // Write data
    memset(ptr, thread_id, alloc_size);

    // Reallocate memory
    size_t realloc_size = 2048;  // 2 KB
    void* new_ptr = my_realloc(ptr, realloc_size);
    if (new_ptr == NULL) {
        printf("Thread %d: Reallocation failed\n", thread_id);
        return NULL;
    }

    printf("Thread %d: Reallocated to %zu bytes at %p\n", thread_id,
           realloc_size, new_ptr);

    // Free memory
    my_free(new_ptr);
    printf("Thread %d: Freed memory at %p\n", thread_id, new_ptr);

    return NULL;
}

int main() {
    // Initialize memory manager
    init_memory_manager(TEST_POOL_SIZE);

    printf("Memory manager initialized with %d bytes.\n", TEST_POOL_SIZE);

    // Single-threaded tests
    printf("\n[Single-threaded tests]\n");
    void* block1 = my_malloc(256);  // Small allocation
    printf("Allocated 256 bytes at %p\n", block1);

    void* block2 = my_malloc(1024);  // Medium allocation
    printf("Allocated 1024 bytes at %p\n", block2);

    void* block3 = my_malloc(65536);  // Large allocation
    printf("Allocated 65536 bytes at %p\n", block3);

    my_free(block2);  // Free medium allocation
    printf("Freed block at %p\n", block2);

    // Reallocate block1
    void* new_block1 = my_realloc(block1, 512);  // Resize to 512 bytes
    printf("Reallocated block at %p to 512 bytes at %p\n", block1, new_block1);

    // Stress test
    printf("\n[Stress test]\n");
    for (int i = 0; i < 10; ++i) {
        void* stress_block = my_malloc(8192);
        printf("Stress test allocated 8192 bytes at %p\n", stress_block);
        my_free(stress_block);
        printf("Stress test freed block at %p\n", stress_block);
    }

    // Multi-threaded tests
    printf("\n[Multi-threaded tests]\n");
    pthread_t threads[4];
    int thread_ids[4];
    for (int i = 0; i < 4; ++i) {
        thread_ids[i] = i + 1;
        pthread_create(&threads[i], NULL, thread_test_function, &thread_ids[i]);
    }

    for (int i = 0; i < 4; ++i) {
        pthread_join(threads[i], NULL);
    }

    // Clean up
    destroy_memory_manager();

    printf("Memory manager destroyed.\n");

    return 0;
}
