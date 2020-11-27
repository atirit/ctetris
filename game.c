#include "game.h"

#include "ncurses_util.h"

#include <ncurses.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

enum Piece { T, S, Z, L, J, O, I };

const struct Coords pieces[7][4] = {
    { { 1, 4 }, { 2, 5 }, { 1, 6 }, { 1, 5 } }, // T
    { { 2, 5 }, { 1, 6 }, { 2, 4 }, { 1, 5 } }, // S
    { { 1, 4 }, { 2, 6 }, { 2, 5 }, { 1, 5 } }, // Z
    { { 1, 6 }, { 2, 4 }, { 2, 6 }, { 2, 5 } }, // L
    { { 1, 4 }, { 2, 4 }, { 2, 6 }, { 2, 5 } }, // J
    { { 1, 5 }, { 1, 6 }, { 2, 5 }, { 2, 6 } }, // O
    { { 1, 4 }, { 1, 5 }, { 1, 6 }, { 1, 7 } }, // I
};

int tetris_check() {
    static char *fullLine;
    static char *emptyLine;
    if (!fullLine) {
        fullLine = malloc(sizeof(char) * (BOARD_WIDTH + 1));
        emptyLine = malloc(sizeof(char) * (BOARD_WIDTH + 1));
        for (int i = 0; i < BOARD_WIDTH; i++) {
            fullLine[i] = '#';
            emptyLine[i] = ' ';
        }
        fullLine[BOARD_WIDTH] = '\0';
        emptyLine[BOARD_WIDTH] = '\0';
    }

    int tetrises = 0;
    char *line = malloc(sizeof(char) * (BOARD_WIDTH + 1));
    for (int i = 1; i <= BOARD_HEIGHT; i++) {
        mvinnstr(i, 1, line, BOARD_WIDTH);
        if (strcmp(line, fullLine) == 0) {
            mvaddstr(i, 1, emptyLine);
            tetrises++;
        }
    }

    int dropAmount = 0;
    for (int i = BOARD_HEIGHT + 1; i >= 1; i--) {
        mvinnstr(i, 1, line, BOARD_WIDTH);
        if (strcmp(line, emptyLine) == 0) {
            dropAmount++;
        } else {
            mvaddstr(i + dropAmount, 1, line);
            if (dropAmount) {
                mvaddstr(i, 1, emptyLine);
            }
        }
    }
    return tetrises;
}

void rotate_piece(struct Coords *originalPiece, enum Piece pieceType) {
    if (pieceType == O) return; // No point in rotating an O
    struct Coords piece[4];
    memcpy(piece, originalPiece, sizeof(struct Coords) * 4);
    if (pieceType == I) {
        if (piece[0].x == piece[1].x) { // Horizontal
            for (int i = 0; i < 4; i++) {
                if (i == 1) continue;
                piece[i].x = piece[1].x + (piece[i].y - piece[1].y);
                piece[i].y = piece[1].y;
            }
        } else { // Vertical
            for (int i = 0; i < 4; i++) {
                if (i == 1) continue;
                piece[i].y = piece[1].y + (piece[i].x - piece[1].x);
                piece[i].x = piece[1].x;
            }
        }
    } else {
        // The last coordinate pair always corresponds to the "center" of a non-I/O piece
        for (int i = 0; i < 3; i++) {
            if (piece[i].x == piece[3].x && piece[i].y == piece[3].y - 1) {
                piece[i].x += 1;
                piece[i].y += 1;
            } else if (piece[i].x == piece[3].x && piece[i].y == piece[3].y + 1) {
                piece[i].x -= 1;
                piece[i].y -= 1;
            } else if (piece[i].x == piece[3].x - 1 && piece[i].y == piece[3].y) {
                piece[i].x += 1;
                piece[i].y -= 1;
            } else if (piece[i].x == piece[3].x + 1 && piece[i].y == piece[3].y) {
                piece[i].x -= 1;
                piece[i].y += 1;
            } else if (piece[i].x == piece[3].x - 1 && piece[i].y == piece[3].y - 1) {
                piece[i].x += 2;
            } else if (piece[i].x == piece[3].x + 1 && piece[i].y == piece[3].y - 1) {
                piece[i].y += 2;
            } else if (piece[i].x == piece[3].x + 1 && piece[i].y == piece[3].y + 1) {
                piece[i].x -= 2;
            } else if (piece[i].x == piece[3].x - 1 && piece[i].y == piece[3].y + 1) {
                piece[i].y -= 2;
            }
        }
    }
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (piece[i].x == originalPiece[j].x && piece[i].y == originalPiece[j].y) {
                goto ROTATE_PIECE_OUTER_CONTINUE;
            }
        }
        chtype ch = mvinch(piece[i].y, piece[i].x);
        if (ch == '#' || ch == L'│' || ch == L'▁' || ch == L'▔') return;
    ROTATE_PIECE_OUTER_CONTINUE:
        continue;
    }
    for (int i = 0; i < 4; i++) {
        mvaddch(originalPiece[i].y, originalPiece[i].x, ' ');
    }
    for (int i = 0; i < 4; i++) {
        mvaddch(piece[i].y, piece[i].x, '#');
    }
    refresh();
    memcpy(originalPiece, piece, sizeof(struct Coords) * 4);
}

bool update_piece_on_screen(struct Coords *piece, enum Direction dir) {
    switch (dir) {
    case Down:
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                if (i == j) continue;
                if (piece[i].x == piece[j].x && piece[i].y == piece[j].y - 1) goto UPDATE_PIECE_DOWN_CONTINUE;
            }
            chtype adjCh = mvinch(piece[i].y + 1, piece[i].x);
            if (adjCh == '#' || adjCh == L'▔') return false;
        UPDATE_PIECE_DOWN_CONTINUE:
            continue;
        }
        for (int i = 0; i < 4; i++) {
            mvaddch(piece[i].y, piece[i].x, ' ');
            piece[i].y++;
        }
        for (int i = 0; i < 4; i++) {
            mvaddch(piece[i].y, piece[i].x, '#');
        }
        break;
    case Left:
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                if (i == j) continue;
                if (piece[i].y == piece[j].y && piece[i].x == piece[j].x + 1) goto UPDATE_PIECE_LEFT_CONTINUE;
            }
            chtype adjCh = mvinch(piece[i].y, piece[i].x - 1);
            if (adjCh == '#' || adjCh == L'│') return false;
        UPDATE_PIECE_LEFT_CONTINUE:
            continue;
        }
        for (int i = 0; i < 4; i++) {
            mvaddch(piece[i].y, piece[i].x, ' ');
            piece[i].x--;
        }
        for (int i = 0; i < 4; i++) {
            mvaddch(piece[i].y, piece[i].x, '#');
        }
        break;
    case Right:
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                if (i == j) continue;
                if (piece[i].y == piece[j].y && piece[i].x == piece[j].x - 1) goto UPDATE_PIECE_RIGHT_CONTINUE;
            }
            chtype adjCh = mvinch(piece[i].y, piece[i].x + 1);
            if (adjCh == '#' || adjCh == L'│') return false;
        UPDATE_PIECE_RIGHT_CONTINUE:
            continue;
        }
        for (int i = 0; i < 4; i++) {
            mvaddch(piece[i].y, piece[i].x, ' ');
            piece[i].x++;
        }
        for (int i = 0; i < 4; i++) {
            mvaddch(piece[i].y, piece[i].x, '#');
        }
        break;
    default: break;
    }

    return true;
}

int add_new_piece() {
    int piece = 6; // rand() % 7;
    for (int i = 0; i < 4; i++) {
        if (mvinch(pieces[piece][i].y, pieces[piece][i].x) == '#') return -1;
        mvaddch(pieces[piece][i].y, pieces[piece][i].x, '#');
    }
    return piece;
}

void draw_game_board() {
    // mvaddstr is used in place of mvaddch for wide characters because macOS's ncurses is broken
    mvaddch(0, 0, ' ');
    for (int x = 1; x <= BOARD_WIDTH; x++) {
        mvaddstr(0, x, "▁");
    }
    mvaddch(0, BOARD_WIDTH + 1, ' ');

    for (int y = 1; y <= BOARD_HEIGHT; y++) {
        mvaddstr(y, 0, "│");
        for (int x = 1; x <= BOARD_WIDTH; x++) {
            mvaddch(y, x, ' ');
        }
        mvaddstr(y, BOARD_WIDTH + 1, "│");
    }

    mvaddch(BOARD_HEIGHT + 1, 0, ' ');
    for (int x = 1; x <= BOARD_WIDTH; x++) {
        mvaddstr(BOARD_HEIGHT + 1, x, "▔");
    }
    mvaddch(BOARD_HEIGHT + 1, BOARD_WIDTH + 1, ' ');

    refresh();
}

void update_game(enum Direction press) {
    static int downTimer = 0;
    static struct Coords *piece;
    static enum Piece pieceType;
    if (!piece) {
        piece = malloc(sizeof(struct Coords *) * 4);
        srand(time(NULL));
        rand();
        pieceType = add_new_piece();
        memcpy(piece, pieces[pieceType], sizeof(struct Coords) * 4);
    }

    switch (press) {
    case Up: rotate_piece(piece, pieceType); break;
    case Left:
    case Right: update_piece_on_screen(piece, press); break;
    case Down: goto MOVE_DOWN;
    case None: break;
    }

    downTimer++;
    if (downTimer >= 50) {
    MOVE_DOWN:
        downTimer = 0;
        if (!update_piece_on_screen(piece, Down)) {
            tetris_check();
            pieceType = add_new_piece();
            if (pieceType == -1) { // Game over
                ncurses_deinit();
                printf("%s\n", "Game over");
                exit(0);
            }
            memcpy(piece, pieces[pieceType], sizeof(struct Coords) * 4);
            for (int i = 0; i < 4; i++) {
                mvaddch(piece[i].y, piece[i].x, '#');
            }
        }
    }

    refresh();
}
