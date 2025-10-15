/*
 * False Sharing Comparison Program
 * 
 * This program runs both the false sharing demonstration and the optimized
 * version side-by-side to clearly show the performance difference achieved
 * through compile-time data transformations.
 * 
 * References:
 * - Dubois, Scheurich & Briggs (1990), "False Sharing and Spatial Locality in Multiprocessor Caches"
 * - Jeremiassen & Eggers (1993), "Reducing False Sharing on Shared Memory Multiprocessors through Compile-Time Data Transformations"
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include <string.h>

#define NUM_THREADS 4
#define ITERATIONS 50000000   // 50 million iterations per thread (reduced for comparison)
#define CACHE_LINE_SIZE 64
#define NUM_RUNS 3           // Run multiple times for more reliable measurements

// Problematic structure (false sharing)
struct SharedData {
    int counter[NUM_THREADS];
};

// Optimized structure (padded)
struct PaddedCounter {
    int counter;
    char padding[CACHE_LINE_SIZE - sizeof(int)];
} __attribute__((aligned(CACHE_LINE_SIZE)));

double get_time_ms() {
#ifdef _WIN32
    return (double)clock() / CLOCKS_PER_SEC * 1000.0;
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1000000.0;
#endif
}

double run_false_sharing_test() {
    struct SharedData shared_data;
    memset(&shared_data, 0, sizeof(shared_data));
    
    omp_set_num_threads(NUM_THREADS);
    
    double start_time = get_time_ms();
    
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        int local_counter = 0;
        
        for (int i = 0; i < ITERATIONS; i++) {
            local_counter++;
            if (i % 1000 == 0) {
                shared_data.counter[thread_id] = local_counter;
            }
        }
        
        shared_data.counter[thread_id] = local_counter;
    }
    
    double end_time = get_time_ms();
    return end_time - start_time;
}

double run_optimized_test() {
    struct PaddedCounter padded_counters[NUM_THREADS];
    memset(padded_counters, 0, sizeof(padded_counters));
    
    omp_set_num_threads(NUM_THREADS);
    
    double start_time = get_time_ms();
    
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        int local_counter = 0;
        
        for (int i = 0; i < ITERATIONS; i++) {
            local_counter++;
            if (i % 1000 == 0) {
                padded_counters[thread_id].counter = local_counter;
            }
        }
        
        padded_counters[thread_id].counter = local_counter;
    }
    
    double end_time = get_time_ms();
    return end_time - start_time;
}

void print_system_info() {
    printf("=== SYSTEM INFORMATION ===\n");
    printf("Number of processors: %d\n", omp_get_num_procs());
    printf("Max threads: %d\n", omp_get_max_threads());
    printf("Test threads: %d\n", NUM_THREADS);
    printf("Iterations per thread: %d\n", ITERATIONS);
    printf("Total operations per run: %d\n", NUM_THREADS * ITERATIONS);
    printf("Cache line size (assumed): %d bytes\n", CACHE_LINE_SIZE);
    printf("\n");
}

void print_memory_layout() {
    struct SharedData shared_test;
    struct PaddedCounter padded_test[NUM_THREADS];
    
    printf("=== MEMORY LAYOUT ANALYSIS ===\n");
    printf("Unoptimized (False Sharing):\n");
    printf("  sizeof(SharedData): %zu bytes\n", sizeof(struct SharedData));
    printf("  Counter spacing: %zu bytes\n", sizeof(int));
    for (int i = 0; i < NUM_THREADS && i < 2; i++) {
        printf("  counter[%d] at offset: %zu bytes\n", i, i * sizeof(int));
    }
    printf("  → Multiple counters in same cache line!\n\n");
    
    printf("Optimized (Padded):\n");
    printf("  sizeof(PaddedCounter): %zu bytes\n", sizeof(struct PaddedCounter));
    printf("  Counter spacing: %zu bytes\n", sizeof(struct PaddedCounter));
    for (int i = 0; i < NUM_THREADS && i < 2; i++) {
        printf("  padded_counter[%d] at offset: %zu bytes\n", i, i * sizeof(struct PaddedCounter));
    }
    printf("  → Each counter in separate cache line!\n\n");
}

int main() {
    printf("=== FALSE SHARING PERFORMANCE COMPARISON ===\n\n");
    
    print_system_info();
    print_memory_layout();
    
    printf("=== PERFORMANCE TESTING ===\n");
    printf("Running %d test iterations for statistical accuracy...\n\n", NUM_RUNS);
    
    double false_sharing_times[NUM_RUNS];
    double optimized_times[NUM_RUNS];
    double false_sharing_avg = 0, optimized_avg = 0;
    
    // Run tests multiple times for better accuracy
    for (int run = 0; run < NUM_RUNS; run++) {
        printf("Run %d/%d:\n", run + 1, NUM_RUNS);
        
        // Test false sharing version
        printf("  Testing false sharing version... ");
        fflush(stdout);
        false_sharing_times[run] = run_false_sharing_test();
        false_sharing_avg += false_sharing_times[run];
        printf("%.2f ms\n", false_sharing_times[run]);
        
        // Small delay between tests
        for (volatile int i = 0; i < 10000000; i++);
        
        // Test optimized version
        printf("  Testing optimized version... ");
        fflush(stdout);
        optimized_times[run] = run_optimized_test();
        optimized_avg += optimized_times[run];
        printf("%.2f ms\n", optimized_times[run]);
        
        printf("  Speedup this run: %.2fx\n\n", false_sharing_times[run] / optimized_times[run]);
    }
    
    // Calculate averages
    false_sharing_avg /= NUM_RUNS;
    optimized_avg /= NUM_RUNS;
    
    // Calculate throughput
    long total_ops = (long)NUM_THREADS * ITERATIONS;
    double false_sharing_throughput = total_ops / (false_sharing_avg / 1000.0);
    double optimized_throughput = total_ops / (optimized_avg / 1000.0);
    
    // Display final results
    printf("=== FINAL RESULTS ===\n");
    printf("Average execution times:\n");
    printf("  Without padding (false sharing): %.2f ms\n", false_sharing_avg);
    printf("  With padding (optimized):       %.2f ms\n", optimized_avg);
    printf("  Performance improvement:        %.2fx speedup\n", false_sharing_avg / optimized_avg);
    printf("  Time reduction:                 %.1f%%\n", 
           ((false_sharing_avg - optimized_avg) / false_sharing_avg) * 100.0);
    
    printf("\nThroughput comparison:\n");
    printf("  False sharing:  %.0f ops/sec (%.2f million ops/sec)\n", 
           false_sharing_throughput, false_sharing_throughput / 1000000.0);
    printf("  Optimized:      %.0f ops/sec (%.2f million ops/sec)\n", 
           optimized_throughput, optimized_throughput / 1000000.0);
    printf("  Throughput gain: %.2fx\n", optimized_throughput / false_sharing_throughput);
    
    printf("\n=== ANALYSIS ===\n");
    printf("The performance improvement demonstrates the effectiveness of:\n");
    printf("1. Compile-time data layout transformations\n");
    printf("2. Cache-conscious programming techniques\n");
    printf("3. Elimination of false sharing through padding\n");
    printf("4. Proper memory alignment strategies\n");
    
    if (false_sharing_avg / optimized_avg > 1.5) {
        printf("\n✓ Significant performance improvement observed!\n");
        printf("  The optimized version successfully eliminates false sharing.\n");
    } else {
        printf("\n⚠ Limited performance difference observed.\n");
        printf("  This may occur on systems with:\n");
        printf("  - Single-core processors\n");
        printf("  - Very fast cache coherence protocols\n");
        printf("  - Different cache line sizes\n");
        printf("  Try increasing ITERATIONS or NUM_THREADS for more pronounced effects.\n");
    }
    
    printf("\nThis demonstrates the principles from:\n");
    printf("• Dubois et al. (1990): Identification of false sharing problems\n");
    printf("• Jeremiassen & Eggers (1993): Compile-time solutions through data transformations\n");
    
    return 0;
}