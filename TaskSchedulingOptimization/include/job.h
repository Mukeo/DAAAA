/*
 * Module: Job formulation

 * Description: Defines the Job struct and prototypes for loading, manual input,
 *              printing, and random generation of jobs.
 */
#ifndef JOB_H
#define JOB_H

#define MAX_JOBS 20

typedef struct {
    int id;
    char name[30];
    int deadline;
    int profit;
} Job;

/* Loads jobs from a text file.
   Parameters:
   - filename: path to file.
   - jobs: array to store loaded jobs.
   - count: pointer to integer that receives number of jobs loaded.
   Returns 0 on success, non-zero on error.
*/
int load_jobs_from_file(const char *filename, Job *jobs, int *count);

/* Allows user to input jobs manually via stdin.
   Parameters similar to load_jobs_from_file.
   Returns 0 on success.
*/
int input_jobs_manually(Job *jobs, int *count);

/* Prints a nicely formatted table of jobs to stdout. */
void print_jobs_table(const Job *jobs, int count);

/* Generates n random jobs (deadline >=1, profit>0). */
void generate_random_jobs(Job *jobs, int n);

#endif // JOB_H