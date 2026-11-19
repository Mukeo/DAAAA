#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../include/bruteforce.h"
#include "../include/job.h"
#include "../include/greedy.h"

static int is_feasible(const Job *subset, int sz) {
    // Sort by deadline (simple insertion sort)
    Job *arr = (Job*)malloc(sizeof(Job) * sz);
    memcpy(arr, subset, sizeof(Job) * sz);
    for (int i = 1; i < sz; ++i) {
        Job key = arr[i];
        int j = i - 1;
        while (j >= 0 && arr[j].deadline > key.deadline) {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = key;
    }
    int time = 0;
    for (int i = 0; i < sz; ++i) {
        time++;
        if (arr[i].deadline < time) { free(arr); return 0; }
    }
    free(arr);
    return 1;
}

Schedule bruteforce_schedule(const Job *jobs, int count) {
    Schedule best;
    for (int i = 0; i < MAX_SLOTS; ++i) best.slots[i] = -1;
    best.total_profit = 0;
    best.jobs_done = 0;
    best.num_slots = 0;

    unsigned long long total = 1ULL << count; // up to 2^count subsets
    for (unsigned long long mask = 1; mask < total; ++mask) {
        // build subset
        Job subset[MAX_JOBS];
        int sz = 0;
        int profit = 0;
        for (int i = 0; i < count; ++i) {
            if (mask & (1ULL << i)) {
                subset[sz++] = jobs[i];
                profit += jobs[i].profit;
            }
        }
        if (!is_feasible(subset, sz)) continue;
        if (profit > best.total_profit) {
            best.total_profit = profit;
            best.jobs_done = sz;
            // assign slots based on deadline (simple greedy for the subset)
            int max_dead = 0;
            for (int i = 0; i < sz; ++i) if (subset[i].deadline > max_dead) max_dead = subset[i].deadline;
            if (max_dead > MAX_SLOTS) max_dead = MAX_SLOTS;
            best.num_slots = max_dead;
            for (int i = 0; i < max_dead; ++i) best.slots[i] = -1;
            // place each job in latest free slot <= deadline
            for (int i = 0; i < sz; ++i) {
                int d = subset[i].deadline;
                if (d > max_dead) d = max_dead;
                while (d > 0 && best.slots[d-1] != -1) d--;
                if (d > 0) best.slots[d-1] = subset[i].id;
            }
        }
    }
    return best;
}

void random_schedule_test(const Job *jobs, int count, int N) {
    srand((unsigned)time(NULL));
    int max_profit = 0;
    for (int i = 0; i < N; ++i) {
        // generate a random feasible schedule by random ordering then greedy placement
        Job copy[MAX_JOBS];
        memcpy(copy, jobs, sizeof(Job) * count);
        // shuffle
        for (int j = count - 1; j > 0; --j) {
            int k = rand() % (j + 1);
            Job tmp = copy[j]; copy[j] = copy[k]; copy[k] = tmp;
        }
        // greedy placement on shuffled order (still profit descending not guaranteed)
        Schedule s = greedy_schedule(copy, count);
        if (s.total_profit > max_profit) max_profit = s.total_profit;
    }
    printf("Random schedule test: best profit from %d random schedules = %d\n", N, max_profit);
}
