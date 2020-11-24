#include "ncurses_util.h"

#include <locale.h>
#include <ncurses.h>
#include <signal.h>
#include <stdlib.h>

void ncurses_init() {
    // Default cleanup for ^C, etc.
    signal(SIGINT, ncurses_should_exit);
    signal(SIGTERM, ncurses_should_exit);
    signal(SIGQUIT, ncurses_should_exit);

    setlocale(LC_ALL, "");
    initscr();
    start_color();
    use_default_colors();
    cbreak();
    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);
    noecho();
}

void ncurses_deinit() {
    endwin();
}

void ncurses_should_exit(int signalValue) {
    ncurses_deinit();
    exit(128 + signalValue);
}
