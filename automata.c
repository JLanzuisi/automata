// cgol.c - conway's game of life.
// Copyright 2024 Jhonny Lanzuisi.
// See LICENSE at end of file.
#include "c-flags.h"
#include "gifenc.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define BOARD_SIZE 100 // 2d matrix size is the square of this
#define PATH_SIZE 256
#define LINE_SIZE 80
#define PALETTE_SIZE 8
#define NEIGHBORS 8

typedef int Board[BOARD_SIZE][BOARD_SIZE];

typedef struct {
    unsigned int rows;
    unsigned int cols;
    Board grid;
} Grid;

typedef struct {
    int B[NEIGHBORS];
    int S[NEIGHBORS];
} CA;

Grid init_grid(unsigned int rows, unsigned int cols, unsigned int offset,
               Board p) {
    Grid g = {offset * 2 + rows, offset * 2 + cols, {0}};

    for (unsigned int i = 0; i < g.rows; i++) {
        for (unsigned int j = 0; j < g.cols; j++) {
            if (offset == 0) {
                g.grid[i][j] = p[i][j];
            } else if (j > offset - 1 && j < g.cols - offset &&
                       i > offset - 1 && i < g.rows - offset) {
                g.grid[i][j] = p[i - offset][j - offset];
            } else {
                g.grid[i][j] = 0;
            }
        }
    }

    return g;
}

unsigned int neighbors(Grid *g, unsigned int row, unsigned int col) {
    unsigned int total = 0;
    signed int x, y;
    int deltas[8][2] = {
        {-1, -1}, {-1, 0}, {-1, 1}, {0, 1}, {1, 1}, {1, 0}, {1, -1}, {0, -1},
    };

    for (unsigned int i = 0; i < 8; i++) {
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

        if (g->grid[x][y]) {
            total++;
        }
    }
    return total;
}

void print_grid_state(Grid *g) {
    for (unsigned int i = 0; i < g->rows; i++) {
        for (unsigned int j = 0; j < g->cols; j++) {
            if (g->grid[i][j]) {
                printf("* ");
            } else {
                printf("  ");
            }
        }
        printf("\n");
    }
}

void next_gen(Grid *g, CA *au) {
    unsigned int count = 0;
    Board nextgen = {0};

    for (unsigned int i = 0; i < g->rows; i++) {
        for (unsigned int j = 0; j < g->cols; j++) {
            count = neighbors(g, i, j);

            if (g->grid[i][j]) {
                nextgen[i][j] = au->S[count];
            } else {
                nextgen[i][j] = au->B[count];
            }
        }
    }

    for (unsigned int i = 0; i < g->rows; i++) {
        for (unsigned int j = 0; j < g->cols; j++) {
            g->grid[i][j] = nextgen[i][j];
        }
    }
}

void print_grid(Grid *g, CA *au, unsigned int generations) {
    char chars[9] = {'.', '\'', ';', '-', '=', '+', '%', '#', '@'};
    unsigned int count = 0;

    for (unsigned int k = 0; k < generations; k++) {
        next_gen(g, au);

        for (unsigned int i = 0; i < g->rows; i++) {
            for (unsigned int j = 0; j < g->cols; j++) {
                if (g->grid[i][j]) {
                    count = neighbors(g, i, j);
                    printf("%c ", chars[count]);
                } else {
                    printf("  ");
                }
            }
            printf("\n");
        }

        for (unsigned int j = 0; j < g->cols; j++) {
            printf("- ");
        }
        printf("\n");
    }
}

void encode_gif(uint8_t init_color[], uint8_t bg_color[], unsigned int factor,
                unsigned int generations, char filename[], Grid *g, CA *au) {
    unsigned int w = g->rows * factor;
    unsigned int h = g->cols * factor;
    unsigned int pindex = 0;
    unsigned int depth = 3;
    unsigned int count = 0;
    float color_factor = 0.7f;

    uint8_t palette[PALETTE_SIZE * 3] = {0};

    palette[0] = bg_color[0];
    palette[1] = bg_color[1];
    palette[2] = bg_color[2];
    palette[3] = init_color[0];
    palette[4] = init_color[1];
    palette[5] = init_color[2];
    for (int i = 2; i < PALETTE_SIZE; i++) {
        palette[i * 3] = palette[(i * 3) - 3] * color_factor;
        palette[i * 3 + 1] = palette[(i * 3) - 2] * color_factor;
        palette[i * 3 + 2] = palette[(i * 3) - 1] * color_factor;
    }

    ge_GIF *gif =
        ge_new_gif(filename,       /* file name */
                   w, h,           /* canvas size */
                   palette, depth, /* palette depth == log2(# of colors) */
                   -1,             /* no transparency */
                   0               /* infinite loop */
        );

    if (gif == NULL) {
        perror("Error generating gif");
        goto end;
    }

    for (unsigned int i = 0; i < generations; i++) {
        next_gen(g, au);

        for (unsigned int j = 0; j < g->rows; j++) {
            for (unsigned int k = 0; k < g->cols; k++) {
                if (g->grid[j][k] == 1) {
                    count = neighbors(g, j, k);
                    if (count == 0) {
                        pindex = 1;
                    } else {
                        pindex = count - 1;
                    }
                } else {
                    pindex = 0;
                }
                for (unsigned int l = j * factor; l < (j + 1) * factor; l++) {
                    for (unsigned int m = k * factor; m < (k + 1) * factor;
                         m++) {
                        gif->frame[l * w + m] = pindex;
                    }
                }
            }
        }

        ge_add_frame(gif, 20);
    }

end:
    ge_close_gif(gif);
}

Grid random_grid(unsigned int rows, unsigned int cols, unsigned int offset) {
    Board pattern = {0};

    for (unsigned int i = 0; i < rows * cols; i++) {
        for (unsigned int j = 0; j < rows * cols; j++) {
            pattern[i][j] = rand() % 2;
        }
    }

    return init_grid(rows, cols, offset, pattern);
}

CA random_automata(void) {
    CA au = {0};

    for (unsigned int j = 0; j < NEIGHBORS; j++) {
        au.B[j] = rand() % 2;
        au.S[j] = rand() % 2;
    }

    return au;
}

void save_num_str(unsigned int *idx, char c, char saved[LINE_SIZE],
                  char line[LINE_SIZE]) {
    unsigned int count = 0;
    unsigned int saved_idx = 0;

    if (line[*idx] == c) {
        count = *idx;
        while (line[*idx] != ',') {
            (*idx)++;
            if (isdigit(line[*idx]) != 0) {
                saved_idx++;
                saved[saved_idx] = line[*idx];
                // printf("s:[%c], l:[%c]\n", saved[saved_idx], line[*idx]);
                // printf("saved: [%s] | line:[%s]\n", saved, line);
                // printf("idx:%d, count:%d\n", *idx, count);
            }
        }
    }
}

Grid import_rle(char path[PATH_SIZE]) {
    FILE *rle;
    char line[LINE_SIZE] = {0};
    char x_str[LINE_SIZE] = {0};
    char y_str[LINE_SIZE] = {0};

    rle = fopen(path, "r");
    if (rle == NULL) {
        fprintf(stderr, "File error on '%s'.", path);
        exit(1);
    }

    while (fgets(line, LINE_SIZE, rle)) {
        if (line[0] != '#') {
            for (unsigned int i = 0; i < LINE_SIZE; i++) {
                if (line[i] == '\n')
                    break;
                if (line[i] != ' ') {
                    save_num_str(&i, 'x', x_str, line);
                    // save_num_str(&i, 'y', y_str, line);
                }
            }
            printf("x:%s y:%s\n", x_str, y_str);
        }
    }

    fclose(rle);

    // return InitGrid(rows, cols, offset, pattern);
}

int main(int argc, char *argv[]) {
    srand(time(NULL));

    Grid g = {0};
    CA au = {0};
    // CA gol = {
    //     {0, 0, 0, 1},
    //     {0, 0, 1, 1},
    // };

    // g = random_grid(10, 10, 10);
    au = random_automata();

    // if (strlen(args.p_path) > 0) {
    //     g = ImportRle(args.p_path);
    // } else {
    //     g = RandomGrid(10, 10, 10);
    // }

    g = init_grid(3, 3, 10,
                  (int[BOARD_SIZE][BOARD_SIZE]){
                      {0, 1, 0},
                      {0, 0, 1},
                      {1, 1, 1},
                  });
    // PrintGrid(&g, &GoL, 20);

    // uint8_t init_color[3] = {107, 102, 255};
    // uint8_t bg_color[3] = {178, 190, 181};
    // encode_gif(init_color, bg_color, 25, 300, "gifs/test.gif", &g, &au);

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
