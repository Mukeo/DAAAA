#include <stdio.h>
#include <stdlib.h>
#include "../include/job.h"
#include "../include/greedy.h"
#include "../include/bruteforce.h"
#include "../include/compare.h"
#include "../include/visual.h"

#define MAX_JOBS 20

void pause_for_enter(void) {
    printf("\n  Press Enter to continue...");
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
    if (c == '\n') {
        getchar();
    }
}

static void print_menu_header(int job_count) {
    printf("\n┌────────────────────────────────────────────────────────┐\n");
    printf("│         TASK SCHEDULING OPTIMIZATION (C99)             │\n");
    printf("│       Job Sequencing with Deadlines & Verification     │\n");
    printf("└────────────────────────────────────────────────────────┘\n");
    printf("  Status: %d job(s) loaded in memory\n", job_count);
    printf("──────────────────────────────────────────────────────────\n");
    printf("  [1] Load sample jobs (data/jobs.txt)\n");
    printf("  [2] Enter jobs manually\n");
    printf("  [3] Generate random benchmark jobs\n");
    printf("  [4] View jobs inventory table\n");
    printf("  [5] Run Greedy scheduler (step-by-step placement)\n");
    printf("  [6] Run Brute-force optimal search (2^n subsets)\n");
    printf("  [7] Compare & Verify (Greedy vs Brute-Force)\n");
    printf("  [8] View Gantt Chart & Profit Bar Visualizer\n");
    printf("  [9] Performance complexity analysis\n");
    printf("  [0] Exit\n");
    printf("──────────────────────────────────────────────────────────\n");
    printf("  Select an option [0-9]: ");
}

int main(void) {
    Job jobs[MAX_JOBS];
    int job_count = 0;
    int choice;
    while (1) {
        print_menu_header(job_count);
        if (scanf("%d", &choice) != 1) {
            printf("\n  [!] Invalid input. Please enter a number between 0 and 9.\n");
            while (getchar() != '\n');
            continue;
        }
        switch (choice) {
            case 1:
                if (load_jobs_from_file("data/jobs.txt", jobs, &job_count) == 0) {
                    printf("\n  [✓] Successfully loaded %d jobs from data/jobs.txt\n", job_count);
                } else {
                    printf("\n  [✗] Failed to load data/jobs.txt\n");
                }
                pause_for_enter();
                break;
            case 2:
                if (input_jobs_manually(jobs, &job_count) == 0) {
                    printf("\n  [✓] Successfully stored %d manual jobs.\n", job_count);
                } else {
                    printf("\n  [✗] Input cancelled or invalid.\n");
                }
                pause_for_enter();
                break;
            case 3:
                printf("\n  Enter number of random jobs (1-%d): ", MAX_JOBS);
                int n; scanf("%d", &n);
                if (n < 1 || n > MAX_JOBS) {
                    printf("  [✗] Invalid count.\n");
                } else {
                    generate_random_jobs(jobs, n);
                    job_count = n;
                    printf("  [✓] Generated %d random jobs.\n", n);
                }
                pause_for_enter();
                break;
            case 4:
                if (job_count == 0) {
                    printf("\n  [!] No jobs in memory. Load or enter jobs first.\n");
                } else {
                    printf("\n");
                    print_jobs_table(jobs, job_count);
                }
                pause_for_enter();
                break;
            case 5:
                if (job_count == 0) {
                    printf("\n  [!] No jobs loaded. Load jobs first.\n");
                } else {
                    printf("\n");
                    greedy_schedule_verbose(jobs, job_count);
                }
                pause_for_enter();
                break;
            case 6:
                if (job_count == 0) {
                    printf("\n  [!] No jobs loaded. Load jobs first.\n");
                } else {
                    printf("\n");
                    Schedule s = bruteforce_schedule(jobs, job_count);
                    printf("  Brute-force Optimal Profit: %d | Scheduled Jobs: %d\n", s.total_profit, s.jobs_done);
                }
                pause_for_enter();
                break;
            case 7:
                if (job_count == 0) {
                    printf("\n  [!] No jobs to compare. Load jobs first.\n");
                } else {
                    printf("\n");
                    compare_greedy_vs_bruteforce(jobs, job_count);
                }
                pause_for_enter();
                break;
            case 8:
                if (job_count == 0) {
                    printf("\n  [!] No jobs to visualize. Load jobs first.\n");
                } else {
                    printf("\n");
                    Schedule g = greedy_schedule(jobs, job_count);
                    print_gantt_chart(&g);
                    print_profit_bar_chart(g.total_profit, g.total_profit, g.total_profit);
                }
                pause_for_enter();
                break;
            case 9:
                printf("\n");
                print_performance_table();
                pause_for_enter();
                break;
            case 0:
                printf("\n  Exiting Task Scheduling Optimization. Goodbye!\n\n");
                return 0;
            default:
                printf("\n  [!] Invalid choice. Select 0-9.\n");
                pause_for_enter();
                break;
        }
    }
    return 0;
}
