// automata.c - Cellular automata in C.
// Copyright 2024 Jhonny Lanzuisi.
// See LICENSE at end of file.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "gifenc.c"
#include "raylib.h"

#define BOARD_SIZE 100 // 2d matrix size is the square of this
#define RULES 50
#define STATES 4
#define COLOR_DEPTH 2
#define COLORS 4 // should always be COLOR_DEPTH^2
#define WIDTH 1280
#define HEIGHT 720

typedef int Board[BOARD_SIZE][BOARD_SIZE];

typedef struct {
    int rows;
    int cols;
    Board board;
    bool modified;
} Grid;

typedef struct {
    char *code;
    int next_state;
} Rule;

typedef struct {
    int rule_amount;
    int default_state;
    Rule rules[RULES];
} RuleSet;

typedef struct {
    int state_amount;
    RuleSet ruleset[STATES];
} CA;

typedef struct {
    Color bg;
    Color fg;
} Colors;

typedef enum {
    TitleScreen,
    Play,
    Paused,
    RenderingGif,
} GameStates;

void clear_board(Board b) {
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            b[i][j] = 0;
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
    bool found = false;

    next_grid->rows = curr_grid->rows;
    next_grid->cols = curr_grid->cols;

    for (int i = 0; i < next_grid->rows; i++) {
        for (int j = 0; j < next_grid->cols; j++) {
            found = false;
            neighbors(curr_grid, i, j, ca->state_amount, code);
            rset = ca->ruleset[curr_grid->board[i][j]];

            for (int k = 0; k < rset.rule_amount; k++) {
                rule = rset.rules[k];

                if (strcmp(rule.code, code) == 0) {
                    next_grid->board[i][j] = rule.next_state;
                    found = true;
                    break;
                }
            }

            if (!found) {
                next_grid->board[i][j] = rset.default_state;
            }
        }
    }

    *curr_grid = *next_grid;
}

void encode_gif(const int generations, const char filename[], Grid *g, Grid *n,
                const CA *ca) {
    const int w = 800;
    const int h = 800;
    int factor = 0;

    uint8_t palette[COLORS * 3] = {0, 0, 0, 255, 255, 0, 100, 0, 0, 0, 255, 0};

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

        ge_add_frame(gif, 25);
    }
end:
    ge_close_gif(gif);
}

void random_grid(int rows, int cols, int states, Grid *curr_grid) {
    srand(time(NULL));

    curr_grid->rows = rows;
    curr_grid->cols = cols;

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            curr_grid->board[i][j] = rand() % states;
        }
    }
}

void init_ruleset(RuleSet *rset, int r_amount, int d_state, char *codes[],
                  int states[]) {
    rset->rule_amount = r_amount;
    rset->default_state = d_state;
    for (int i = 0; i < rset->rule_amount; i++) {
        rset->rules[i].code = codes[i];
        rset->rules[i].next_state = states[i];
    }
}

// https://conwaylife.com/wiki/Conway%27s_Game_of_Life
void GoL(CA *ca) {
    ca->state_amount = 2;

    init_ruleset(&ca->ruleset[0], 1, 0, (char *[]){"53"}, (int[]){1});

    init_ruleset(&ca->ruleset[1], 2, 0, (char *[]){"62", "53"}, (int[]){1, 1});
}

// https://conwaylife.com/wiki/OCA:Seeds
void Seeds(CA *ca) {
    ca->state_amount = 2;

    init_ruleset(&ca->ruleset[0], 1, 0, (char *[]){"62"}, (int[]){1});
}

// https://conwaylife.com/wiki/OCA:H-trees
void HT(CA *ca) {
    ca->state_amount = 2;

    init_ruleset(&ca->ruleset[0], 1, 0, (char *[]){"71"}, (int[]){1});

    ca->ruleset[1].default_state = 1;
}

// https://conwaylife.com/wiki/OCA:Serviettes
void Serv(CA *ca) {
    ca->state_amount = 2;

    init_ruleset(&ca->ruleset[0], 3, 0, (char *[]){"62", "53", "44"},
                 (int[]){1, 1, 1});
}

// https://conwaylife.com/wiki/OCA:Brian%27s_Brain
void BB(CA *ca) {
    ca->state_amount = 3;

    init_ruleset(&ca->ruleset[0], 7, 0,
                 (char *[]){"620", "521", "422", "323", "224", "125", "026"},
                 (int[]){1, 1, 1, 1, 1, 1, 1});

    ca->ruleset[1].default_state = 2;
}

void draw_grid(Grid curr_grid, Colors palette, int screen_width,
               int screen_height, int *square_size, int *y_offset,
               int *x_offset) {
    int grid_h_boundary = 0;
    int grid_v_boundary = 0;

    *x_offset = screen_width * 0.02;
    grid_h_boundary = (screen_width * 0.7) + *x_offset;
    grid_v_boundary = screen_height;

    if (grid_v_boundary / curr_grid.rows < grid_h_boundary / curr_grid.cols) {
        *square_size = grid_v_boundary / curr_grid.rows;
    } else {
        *square_size = grid_h_boundary / curr_grid.cols;
    }

    *y_offset = (screen_height - (*square_size * curr_grid.rows)) / 2;

    for (int i = 0; i < curr_grid.rows; i++) {
        for (int j = 0; j < curr_grid.cols; j++) {
            if (curr_grid.board[i][j] == 1) {
                DrawRectangle((j * *square_size) + *x_offset,
                              (i * *square_size) + *y_offset, *square_size,
                              *square_size, palette.fg);
            } else {
                DrawRectangleLines((j * *square_size) + *x_offset,
                                   (i * *square_size) + *y_offset, *square_size,
                                   *square_size, palette.fg);
            }
        }
    }
}

void DrawTextCentered(char *text, int font_size, int y_offset, Color color,
                      int swidth, int sheight) {
    DrawText(text, (swidth / 2) - MeasureText(text, font_size) / 2,
             sheight / 2 - y_offset, font_size, color);
}

void check_keyboard_input(GameStates *state, Grid *curr_grid,
                          Grid *initial_grid, const CA ca) {
    if (*state == TitleScreen) {
        if (IsKeyReleased(KEY_ENTER)) {
            *state = Paused;
        }
    } else if (*state == Play) {
        if (IsKeyReleased(KEY_P)) {
            *state = Paused;
        }
    } else if (*state == Paused) {
        if (IsKeyReleased(KEY_P)) {
            if (curr_grid->modified) {
                *initial_grid = *curr_grid;
                curr_grid->modified = false;
            }
            *state = Play;
        }
    }

    if (*state == Play || *state == Paused) {
        if (IsKeyReleased(KEY_G)) {
            *state = RenderingGif;
        } else if (IsKeyReleased(KEY_T)) {
            *state = TitleScreen;
        } else if (IsKeyReleased(KEY_C)) {
            clear_board(curr_grid->board);
            *initial_grid = *curr_grid;
        } else if (IsKeyReleased(KEY_R)) {
            *curr_grid = *initial_grid;
        } else if (IsKeyReleased(KEY_N)) {
            random_grid(curr_grid->rows, curr_grid->cols, ca.state_amount,
                        curr_grid);
            *initial_grid = *curr_grid;
        }
    }
}

int main(void) {
    Grid curr_grid = {0};
    Grid next_grid = {0};
    Grid initial_grid = {0};
    CA ca = {0};

    GoL(&ca);

    curr_grid.cols = 20;
    curr_grid.rows = 15;
    initial_grid = curr_grid;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(WIDTH, HEIGHT, "Automata");
    const Colors palette = {BLACK, BLUE};
    int screen_width = GetScreenWidth();
    int screen_height = GetScreenHeight();
    int square_size = 0;
    int grid_y_offset = 0;
    int grid_x_offset = 0;
    int mouse_row = 0;
    int mouse_col = 0;
    float delta_time = 0.0f;
    float time_when_pressed = 0.0f;
    float grid_refresh = 0.5f;

    GameStates state;
    state = TitleScreen;

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        screen_width = GetScreenWidth();
        screen_height = GetScreenHeight();
        delta_time += GetFrameTime();

        BeginDrawing();
        ClearBackground(palette.bg);

        switch (state) {
        case TitleScreen:
            DrawTextCentered(
                "This is a visualization tool for cellular automata.", 30, 10,
                palette.fg, screen_width, screen_height);
            DrawTextCentered("Press Enter to begin.", 25, -40, palette.fg,
                             screen_width, screen_height);

            check_keyboard_input(&state, &curr_grid, &initial_grid, ca);

            break;
        case Paused:
            draw_grid(curr_grid, palette, screen_width, screen_height,
                      &square_size, &grid_y_offset, &grid_x_offset);

            // TODO: Maybe use CheckCollision*Rec funtions here
            if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) ||
                IsMouseButtonReleased(MOUSE_BUTTON_RIGHT)) {
                TraceLog(LOG_DEBUG, "%d, %d",
                         (GetMouseX() - grid_x_offset) / square_size,
                         (GetMouseY() - grid_y_offset) / square_size);

                curr_grid.modified = true;

                mouse_col = (GetMouseX() - grid_x_offset) / square_size;
                mouse_row = (GetMouseY() - grid_y_offset) / square_size;

                if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
                    curr_grid.board[mouse_row][mouse_col] = 1;
                } else {
                    curr_grid.board[mouse_row][mouse_col] = 0;
                }
            }

            check_keyboard_input(&state, &curr_grid, &initial_grid, ca);

            break;
        case Play:
            draw_grid(curr_grid, palette, screen_width, screen_height,
                      &square_size, &grid_y_offset, &grid_x_offset);

            if (delta_time > grid_refresh) {
                delta_time = 0.0f;
                next_gen(&curr_grid, &next_grid, &ca);
            }

            check_keyboard_input(&state, &curr_grid, &initial_grid, ca);

            break;
        case RenderingGif:
            DrawTextCentered("Rendering gif...", 30, 10, palette.fg,
                             screen_width, screen_height);

            if (GetTime() > time_when_pressed + 5) {
                encode_gif(1000, "test.gif", &initial_grid, &next_grid, &ca);
                state = Paused;
            }

            break;
        }
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
// LICENSE
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// “Software”), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to permit
// persons to whom the Software is furnished to do so, subject to the
// following conditions:

// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
// NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
// OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
// USE OR OTHER DEALINGS IN THE SOFTWARE.
