/*
 * Module: Greedy scheduling

 * Description: Implements the greedy algorithm for job sequencing with deadlines.
 */
#ifndef GREEDY_H
#define GREEDY_H

#include "job.h"

#define MAX_SLOTS 100

typedef struct {
    int slots[MAX_SLOTS]; // slot index holds job id, -1 if empty
    int num_slots;        // number of slots used (max deadline)
    int total_profit;
    int jobs_done;
} Schedule;

/* Performs greedy scheduling on a copy of the jobs array.
   Returns a Schedule containing slot assignments and profit.
*/
Schedule greedy_schedule(const Job *jobs, int count);

/* Verbose version that prints each step of the greedy algorithm. */
void greedy_schedule_verbose(const Job *jobs, int count);

#endif // GREEDY_H