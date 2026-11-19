#include <stdio.h>
#include <stdlib.h>
#include "../include/job.h"
#include "../include/greedy.h"
#include "../include/bruteforce.h"
#include "../include/compare.h"
#include "../include/visual.h"

#define MAX_JOBS 20

void pause_for_enter(void) {
    printf("\nPress Enter to continue...");
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
    if (c == '\n') {
        getchar();
    }
}

int main(void) {
    Job jobs[MAX_JOBS];
    int job_count = 0;
    int choice;
    while (1) {
        printf("\n=== Task Scheduling Optimization ===\n");
        printf("1. Load jobs from file\n");
        printf("2. Enter jobs manually\n");
        printf("3. Generate random jobs\n");
        printf("4. Display jobs\n");
        printf("5. Run greedy scheduling (verbose)\n");
        printf("6. Run brute‑force schedule\n");
        printf("7. Compare greedy vs brute‑force\n");
        printf("8. Show Gantt chart and profit chart\n");
        printf("9. Performance analysis\n");
        printf("0. Exit\n");
        printf("Enter choice: ");
        if (scanf("%d", &choice) != 1) {
            printf("Invalid input.\n");
            while (getchar() != '\n');
            continue;
        }
        switch (choice) {
            case 1:
                if (load_jobs_from_file("data/jobs.txt", jobs, &job_count) == 0) {
                    printf("Jobs loaded successfully (%d jobs).\n", job_count);
                } else {
                    printf("Failed to load jobs.\n");
                }
                pause_for_enter();
                break;
            case 2:
                if (input_jobs_manually(jobs, &job_count) == 0) {
                    printf("Jobs entered successfully (%d jobs).\n", job_count);
                } else {
                    printf("Failed to input jobs.\n");
                }
                pause_for_enter();
                break;
            case 3:
                printf("Enter number of random jobs (max %d): ", MAX_JOBS);
                int n; scanf("%d", &n);
                if (n < 1 || n > MAX_JOBS) {
                    printf("Invalid count.\n");
                } else {
                    generate_random_jobs(jobs, n);
                    job_count = n;
                    printf("Generated %d random jobs.\n", n);
                }
                pause_for_enter();
                break;
            case 4:
                if (job_count == 0) {
                    printf("No jobs loaded.\n");
                } else {
                    print_jobs_table(jobs, job_count);
                }
                pause_for_enter();
                break;
            case 5:
                if (job_count == 0) {
                    printf("No jobs to schedule.\n");
                } else {
                    greedy_schedule_verbose(jobs, job_count);
                }
                pause_for_enter();
                break;
            case 6:
                if (job_count == 0) {
                    printf("No jobs to schedule.\n");
                } else {
                    Schedule s = bruteforce_schedule(jobs, job_count);
                    printf("Brute‑force profit: %d, jobs done: %d\n", s.total_profit, s.jobs_done);
                }
                pause_for_enter();
                break;
            case 7:
                if (job_count == 0) {
                    printf("No jobs to compare.\n");
                } else {
                    compare_greedy_vs_bruteforce(jobs, job_count);
                }
                pause_for_enter();
                break;
            case 8:
                if (job_count == 0) {
                    printf("No jobs to visualize.\n");
                } else {
                    Schedule g = greedy_schedule(jobs, job_count);
                    print_gantt_chart(&g);
                    print_profit_bar_chart(g.total_profit, g.total_profit, g.total_profit);
                }
                pause_for_enter();
                break;
            case 9:
                print_performance_table();
                pause_for_enter();
                break;
            case 0:
                printf("Good bye!\n");
                return 0;
            default:
                printf("Invalid option.\n");
                pause_for_enter();
                break;
        }
    }
    return 0;
}
