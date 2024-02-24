// automata.c - Cellular automata in C.
// Copyright 2024 Jhonny Lanzuisi.
// See LICENSE at end of file.
#include "gifenc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define TRUE 1
#define FALSE 0
#define BOARD_SIZE 100 // 2d matrix size is the square of this
#define RULES 10
#define STATES 4
#define COLOR_DEPTH 2
#define COLORS 4 // should always be COLOR_DEPTH^2

typedef int Board[BOARD_SIZE][BOARD_SIZE];

typedef struct {
    int rows;
    int cols;
    Board board;
} Grid;

typedef struct {
    char *code;
    int next_state;
} Rule;

typedef struct {
    int rule_amount;
    Rule rules[RULES];
} RuleSet;

typedef struct {
    int state_amount;
    RuleSet rules[STATES];
} CA;

void init_grid(const int offset, const Grid *p, Grid *g) {
    g->rows = offset * 2 + p->rows;
    g->cols = offset * 2 + p->cols;

    for (int i = 0; i < g->rows; i++) {
        for (int j = 0; j < g->cols; j++) {
            if (offset == 0) {
                g->board[i][j] = p->board[i][j];
            } else if (j > offset - 1 && j < g->cols - offset &&
                       i > offset - 1 && i < g->rows - offset) {
                g->board[i][j] = p->board[i - offset][j - offset];
            } else {
                g->board[i][j] = 0;
            }
        }
    }
}

void neighbors(const Grid *g, const int row, const int col, const int states,
               char code[]) {
    signed int x, y;
    int deltas[8][2] = {
        {-1, -1}, {-1, 0}, {-1, 1}, {0, 1}, {1, 1}, {1, 0}, {1, -1}, {0, -1},
    };
    int total[STATES] = {0};

    for (int i = 0; i < 8; i++) {
        x = row + deltas[i][0];
        y = col + deltas[i][1];

        if (x == -1) {
            x = g->rows - 1;
        } else if (x == (int)g->rows) {
            x = 0;
        };
        if (y == -1) {
            y = g->cols - 1;
        } else if (y == (int)g->cols) {
            y = 0;
        };

        total[g->board[x][y]]++;
    }

    for (int i = 0; i < states; i++) {
        code[i] = total[i] + '0';
    }
    code[states] = '\0';
}

void print_grid_state(const Grid *g) {
    for (int i = 0; i < g->rows; i++) {
        for (int j = 0; j < g->cols; j++) {
            if (g->board[i][j] > 0) {
                printf("%d ", g->board[i][j]);
            } else {
                printf("- ");
            }
        }
        printf("\n");
    }
}

void next_gen(Grid *curr_grid, Grid *next_grid, const CA *ca) {
    char code[STATES] = "";
    Rule rule = {0};
    RuleSet rset = {0};
    int found = FALSE;

    next_grid->rows = curr_grid->rows;
    next_grid->cols = curr_grid->cols;

    for (int i = 0; i < next_grid->rows; i++) {
        for (int j = 0; j < next_grid->cols; j++) {
            found = FALSE;
            neighbors(curr_grid, i, j, ca->state_amount, code);
            rset = ca->rules[curr_grid->board[i][j]];

            for (int k = 0; k < rset.rule_amount; k++) {
                rule = rset.rules[k];

                if (strcmp(rule.code, code) == 0) {
                    next_grid->board[i][j] = rule.next_state;
                    found = TRUE;
                    break;
                }
            }

            if (!found) {
                next_grid->board[i][j] = rset.default_state;
            }
        }
    }

    memcpy(curr_grid->board, next_grid->board, sizeof next_grid->board);
}

void encode_gif(const int generations, const char filename[], Grid *g, Grid *n,
                const CA *ca) {
    const int w = 800;
    const int h = 800;
    int factor = 0;
    int pindex = 0;
    int count = 0;
    int rh = 0;
    int rw = 0;

    uint8_t palette[COLORS * 3] = {178, 190, 181, 107, 102, 255,
                                   255, 0,   0,   0,   255, 0};

    ge_GIF *gif = ge_new_gif(
        filename,             /* file name */
        w, h,                 /* canvas size */
        palette, COLOR_DEPTH, /* palette depth == log2(# of colors) */
        -1,                   /* no transparency */
        0                     /* infinite loop */
    );

    if (gif == NULL) {
        perror("Error generating gif");
        goto end;
    }

    printf("INFO: %d,%d\n", g->cols, g->rows);

    for (int i = 0; i < generations; i++) {
        factor = w / g->cols;

        for (int row = 0; row < g->rows; row++) {
            for (int col = 0; col < g->cols; col++) {
                for (int l = row * factor; l < (row + 1) * factor; l++) {
                    for (int m = col * factor; m < (col + 1) * factor; m++) {
                        gif->frame[l * w + m] = g->board[row][col];
                    }
                }
            }
        }

        next_gen(g, n, ca);

        ge_add_frame(gif, 20);
    }
end:
    ge_close_gif(gif);
}

void random_grid(int rows, int cols, int offset, int states, Grid *curr_grid) {
    Grid pattern = {rows, cols, {0}};

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            pattern.board[i][j] = rand() % states;
        }
    }

    init_grid(offset, &pattern, curr_grid);
}

int main(void) {
    srand(time(NULL));

    Grid curr_grid = {0};
    Grid next_grid = {0};
    // CA ca = {
    //     2,
    //     {
    //         {1, {{"53", 1}}},
    //         {2, {{"62", 1}, {"53", 1}}},
    //     },
    // };
    // CA ca = {
    //     2,
    //     {
    //         {1, {{"62", 1}}},
    //         {},
    //     },
    // };
    CA ca = {
        3,
        {
            {7,
             0,
             {{"602", 2},
              {"512", 2},
              {"422", 2},
              {"332", 2},
              {"242", 2},
              {"152", 2},
              {"062", 2}}},
            {
                0,
                0,
                {0},
            },
            {
                0,
                1,
                {0},
            },
        },
    };
    // Grid pattern = {
    //     4,
    //     4,
    //     {
    //         {0, 1, 0, 1},
    //         {2, 0, 2, 1},
    //         {2, 0, 1, 0},
    //         {1, 0, 1, 0},
    //     },
    // };

    // init_grid(10, &pattern, &curr_grid);

    random_grid(15, 15, 5, ca.state_amount, &curr_grid);

    // print_grid_state(&curr_grid);

    // print_grid_state(&curr_grid);

    // uint8_t init_color[3] = {107, 102, 255};
    // uint8_t bg_color[3] = {178, 190, 181};
    encode_gif(50, "test.gif", &curr_grid, &next_grid, &ca);

    return 0;
}
// LICENSE
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the “Software”), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
