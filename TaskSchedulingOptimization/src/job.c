#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/job.h"

int load_jobs_from_file(const char *filename, Job *jobs, int *count) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        if (strncmp(filename, "../", 3) == 0) {
            fp = fopen(filename + 3, "r");
        } else {
            char alt[128];
            snprintf(alt, sizeof(alt), "../%s", filename);
            fp = fopen(alt, "r");
        }
    }
    if (!fp) return -1;

    char line[128];
    int i = 0;
    while (i < MAX_JOBS && fgets(line, sizeof(line), fp)) {
        char *p = line;
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '#' || *p == '\n' || *p == '\r' || *p == '\0') continue;

        int id, deadline, profit;
        char name[30];
        if (sscanf(p, "%d %29s %d %d", &id, name, &deadline, &profit) == 4) {
            if (deadline < 1 || profit <= 0) continue;
            jobs[i].id = id;
            strncpy(jobs[i].name, name, 30);
            jobs[i].deadline = deadline;
            jobs[i].profit = profit;
            i++;
        }
    }
    *count = i;
    fclose(fp);
    return (i > 0) ? 0 : -1;
}

int input_jobs_manually(Job *jobs, int *count) {
    printf("Enter number of jobs (max %d): ", MAX_JOBS);
    int n; if (scanf("%d", &n) != 1) return -1;
    if (n < 1 || n > MAX_JOBS) return -1;
    for (int i = 0; i < n; ++i) {
        printf("Job %d - ID Name Deadline Profit: ", i+1);
        if (scanf("%d %29s %d %d", &jobs[i].id, jobs[i].name, &jobs[i].deadline, &jobs[i].profit) != 4) return -1;
        if (jobs[i].deadline < 1 || jobs[i].profit <= 0) return -1;
    }
    *count = n;
    return 0;
}

void print_jobs_table(const Job *jobs, int count) {
    printf("+----+----------------------------+----------+--------+\n");
    printf("| ID | Name                       | Deadline | Profit |\n");
    printf("+----+----------------------------+----------+--------+\n");
    for (int i = 0; i < count; ++i) {
        printf("| %2d | %-26s | %8d | %6d |\n", jobs[i].id, jobs[i].name, jobs[i].deadline, jobs[i].profit);
    }
    printf("+----+----------------------------+----------+--------+\n");
}

void generate_random_jobs(Job *jobs, int n) {
    if (n > MAX_JOBS) n = MAX_JOBS;
    srand((unsigned)time(NULL));
    for (int i = 0; i < n; ++i) {
        jobs[i].id = i+1;
        snprintf(jobs[i].name, 30, "J%d", i+1);
        jobs[i].deadline = (rand() % n) + 1; // deadline between 1 and n
        jobs[i].profit = (rand() % 100) + 1; // profit between 1 and 100
    }
}
