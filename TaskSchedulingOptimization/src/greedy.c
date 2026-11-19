#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/greedy.h"

/* Helper to copy jobs array */
static void copy_jobs(const Job *src, Job *dst, int count) {
    for (int i = 0; i < count; ++i) dst[i] = src[i];
}

/* Simple bubble sort descending by profit */
static void sort_by_profit(Job *jobs, int count) {
    for (int i = 0; i < count - 1; ++i) {
        for (int j = 0; j < count - i - 1; ++j) {
            if (jobs[j].profit < jobs[j+1].profit) {
                Job tmp = jobs[j];
                jobs[j] = jobs[j+1];
                jobs[j+1] = tmp;
            }
        }
    }
}

Schedule greedy_schedule(const Job *jobs, int count) {
    Schedule sched;
    for (int i = 0; i < MAX_SLOTS; ++i) sched.slots[i] = -1;
    sched.total_profit = 0;
    sched.jobs_done = 0;
    sched.num_slots = 0;

    // Find maximum deadline
    int max_dead = 0;
    for (int i = 0; i < count; ++i) if (jobs[i].deadline > max_dead) max_dead = jobs[i].deadline;
    if (max_dead > MAX_SLOTS) max_dead = MAX_SLOTS;
    sched.num_slots = max_dead;

    // Work on a copy so original order unchanged
    Job *copy = (Job*)malloc(sizeof(Job) * count);
    copy_jobs(jobs, copy, count);
    sort_by_profit(copy, count);

    // Greedy placement
    for (int i = 0; i < count; ++i) {
        int slot = copy[i].deadline;
        if (slot > max_dead) slot = max_dead;
        while (slot > 0 && sched.slots[slot-1] != -1) slot--;
        if (slot > 0) {
            sched.slots[slot-1] = copy[i].id;
            sched.total_profit += copy[i].profit;
            sched.jobs_done++;
        }
    }
    free(copy);
    return sched;
}

void greedy_schedule_verbose(const Job *jobs, int count) {
    printf("Greedy scheduling (profit descending)\n");
    // Determine max deadline
    int max_dead = 0;
    for (int i = 0; i < count; ++i) if (jobs[i].deadline > max_dead) max_dead = jobs[i].deadline;
    if (max_dead > MAX_SLOTS) max_dead = MAX_SLOTS;
    int *slot = calloc(max_dead, sizeof(int));
    for (int i = 0; i < max_dead; ++i) slot[i] = -1;

    // Copy and sort
    Job *copy = (Job*)malloc(sizeof(Job) * count);
    copy_jobs(jobs, copy, count);
    sort_by_profit(copy, count);

    for (int i = 0; i < count; ++i) {
        printf("Considering Job ID=%d, profit=%d, deadline=%d\n", copy[i].id, copy[i].profit, copy[i].deadline);
        int d = copy[i].deadline;
        if (d > max_dead) d = max_dead;
        while (d > 0 && slot[d-1] != -1) d--;
        if (d > 0) {
            slot[d-1] = copy[i].id;
            printf("   Placed in slot %d\n", d);
        } else {
            printf("   Rejected (no free slot)\n");
        }
    }
    // Final schedule
    printf("Final schedule (slot : JobID)\n");
    for (int i = 0; i < max_dead; ++i) {
        if (slot[i] != -1) printf("   %2d : %d\n", i+1, slot[i]);
        else printf("   %2d : -\n", i+1);
    }
    free(slot);
    free(copy);
}
