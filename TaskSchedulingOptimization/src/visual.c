#include <stdio.h>
#include <stdlib.h>
#include "../include/visual.h"
#include "../include/greedy.h"

void print_gantt_chart(const Schedule *sched) {
    printf("Gantt Chart (Time Slots)\n");
    printf("+----+------------+\n");
    for (int i = 0; i < sched->num_slots; ++i) {
        if (sched->slots[i] != -1)
            printf("| T%2d | Job %2d   |\n", i+1, sched->slots[i]);
        else
            printf("| T%2d |   -       |\n", i+1);
        printf("+----+------------+\n");
    }
}

void print_profit_bar_chart(int greedy_profit, int brute_profit, int avg_random_profit) {
    printf("Profit Bar Chart\n");
    const int scale = 1; // 1 profit = 1 '#'
    printf("Greedy     : ");
    for (int i = 0; i < greedy_profit * scale; ++i) putchar('#');
    printf(" (%d)\n", greedy_profit);
    printf("BruteForce : ");
    for (int i = 0; i < brute_profit * scale; ++i) putchar('#');
    printf(" (%d)\n", brute_profit);
    printf("Random Avg : ");
    for (int i = 0; i < avg_random_profit * scale; ++i) putchar('#');
    printf(" (%d)\n", avg_random_profit);
}

void print_performance_table(void) {
    printf("Performance Table (ms)\n");
    printf("+------+-----------+------------+\n");
    printf("| Jobs | Greedy(ms)| Brute(ms)  |\n");
    printf("+------+-----------+------------+\n");
    int sizes[] = {5, 10, 15, 18};
    for (int i = 0; i < 4; ++i) {
        printf("| %4d |    N/A    |    N/A    |\n", sizes[i]);
    }
    printf("+------+-----------+------------+\n");
}

void export_results_csv(const char *filepath) {
    FILE *fp = fopen(filepath, "w");
    if (!fp) {
        perror("Failed to open CSV file");
        return;
    }
    fprintf(fp, "Jobs,Greedy_ms,Brute_ms\n");
    int sizes[] = {5,10,15,18};
    for (int i = 0; i < 4; ++i) {
        fprintf(fp, "%d,0,0\n", sizes[i]);
    }
    fclose(fp);
    printf("Results exported to %s\n", filepath);
}
