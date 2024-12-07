#ifndef MY_MALLOC_H
#define MY_MALLOC_H

#include <stddef.h>

// Memory manager functions
void* my_malloc(size_t size);
void my_free(void* ptr);
void* my_realloc(void* ptr, size_t size);

// Memory manager initialization and destruction
void init_memory_manager(size_t total_memory);
void destroy_memory_manager(void);

// Utility for logging
void enable_logging(int enable);

#endif
