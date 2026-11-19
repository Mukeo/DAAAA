#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include "ui.h"
#include "job.h"
#include "greedy.h"
#include "bruteforce.h"
#include "compare.h"
#include "visual.h"

#define MAX_MENU_ITEMS 10
static const char *menu_items[MAX_MENU_ITEMS] = {
    "Load jobs from file",
    "Enter jobs manually",
    "Generate random jobs",
    "Display jobs",
    "Run greedy scheduling (verbose)",
    "Run brute-force schedule",
    "Compare greedy vs brute-force",
    "Show Gantt chart & profit chart",
    "Performance analysis (placeholder)",
    "Exit"
};

static Job jobs[MAX_JOBS];
static int job_cnt = 0;

static void clear_output(WINDOW *outwin) {
    werase(outwin);
    wrefresh(outwin);
}

static void ui_load_jobs(WINDOW *outwin) {
    clear_output(outwin);
    if (load_jobs_from_file("../data/jobs.txt", jobs, &job_cnt) == 0)
        wprintw(outwin, "\xE2\x9C\x94 Loaded %d jobs from file.\n", job_cnt);
    else
        wprintw(outwin, "\xE2\x9D\x8C Failed to read jobs file.\n");
    wrefresh(outwin);
}

static void ui_input_jobs(WINDOW *outwin) {
    clear_output(outwin);
    if (input_jobs_manually(jobs, &job_cnt) == 0)
        wprintw(outwin, "\xE2\x9C\x94 %d jobs entered manually.\n", job_cnt);
    else
        wprintw(outwin, "\xE2\x9D\x8C Input failed.\n");
    wrefresh(outwin);
}

static void ui_generate_random(WINDOW *outwin) {
    echo();
    mvwprintw(outwin, 0, 0, "Enter number of random jobs (max %d): ", MAX_JOBS);
    wrefresh(outwin);
    char buf[8];
    wgetnstr(outwin, buf, sizeof(buf)-1);
    noecho();
    int n = atoi(buf);
    if (n < 1 || n > MAX_JOBS) {
        wprintw(outwin, "\n\xE2\x9D\x8C Invalid count.\n");
    } else {
        generate_random_jobs(jobs, n);
        job_cnt = n;
        wprintw(outwin, "\n\xE2\x9C\x94 Generated %d random jobs.\n", n);
    }
    wrefresh(outwin);
}

static void ui_display_jobs(WINDOW *outwin) {
    clear_output(outwin);
    if (job_cnt == 0) {
        wprintw(outwin, "\xE2\x9D\x8C No jobs loaded.\n");
    } else {
        wprintw(outwin,
                "\n+----+----------------------------+----------+--------+\n"
                "| ID | Name                       | Deadline | Profit |\n"
                "+----+----------------------------+----------+--------+\n");
        for (int i = 0; i < job_cnt; ++i) {
            wprintw(outwin,
                    "| %2d | %-26s | %8d | %6d |\n",
                    jobs[i].id, jobs[i].name,
                    jobs[i].deadline, jobs[i].profit);
        }
        wprintw(outwin,
                "+----+----------------------------+----------+--------+\n");
    }
    wrefresh(outwin);
}

static void ui_greedy_verbose(WINDOW *outwin) {
    clear_output(outwin);
    if (job_cnt == 0) {
        wprintw(outwin, "\xE2\x9D\x8C No jobs to schedule.\n");
    } else {
        greedy_schedule_verbose(jobs, job_cnt);
        wprintw(outwin, "\n(See terminal output above.)\n");
    }
    wrefresh(outwin);
}

static void ui_bruteforce(WINDOW *outwin) {
    clear_output(outwin);
    if (job_cnt == 0) {
        wprintw(outwin, "\xE2\x9D\x8C No jobs to schedule.\n");
    } else {
        Schedule s = bruteforce_schedule(jobs, job_cnt);
        wprintw(outwin, "\xE2\x9C\x94 Brute-force profit: %d, jobs done: %d\n",
                s.total_profit, s.jobs_done);
    }
    wrefresh(outwin);
}

static void ui_compare(WINDOW *outwin) {
    clear_output(outwin);
    if (job_cnt == 0) {
        wprintw(outwin, "\xE2\x9D\x8C No jobs to compare.\n");
    } else {
        compare_greedy_vs_bruteforce(jobs, job_cnt);
        wprintw(outwin, "\n(See terminal output above.)\n");
    }
    wrefresh(outwin);
}

static void ui_visualise(WINDOW *outwin) {
    clear_output(outwin);
    if (job_cnt == 0) {
        wprintw(outwin, "\xE2\x9D\x8C No jobs to visualise.\n");
    } else {
        Schedule g = greedy_schedule(jobs, job_cnt);
        print_gantt_chart(&g);
        print_profit_bar_chart(g.total_profit, g.total_profit, g.total_profit);
        wprintw(outwin, "\n(See terminal output above.)\n");
    }
    wrefresh(outwin);
}

static void ui_performance(WINDOW *outwin) {
    clear_output(outwin);
    print_performance_table();
    wprintw(outwin, "\n(See terminal output above.)\n");
    wrefresh(outwin);
}

/* -------------------------------------------------------------
 *  run_ui – main UI loop
 * ------------------------------------------------------------- */
void run_ui(void) {
    initscr();
    noecho();
    cbreak();
    keypad(stdscr, TRUE);
    curs_set(0);

    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    /* Title window (single line, centered) */
    WINDOW *title = newwin(3, cols, 0, 0);
    box(title, 0, 0);
    mvwprintw(title, 1, (cols - 34) / 2,
               "Task Scheduling Optimization – UI v1");
    wrefresh(title);

    /* Menu window */
    int menu_h = MAX_MENU_ITEMS + 2;
    int menu_w = 40;
    WINDOW *menu = newwin(menu_h, menu_w, 4, 2);
    box(menu, 0, 0);

    /* Output window (takes the rest of the screen) */
    int out_h = rows - 4;
    int out_w = cols - menu_w - 6;
    WINDOW *outwin = newwin(out_h, out_w, 4, menu_w + 4);
    box(outwin, 0, 0);
    mvwprintw(outwin, 0, 2, " Output ");
    wrefresh(outwin);

    int highlight = 0;
    int ch;
    while (1) {
        /* Draw menu items */
        for (int i = 0; i < MAX_MENU_ITEMS; ++i) {
            if (i == highlight) {
                wattron(menu, A_REVERSE);
                mvwprintw(menu, i + 1, 2, "%s", menu_items[i]);
                wattroff(menu, A_REVERSE);
            } else {
                mvwprintw(menu, i + 1, 2, "%s", menu_items[i]);
            }
        }
        wrefresh(menu);

        ch = wgetch(menu);
        switch (ch) {
            case KEY_UP:
                highlight = (highlight == 0) ? MAX_MENU_ITEMS - 1 : highlight - 1;
                break;
            case KEY_DOWN:
                highlight = (highlight == MAX_MENU_ITEMS - 1) ? 0 : highlight + 1;
                break;
            case 10: /* Enter */
                if (highlight == 9) { /* Exit */
                    endwin();
                    return;
                }
                switch (highlight) {
                    case 0: ui_load_jobs(outwin); break;
                    case 1: ui_input_jobs(outwin); break;
                    case 2: ui_generate_random(outwin); break;
                    case 3: ui_display_jobs(outwin); break;
                    case 4: ui_greedy_verbose(outwin); break;
                    case 5: ui_bruteforce(outwin); break;
                    case 6: ui_compare(outwin); break;
                    case 7: ui_visualise(outwin); break;
                    case 8: ui_performance(outwin); break;
                }
                break;
            case 27: /* Escape */
                endwin();
                return;
        }
    }
}
