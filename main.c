#include "game.h"
#include "ncurses_util.h"

#include <ncurses.h>
#include <unistd.h>

int main(void) {
    ncurses_init();
    curs_set(0); // Invisible cursor
    draw_game_board();

    int ch;
    enum Direction dir;
    while ((ch = getch())) {
        switch (ch) {
        case KEY_DOWN: dir = Down; break;
        case KEY_UP: dir = Up; break;
        case KEY_LEFT: dir = Left; break;
        case KEY_RIGHT: dir = Right; break;
        default: dir = None; break;
        }
        update_game(dir);
        usleep(20000); // 50FPS
    }
}
