/*
 * Module: Visualization

 * Description: Terminal‑based visualisation of schedules, profit bar charts,
 *              performance tables and CSV export.
 */
#ifndef VISUAL_H
#define VISUAL_H

#include "greedy.h"
#include "job.h"

/* Print a simple ASCII Gantt chart for a Schedule */
void print_gantt_chart(const Schedule *sched);

/* Print a horizontal bar chart comparing profits of greedy, brute‑force,
   and average random schedules. */
void print_profit_bar_chart(int greedy_profit, int brute_profit, int avg_random_profit);

/* Print a performance table (execution times) for various job counts. */
void print_performance_table(void);

/* Export results to CSV (used by print_performance_table). */
void export_results_csv(const char *filepath);

#endif // VISUAL_H