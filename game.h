#ifndef TETRIS_GAME_H
#define TETRIS_GAME_H

#define BOARD_WIDTH  10
#define BOARD_HEIGHT 12

enum Direction { Up, Down, Left, Right, None };

struct Coords {
    int y;
    int x;
};

void draw_game_board();
void update_game(enum Direction press);

#endif /* TETRIS_GAME_H */
