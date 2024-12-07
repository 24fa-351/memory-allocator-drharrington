# Memory Allocator

This project contains a custom memory manager with a series of tests to verify its functionality, including single-threaded, multi-threaded, and stress tests. The program can be compiled with either the system allocator or a custom allocator.

## Compilation

To compile the program, you can use the provided `Makefile` to build two versions:

1. **System Allocator**: Uses the system's `malloc`, `free`, and `realloc`.

    ```bash
    make test_system
    ```

2. **Custom Allocator**: Uses the custom memory management functions you implemented.

    ```bash
    make test_custom
    ```

Both versions will create a binary in the `bin/` directory.

## Running the Tests

Once compiled, you can run the binaries:

- For the system allocator:

    ```bash
    ./bin/test_system
    ```

- For the custom allocator:

    ```bash
    ./bin/test_custom
    ```

The tests will output log information about memory allocation, reallocation, and deallocation. The program performs the following tests:

1. **Single-threaded Tests**: Allocates and frees memory sequentially.
2. **Stress Test**: Allocates and frees memory in a loop to stress-test the allocator.
3. **Multi-threaded Tests**: Runs memory operations across multiple threads to check thread safety.

## Notes

- Memory manager initialization is done with 1 MB of memory.
- The program logs memory addresses and actions (allocate, free, realloc) for verification.

## License

This project is open-source and available for modification. Use it as you see fit!
