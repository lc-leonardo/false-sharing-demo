/*
 * False Sharing Demonstration Program
 * 
 * Inspired by:
 * - Dubois, Scheurich & Briggs (1990), "False Sharing and Spatial Locality 
 *   in Multiprocessor Caches"
 * 
 * This program demonstrates the performance impact of false sharing by having
 * multiple threads continuously update variables that reside in the same cache line.
 * 
 * FALSE SHARING OCCURS WHEN:
 * - Multiple threads access different variables
 * - These variables are located in the same cache line (typically 64 bytes)
 * - At least one thread is writing to its variable
 * - This causes unnecessary cache invalidation and coherence traffic
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

#define NUM_THREADS 4
#define ITERATIONS 100000000  // 100 million iterations per thread

/*
 * PROBLEMATIC DATA STRUCTURE:
 * All counters are packed together in memory, causing multiple counters
 * to share the same 64-byte cache line. When one thread updates its counter,
 * it invalidates the cache line for all other threads working on nearby counters.
 */
struct SharedData {
    int counter[NUM_THREADS];  // These will be in the same cache line!
};

// Global shared data - this creates the false sharing problem
struct SharedData shared_data = {0};

double get_time_ms() {
#ifdef _WIN32
    return (double)clock() / CLOCKS_PER_SEC * 1000.0;
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1000000.0;
#endif
}

int main() {
    printf("=== FALSE SHARING DEMONSTRATION ===\n");
    printf("Number of threads: %d\n", NUM_THREADS);
    printf("Iterations per thread: %d\n", ITERATIONS);
    printf("Total operations: %d\n", NUM_THREADS * ITERATIONS);
    
    // Print memory layout information
    printf("\nMemory Layout Analysis:\n");
    printf("sizeof(int): %zu bytes\n", sizeof(int));
    printf("Counter array size: %zu bytes\n", sizeof(shared_data.counter));
    printf("Typical cache line size: 64 bytes\n");
    
    for (int i = 0; i < NUM_THREADS; i++) {
        printf("counter[%d] address: %p (offset: %zu bytes)\n", 
               i, &shared_data.counter[i], 
               (char*)&shared_data.counter[i] - (char*)&shared_data.counter[0]);
    }
    
    printf("\nALL COUNTERS ARE IN THE SAME CACHE LINE!\n");
    printf("This will cause FALSE SHARING when multiple threads write simultaneously.\n\n");

    // Set number of threads
    omp_set_num_threads(NUM_THREADS);
    
    double start_time = get_time_ms();
    
    /*
     * FALSE SHARING IN ACTION:
     * Each thread works on its own counter[thread_id], but since all counters
     * are adjacent in memory, they share cache lines. Every write by one thread
     * invalidates the cache line for other threads, causing expensive cache
     * coherence traffic and memory stalls.
     */
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        int local_counter = 0;
        
        // Each thread performs its iterations
        for (int i = 0; i < ITERATIONS; i++) {
            local_counter++;
            // Periodically update the shared counter (this causes false sharing!)
            if (i % 1000 == 0) {
                shared_data.counter[thread_id] = local_counter;
            }
        }
        
        // Final update
        shared_data.counter[thread_id] = local_counter;
    }
    
    double end_time = get_time_ms();
    double execution_time = end_time - start_time;
    
    // Calculate and display results
    long total_operations = (long)NUM_THREADS * ITERATIONS;
    double operations_per_second = total_operations / (execution_time / 1000.0);
    
    printf("=== RESULTS ===\n");
    printf("Execution time: %.2f ms\n", execution_time);
    printf("Operations per second: %.0f\n", operations_per_second);
    printf("Throughput: %.2f million ops/sec\n", operations_per_second / 1000000.0);
    
    // Verify correctness
    printf("\nFinal counter values:\n");
    for (int i = 0; i < NUM_THREADS; i++) {
        printf("Thread %d: %d operations\n", i, shared_data.counter[i]);
    }
    
    printf("\n=== FALSE SHARING ANALYSIS ===\n");
    printf("Performance degradation is caused by:\n");
    printf("1. Cache line contention between threads\n");
    printf("2. Unnecessary cache invalidation traffic\n");
    printf("3. Memory stalls waiting for cache coherence\n");
    printf("4. Reduced effective memory bandwidth\n");
    printf("\nSolution: Use padding to separate variables into different cache lines!\n");
    
    return 0;
}