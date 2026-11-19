#include <stdio.h>
#include <time.h>
#include "../include/compare.h"
#include "../include/job.h"
#include "../include/greedy.h"
#include "../include/bruteforce.h"

void compare_greedy_vs_bruteforce(const Job *jobs, int count) {
    clock_t start, end;
    double greedy_time, brute_time;

    // Greedy
    start = clock();
    Schedule greedy = greedy_schedule(jobs, count);
    end = clock();
    greedy_time = ((double)(end - start)) / CLOCKS_PER_SEC * 1000.0; // ms

    // Brute-force
    start = clock();
    Schedule brute = bruteforce_schedule(jobs, count);
    end = clock();
    brute_time = ((double)(end - start)) / CLOCKS_PER_SEC * 1000.0; // ms

    // Print results
    printf("--- Comparison ---\n");
    printf("Greedy profit   : %d (time %.2f ms)\n", greedy.total_profit, greedy_time);
    printf("Brute-force profit: %d (time %.2f ms)\n", brute.total_profit, brute_time);
    if (greedy.total_profit == brute.total_profit) {
        printf("VERIFIED: Greedy is optimal.\n");
    } else {
        printf("MISMATCH: Greedy profit differs from optimal.\n");
    }
    // Show slot assignments (simple)
    printf("Greedy schedule slots:\n");
    for (int i = 0; i < greedy.num_slots; ++i) {
        if (greedy.slots[i] != -1)
            printf("  Slot %d -> Job %d\n", i+1, greedy.slots[i]);
    }
    printf("Brute-force schedule slots:\n");
    for (int i = 0; i < brute.num_slots; ++i) {
        if (brute.slots[i] != -1)
            printf("  Slot %d -> Job %d\n", i+1, brute.slots[i]);
    }
}
