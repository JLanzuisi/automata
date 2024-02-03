#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include "gifenc.h"

typedef struct {
	size_t rows;
	size_t cols;
	bool grid[];
} Grid;

Grid *InitGrid (size_t rows, size_t cols, size_t offset, bool p[]){
	size_t grows =  offset*2 + rows;
	size_t gcols =  offset*2 + cols;
	
	Grid *g = calloc(1, sizeof(Grid) + grows*gcols*sizeof(bool));

	g->rows = grows;
	g->cols = gcols;
	
	for (size_t i=0; i < g->rows; i++){
		for (size_t j=0; j < g->cols; j++){
			if (offset == 0) {
				g->grid[i*g->cols + j] = p[i*cols + j];
			} else if (j > offset-1 && j < g->cols-offset && i > offset-1 && i < g->rows-offset){
				g->grid[i*g->cols + j] = p[(i - offset)*cols + (j - offset)];
			}
		}
	}

	return g;
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
	unsigned int count = 0;
	bool nextgen[g->cols*g->rows];
	
	for (size_t i=0; i < g->rows; i++){
		for (size_t j=0; j < g->cols; j++){
			count = Neighbors(g, i, j);

			if (g->grid[(i*g->cols)+j]) {
				if (count != 2 && count != 3){
					nextgen[(i*g->cols)+j] = 0;
				} else {
					nextgen[(i*g->cols)+j] = 1;
				}
			} else {
				if (count == 3){
					nextgen[(i*g->cols)+j] = 1;
				} else {
					nextgen[(i*g->cols)+j] = 0;
				}
			}
		}
	}
	memcpy(g->grid, nextgen, sizeof(nextgen));
}

int main(void){
	Grid *g = InitGrid(
		3,3,10,
		(bool[]){
			0, 1, 0,
			0, 0, 1,
			1, 1, 1,
		}
		);

	/* create a GIF */
	int w = g->rows*100, h = g->cols*100;
    ge_GIF *gif = ge_new_gif(
        "example.gif",  /* file name */
        w, h,           /* canvas size */
        (uint8_t []) {  /* palette */
            0, 0, 0,
            255, 255, 255,
        },
        1,              /* palette depth == log2(# of colors) */
        -1,             /* no transparency */
        0               /* infinite loop */
		);
    /* /\* /\\* draw some frames *\\/ *\/ */
	for (int i = 0; i < 100; i++) {
		NextGen(g);
		for (int j = 0; j < g->rows; j++) {
			for (int k = 0; k < g->cols; k++) {
				if (g->grid[j*g->cols + k] == 1) {
					for (int l = j*100; l < (j+1)*100; l++) {
						for (int m = k*100; m < (k+1)*100; m++) {
							gif->frame[l*w + m] = 1;
						}
					}
				} else {
					for (int l = j*100; l < (j+1)*100; l++) {
						for (int m = k*100; m < (k+1)*100; m++) {
							gif->frame[l*w + m] = 0;
						}
					}
				}
			}
		}
		ge_add_frame(gif, 10);
	}
    /* /\* /\\* remember to close the GIF *\\/ *\/ */
    ge_close_gif(gif);

	free(g);
	return 0;
}
