/*
 * Module: Brute‑force verification

 * Description: Exhaustive search to find the optimal schedule and a random
 *              schedule tester for comparison with the greedy solution.
 */
#ifndef BRUTEFORCE_H
#define BRUTEFORCE_H

#include "job.h"
#include "greedy.h"

/* Brute-force schedule: tries all subsets (2^n). Returns a Schedule
   with the best feasible profit. */
Schedule bruteforce_schedule(const Job *jobs, int count);

/* Generate N random feasible schedules and report their profits. */
void random_schedule_test(const Job *jobs, int count, int N);

#endif // BRUTEFORCE_H