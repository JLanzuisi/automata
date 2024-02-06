#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <errno.h>
#include "gifenc.h"

#define INDEX(a, columns, i, j) ((a)[(i) * (columns) + (j)])
#define INDEX_G(g, i, j) INDEX(g->grid, g->cols, i, j)

#define PALLETE_SIZE 8

typedef struct {
	int rows;
	int cols;
	bool grid[];
} Grid;

Grid *InitGrid (int rows, int cols, int offset, bool p[]){
	int grows =  offset*2 + rows;
	int gcols =  offset*2 + cols;
	
	Grid *g = calloc(1, sizeof(Grid) + grows*gcols*sizeof(bool));
	if (g == NULL) {
		goto end;
	}

	g->rows = grows;
	g->cols = gcols;
	
	for (int i=0; i < g->rows; i++){
		for (int j=0; j < g->cols; j++){
			if (offset == 0) {
				INDEX_G(g, i, j) = INDEX(p, cols, i, j);
			} else if (j > offset-1 && j < g->cols-offset && i > offset-1 && i < g->rows-offset){
				INDEX_G(g, i, j) = INDEX(p, cols, i-offset, j-offset);
			}
		}
	}

end:
	return g;
}

void PrintGrid(Grid *g){
  	for (int i=0; i < g->rows; i++){
		for (int j=0; j < g->cols; j++){
			if (INDEX_G(g, i, j)) {
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

			if (INDEX_G(g, x, y)) total++;
	}
    return total;
}

void NextGen(Grid *g){
	int count = 0;
	bool nextgen[g->cols*g->rows];
	
	for (int i=0; i < g->rows; i++){
		for (int j=0; j < g->cols; j++){
			count = Neighbors(g, i, j);

			if (INDEX_G(g, i, j)) {
				if (count != 2 && count != 3){
					INDEX(nextgen, g->cols, i, j) = 0;
				} else {
					INDEX(nextgen, g->cols, i, j) = 1;
				}
			} else {
				if (count == 3){
					INDEX(nextgen, g->cols, i, j) = 1;
				} else {
					INDEX(nextgen, g->cols, i, j) = 0;
				}
			}
		}
	}
	memcpy(g->grid, nextgen, sizeof(nextgen));
}

void EncodeGif(int factor, int generations, char *filename, Grid *g){
	int w = g->rows*factor, h = g->cols*factor;
	int pindex = 0, depth = 3, count = 0;
	float color_factor = 0.7f;

	uint8_t palette[PALLETE_SIZE*3] = {0};
	uint8_t init_color[3] = {107,102,255};
	uint8_t bg_color[3] = {178, 190, 181};

	palette[0] = bg_color[0];
	palette[1] = bg_color[1];
	palette[2] = bg_color[2];
	palette[3] = init_color[0];
	palette[4] = init_color[1];
	palette[5] = init_color[2];
	for (int i=2; i<PALLETE_SIZE; i++){
		palette[i*3] = palette[(i*3)-3] * color_factor;
		palette[i*3+1] = palette[(i*3)-2] * color_factor;
		palette[i*3+2] = palette[(i*3)-1] * color_factor;
	}
	
    ge_GIF *gif = ge_new_gif(
        filename,  /* file name */
        w, h,           /* canvas size */
		palette,
        depth,              /* palette depth == log2(# of colors) */
        -1,             /* no transparency */
        0               /* infinite loop */
		);

	if (gif == NULL) {
		perror("Error generating gif");
		goto end;
	}

	for (int i = 0; i < generations; i++) {
		NextGen(g);
		
		for (int j = 0; j < g->rows; j++) {
			for (int k = 0; k < g->cols; k++) {
				if (g->grid[j*g->cols + k] == 1) {
					count = Neighbors(g,j,k);
					if ( count == 0 ) {
						pindex = 1;
					} else {
						pindex = count-1;
					}
				} else {
					pindex = 0;
				}
				for (int l = j*factor; l < (j+1)*factor; l++) {
					for (int m = k*factor; m < (k+1)*factor; m++) {
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

Grid* RandomGrid(int rows, int cols, int offset){
	bool pattern[rows*cols];

	srand(time(NULL));
	
	for (int i=0; i<rows*cols; i++){
		pattern[i] = rand()%2;
	}
	
	return InitGrid(rows, cols, offset, pattern);
}

int main(int argc, char *argv[]){
	FILE * rle;
	char *filename = "test.txt";
	rle = fopen (filename, "r");
	if (rle == NULL) {
		fprintf(stderr, "File error on '%s': %s.", filename, strerror(errno));
		return 1;
	}
	
	while (argc > 1){
		if (strcmp(argv[1], "file") == 0) {
			break;
		} else {
			fprintf(stderr, "Subcommand '%s' unknown.\n", argv[1]);
			return 1;
		}
	}

	fclose(rle);

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
		return 1;
	}

	EncodeGif(25, 300, "test.gif", g);

	free(g);
	return 0;
}
