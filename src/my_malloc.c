#include "my_malloc.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MIN_BLOCK_SIZE 16  // Minimum block size for metadata
#define ALIGN8(size) (((size) + 7) & ~7)

// Struct for memory blocks
typedef struct Block {
    size_t size;         // Size of the block
    int is_free;         // 1 if free, 0 if allocated
    struct Block* next;  // Next block in the heap (for merging)
} Block;

// Global variables
static void* memory_pool = NULL;
static size_t memory_pool_size = 0;
static pthread_mutex_t memory_lock = PTHREAD_MUTEX_INITIALIZER;
static int logging_enabled = 0;

// Min-heap for free blocks
static Block** free_heap = NULL;
static size_t heap_size = 0;
static size_t heap_capacity = 0;

// Function prototypes
void heap_push(Block* block);
Block* heap_pop();
void heapify_up(size_t index);
void heapify_down(size_t index);
void split_block(Block* block, size_t size);
void merge_adjacent_blocks(Block* block);
Block* request_memory_from_os(size_t size);

// Logging
void enable_logging(int enable) { logging_enabled = enable; }

// Initialize memory manager
void init_memory_manager(size_t total_memory) {
    pthread_mutex_lock(&memory_lock);

    memory_pool = sbrk(total_memory);
    if (memory_pool == (void*)-1) {
        fprintf(stderr, "Failed to allocate memory pool.\n");
        pthread_mutex_unlock(&memory_lock);
        exit(EXIT_FAILURE);
    }

    memory_pool_size = total_memory;
    Block* initial_block = (Block*)memory_pool;
    initial_block->size = total_memory - sizeof(Block);
    initial_block->is_free = 1;
    initial_block->next = NULL;

    // Initialize the heap
    heap_capacity = 128;
    free_heap = (Block**)malloc(heap_capacity * sizeof(Block*));
    heap_size = 0;

    heap_push(initial_block);

    if (logging_enabled)
        fprintf(stderr, "Memory manager initialized with %zu bytes\n",
                total_memory);

    pthread_mutex_unlock(&memory_lock);
}

// Destroy memory manager
void destroy_memory_manager() {
    pthread_mutex_lock(&memory_lock);

    memory_pool = NULL;
    memory_pool_size = 0;
    free(free_heap);
    free_heap = NULL;
    heap_size = 0;
    heap_capacity = 0;

    if (logging_enabled) fprintf(stderr, "Memory manager destroyed\n");

    pthread_mutex_unlock(&memory_lock);
}

// Malloc implementation
void* my_malloc(size_t size) {
    if (size <= 0) return NULL;
    size = ALIGN8(size);

    pthread_mutex_lock(&memory_lock);

    Block* block = NULL;
    while (heap_size > 0) {
        block = heap_pop();
        if (block->size >= size && block->is_free) break;
        block = NULL;
    }

    if (!block) {
        block = request_memory_from_os(size);
        if (!block) {
            pthread_mutex_unlock(&memory_lock);
            return NULL;
        }
    }

    block->is_free = 0;
    split_block(block, size);

    pthread_mutex_unlock(&memory_lock);

    memset((void*)(block + 1), 0, size);

    if (logging_enabled)
        fprintf(stderr, "Allocated %zu bytes at %p\n", size,
                (void*)(block + 1));

    return (void*)(block + 1);
}

// Free implementation
void my_free(void* ptr) {
    if (!ptr) return;

    pthread_mutex_lock(&memory_lock);

    Block* block = (Block*)ptr - 1;
    block->is_free = 1;

    merge_adjacent_blocks(block);
    heap_push(block);

    pthread_mutex_unlock(&memory_lock);

    if (logging_enabled) fprintf(stderr, "Freed memory at %p\n", ptr);
}

// Realloc implementation
void* my_realloc(void* ptr, size_t size) {
    if (!ptr) return my_malloc(size);
    if (size == 0) {
        my_free(ptr);
        return NULL;
    }

    pthread_mutex_lock(&memory_lock);

    Block* block = (Block*)ptr - 1;
    if (block->size >= size) {
        pthread_mutex_unlock(&memory_lock);
        return ptr;
    }

    pthread_mutex_unlock(&memory_lock);

    void* new_ptr = my_malloc(size);
    if (new_ptr) {
        memcpy(new_ptr, ptr, block->size);
        my_free(ptr);
    }

    return new_ptr;
}

// Request memory from OS
Block* request_memory_from_os(size_t size) {
    void* mem = sbrk(size + sizeof(Block));
    if (mem == (void*)-1) return NULL;

    Block* block = (Block*)mem;
    block->size = size;
    block->is_free = 0;
    block->next = NULL;

    return block;
}

// Split a block into two if it's large enough
void split_block(Block* block, size_t size) {
    if (block->size >= size + sizeof(Block) + MIN_BLOCK_SIZE) {
        Block* new_block = (Block*)((char*)(block + 1) + size);
        new_block->size = block->size - size - sizeof(Block);
        new_block->is_free = 1;
        new_block->next = block->next;

        block->size = size;
        block->next = new_block;

        heap_push(new_block);
    }
}

// Merge adjacent blocks
void merge_adjacent_blocks(Block* block) {
    Block* current = block;
    while (current->next && current->next->is_free) {
        current->size += sizeof(Block) + current->next->size;
        current->next = current->next->next;
    }
}

// Min-heap operations
void heap_push(Block* block) {
    if (heap_size == heap_capacity) {
        heap_capacity *= 2;
        free_heap = (Block**)realloc(free_heap, heap_capacity * sizeof(Block*));
    }
    free_heap[heap_size] = block;
    heapify_up(heap_size++);
}

Block* heap_pop() {
    if (heap_size == 0) return NULL;

    Block* root = free_heap[0];
    free_heap[0] = free_heap[--heap_size];
    heapify_down(0);
    return root;
}

void heapify_up(size_t index) {
    size_t parent = (index - 1) / 2;
    while (index > 0 && free_heap[index]->size < free_heap[parent]->size) {
        Block* temp = free_heap[index];
        free_heap[index] = free_heap[parent];
        free_heap[parent] = temp;
        index = parent;
        parent = (index - 1) / 2;
    }
}

void heapify_down(size_t index) {
    size_t left = 2 * index + 1;
    size_t right = 2 * index + 2;
    size_t smallest = index;

    if (left < heap_size && free_heap[left]->size < free_heap[smallest]->size)
        smallest = left;
    if (right < heap_size && free_heap[right]->size < free_heap[smallest]->size)
        smallest = right;

    if (smallest != index) {
        Block* temp = free_heap[index];
        free_heap[index] = free_heap[smallest];
        free_heap[smallest] = temp;
        heapify_down(smallest);
    }
}
