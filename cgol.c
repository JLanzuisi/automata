// cgol.c - conway's game of life.
// Copyright 2024 Jhonny Lanzuisi.
// See LICENSE at end of file.
#include "gifenc.h"
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define INDEX(a, columns, i, j) ((a)[(i) * (columns) + (j)])
#define INDEX_G(g, i, j) INDEX(g->grid, g->cols, i, j)
#define SIZE_OF(a) sizeof(a) / sizeof(a[0])

#define PALLETE_SIZE 8

typedef struct {
    unsigned int rows;
    unsigned int cols;
    bool grid[];
} Grid;

typedef struct {
    char *p_path;
} CmdArgs;

Grid *InitGrid(unsigned int rows, unsigned int cols, unsigned int offset,
               bool p[]) {
    unsigned int grows = offset * 2 + rows;
    unsigned int gcols = offset * 2 + cols;

    Grid *g = calloc(1, sizeof(Grid) + grows * gcols * sizeof(bool));
    if (g == NULL) {
        goto end;
    }

    g->rows = grows;
    g->cols = gcols;

    for (unsigned int i = 0; i < g->rows; i++) {
        for (unsigned int j = 0; j < g->cols; j++) {
            if (offset == 0) {
                INDEX_G(g, i, j) = INDEX(p, cols, i, j);
            } else if (j > offset - 1 && j < g->cols - offset &&
                       i > offset - 1 && i < g->rows - offset) {
                INDEX_G(g, i, j) = INDEX(p, cols, i - offset, j - offset);
            }
        }
    }

end:
    return g;
}

size_t neighbors(Grid *g, size_t row, size_t col) {
    size_t total = 0;
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

        if (INDEX_G(g, x, y)) {
            total++;
        }
    }
    return total;
}

void next_gen(Grid *g) {
    unsigned int count = 0;
    bool nextgen[g->cols * g->rows];

    for (unsigned int i = 0; i < g->rows; i++) {
        for (unsigned int j = 0; j < g->cols; j++) {
            count = neighbors(g, i, j);

            if (INDEX_G(g, i, j)) {
                if (count != 2 && count != 3) {
                    INDEX(nextgen, g->cols, i, j) = 0;
                } else {
                    INDEX(nextgen, g->cols, i, j) = 1;
                }
            } else {
                if (count == 3) {
                    INDEX(nextgen, g->cols, i, j) = 1;
                } else {
                    INDEX(nextgen, g->cols, i, j) = 0;
                }
            }
        }
    }
    memcpy(g->grid, nextgen, sizeof(nextgen));
}

void PrintGrid(Grid *g, unsigned int generations) {
    char chars[9] = {'.', '\'', ';', '-', '=', '+', '%', '#', '@'};
    unsigned int count = 0;

    for (unsigned int k = 0; k < generations; k++) {
        next_gen(g);

        for (unsigned int i = 0; i < g->rows; i++) {
            for (unsigned int j = 0; j < g->cols; j++) {
                if (INDEX_G(g, i, j)) {
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

void EncodeGif(uint8_t init_color[], uint8_t bg_color[], unsigned int factor,
               unsigned int generations, char *filename, Grid *g) {
    unsigned int w = g->rows * factor;
    unsigned int h = g->cols * factor;
    unsigned int pindex = 0;
    unsigned int depth = 3;
    unsigned int count = 0;
    float color_factor = 0.7f;

    uint8_t palette[PALLETE_SIZE * 3] = {0};

    palette[0] = bg_color[0];
    palette[1] = bg_color[1];
    palette[2] = bg_color[2];
    palette[3] = init_color[0];
    palette[4] = init_color[1];
    palette[5] = init_color[2];
    for (int i = 2; i < PALLETE_SIZE; i++) {
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
        next_gen(g);

        for (unsigned int j = 0; j < g->rows; j++) {
            for (unsigned int k = 0; k < g->cols; k++) {
                if (g->grid[j * g->cols + k] == 1) {
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
                        INDEX(gif->frame, w, l, m) = pindex;
                    }
                }
            }
        }

        ge_add_frame(gif, 20);
    }

end:
    ge_close_gif(gif);
}

Grid *RandomGrid(unsigned int rows, unsigned int cols, unsigned int offset) {
    bool pattern[rows * cols];

    srand(time(NULL));

    for (unsigned int i = 0; i < rows * cols; i++) {
        pattern[i] = rand() % 2;
    }

    return InitGrid(rows, cols, offset, pattern);
}

void pop_arg(int *argc, char *argv[]) {
    if (*argc > 0) {
        for (int i = 1; i < *argc; i++) {
            argv[i - 1] = argv[i];
        }
        argv[*argc - 1] = "";
        *argc = *argc - 1;
    }
}

CmdArgs *ParseArgs(int *argc, char *argv[]) {
    CmdArgs *args = malloc(sizeof(CmdArgs));
    if (args == NULL) {
        goto end;
    };
    // FILE *rle;
    // char *filene = "test.txt";

    // rle = fopen(filename, "r");
    // if (rle == NULL) {
    //     fprintf(stderr, "File error on '%s': %s.", filename,
    //     strerror(errno)); exit(1);
    // }

    pop_arg(argc, argv);
    while (*argc > 0) {
        if (strcmp(argv[0], "set-pattern") == 0) {
            pop_arg(argc, argv);
            if (*argc > 0) {
                args->p_path = calloc(strlen(argv[0]), sizeof(char));
                strcpy(args->p_path, argv[0]);
            } else {
                fprintf(stderr, "No path provided to 'set-pattern'.\n");
                exit(1);
            }
            break;
        } else {
            fprintf(stderr, "Subcommand '%s' unknown.\n", argv[0]);
            exit(1);
        }
    }

    // fclose(rle);
end:
    return args;
}

int main(int argc, char *argv[]) {
    CmdArgs *args = ParseArgs(&argc, argv);
    Grid *g = RandomGrid(10, 10, 10);
    /* Grid *g = InitGrid( */
    /* 	3, 3, 10, */
    /* 	(bool[]){ */
    /* 		0, 1, 0, */
    /* 		0, 0, 1, */
    /* 		1, 1, 1, */
    /* 	} */
    /* 	); */

    if (g == NULL) {
        perror("Error generating Grid");
        return EXIT_FAILURE;
    }

    uint8_t init_color[3] = {107, 102, 255};
    uint8_t bg_color[3] = {178, 190, 181};
    EncodeGif(init_color, bg_color, 25, 300, "test.gif", g);
    // PrintGrid(g, 25);

    free(g);
    free(args);
    return EXIT_SUCCESS;
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
