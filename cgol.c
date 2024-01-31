#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

typedef struct {
	size_t rows;
	size_t cols;
	unsigned int *grid;
} Grid;

typedef struct {
    Grid grid;
	size_t offset;
} Pattern;

void InitPattern (Pattern *p){
	unsigned int init[] = {
		0, 1, 0,
		1, 1, 1,
        0, 1, 0,
	};

	p->grid.rows = 3;
	p->grid.cols = 3;
	p->offset = 15;
	p->grid.grid = calloc(p->grid.rows*p->grid.cols, sizeof(unsigned int));

	memcpy(p->grid.grid, init, sizeof(init));
}

void InitGrid(Pattern *p, Grid *g){
	g->rows = p->offset*2 + p->grid.rows;
	g->cols = p->offset*2 + p->grid.cols;
	g->grid = calloc(g->rows*g->cols, sizeof(unsigned int));

	for (size_t i=0; i < g->rows; i++){
		for (size_t j=0; j < g->cols; j++){
 			if (j > p->offset-1 && j < g->cols-p->offset && i > p->offset-1 && i < g->rows-p->offset){
				g->grid[(i*g->cols)+j] = p->grid.grid[(i - p->offset)*p->grid.cols + (j - p->offset)];
			}
		}
	}

	free(p->grid.grid);
}

void PrintGrid(Grid *g){
  	for (size_t i=0; i < g->rows; i++){
		for (size_t j=0; j < g->cols; j++){
          if (g->grid[(i*g->cols)+j]) {
			printf("* ");
          } else {
            printf(". ");
          }
		}
		printf("\n");
	}
    printf("\n");
}

size_t Neighbors(Grid *g, size_t row, size_t col){
	size_t total = 0;
	int x, y;
	int deltas[8][2] = {
		{-1, -1},
		{-1, 0},
		{-1, 1},
		{0, 1},
		{1, 1},
		{1, 0},
		{1, -1},
		{0, -1},
	};

	for (size_t i=0; i < 8; i++){
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

			if (g->grid[(x*g->cols)+y]) total++;
	}
    return total;
}

void NextGen(Grid *g){
	unsigned int next[g->cols], buf[g->cols], count = 0;
	
	for (size_t i=0; i < g->rows; i++){
		for (size_t j=0; j < g->cols; j++){
			count = Neighbors(g, i, j);

			if (g->grid[(i*g->cols)+j]) {
				if (count != 2 && count != 3){
					next[j] = 0;
				} else {
					next[j] = 1;
				}
			} else {
				if (count == 3){
					next[j] = 1;
				} else {
					next[j] = 0;
				}
			}
		}

		if (i) {
          memcpy(&g->grid[(i-1)*g->cols], buf, sizeof(buf));
		};

		if (i == g->rows-1) {
			memcpy(&g->grid[i*g->cols], next, sizeof(next));
		} else {
			memcpy(buf, next, sizeof(next));
		}
	}
}

int main(void){
	Pattern p = {0};
	Grid g = {0};

	InitPattern(&p);

	InitGrid(&p, &g);

    PrintGrid(&g);

    while (true){
      sleep(1);
	NextGen(&g);

    PrintGrid(&g);
      }

	free(g.grid);
	return 0;
}
