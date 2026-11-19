/*
 * Module: Comparison & verification

 * Description: Functions to compare the greedy schedule against the optimal
 *              brute‑force schedule, printing both results and timing.
 */
#ifndef COMPARE_H
#define COMPARE_H

#include "greedy.h"
#include "bruteforce.h"

/* Compare greedy schedule with brute‑force schedule.
   Prints both schedules, their total profit, and whether they match.
   Also prints execution time for each algorithm (in milliseconds). */
void compare_greedy_vs_bruteforce(const Job *jobs, int count);

#endif // COMPARE_H