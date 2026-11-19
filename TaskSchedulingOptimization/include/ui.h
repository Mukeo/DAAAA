#ifndef UI_H
#define UI_H

/* -------------------------------------------------------------
 *  UI – medium‑style ncurses front‑end
 * -------------------------------------------------------------
 *
 *  run_ui() starts the interactive terminal UI.  It presents a
 *  boxed menu, highlights the current choice with reverse video,
 *  and forwards the selected option to the underlying business
 *  logic (the functions already in job.c, greedy.c, etc.).
 *
 *  No new external dependencies apart from ncurses.
 * ------------------------------------------------------------- */
void run_ui(void);

#endif // UI_H
