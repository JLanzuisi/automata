package main

import (
	"image"
	"image/color"
	"image/gif"
	"log"
	"os"
)

type Grid [ROWS][COLUMNS]int

const (
	ROWS        = 60
	COLUMNS     = 60
	GENERATIONS = 150
	GFACTOR     = 10
	GWIDTH      = ROWS * GFACTOR
	GHEIGHT     = COLUMNS * GFACTOR
	GDELAY      = 20
)

func neighbors(grid Grid, x int, y int) int {
	total := 0
	deltas := [][]int{
		{-1, -1},
		{-1, 0},
		{-1, 1},
		{0, 1},
		{1, 1},
		{1, 0},
		{1, -1},
		{0, -1},
	}

	for _, v := range deltas {
		row := x + v[0]
		col := y + v[1]

		if row == -1 {
			row = ROWS - 1
		}
		if row == ROWS {
			row = 0
		}
		if col == -1 {
			col = COLUMNS - 1
		}
		if col == COLUMNS {
			col = 0
		}

		if grid[row][col] == 1 {
			total++
		}
	}

	return total
}

func NextGen(grid Grid) Grid {
	nextgen := grid

	for row := 0; row < ROWS; row++ {
		for col := 0; col < COLUMNS; col++ {
			count := neighbors(grid, row, col)

			if grid[row][col] == 1 {
				if count != 2 && count != 3 {
					nextgen[row][col] = 0
				}
			} else {
				if count == 3 {
					nextgen[row][col] = 1
				}
			}
		}
	}

	return nextgen
}

func encodeGIF(g Grid, path string) {
	var colorIdx int

	nextg := g
	p := []color.Color{color.White, color.Black}
	r := image.Rectangle{image.Point{0, 0}, image.Point{GWIDTH, GHEIGHT}}

	anim := gif.GIF{LoopCount: -1}

	for i := 0; i < GENERATIONS; i++ {
		im := image.NewPaletted(r, p)

		beg := image.Point{0, 0}
		end := image.Point{0, 0}

		for j := 0; j < ROWS; j++ {
			beg.X = end.X
			end.X = (j+1)*GFACTOR - 1
			beg.Y = 0
			end.Y = 0
			for k := 0; k < COLUMNS; k++ {
				beg.Y = end.Y
				end.Y = (k+1)*GFACTOR - 1

				if nextg[j][k] == 1 {
					colorIdx = 1
				} else {
					colorIdx = 0
				}

				for l := beg.X; l < end.X; l++ {
					for m := beg.Y; m < end.Y; m++ {
						im.Set(m, l, p[colorIdx])
					}
				}
			}
		}

		anim.Delay = append(anim.Delay, GDELAY)
		anim.Image = append(anim.Image, im)

		nextg = NextGen(nextg)
	}

	f, _ := os.Create(path)

	defer f.Close()

	gif.EncodeAll(f, &anim)
}

func patternToGrid(p Pattern) (g Grid) {
	missing := 0
	rows := [2]int{0, 0}
	cols := [2]int{0, 0}
	prows := len(p)
	pcols := len(p[0])

	if prows < 1 || pcols < 1 {
		log.Fatal("Invalid pattern.")
	}

	missing = ROWS - prows
	rows[0] = missing / 2
	rows[1] = missing - rows[0]

	missing = COLUMNS - pcols
	cols[0] = missing / 2
	cols[1] = missing - cols[0]

	for i := 0; i < ROWS; i++ {
		for j := 0; j < COLUMNS; j++ {
			if i > rows[0]-1 && i < rows[0]+prows && j > cols[0]-1 && j < cols[0]+pcols {
				g[i][j] = p[i-rows[0]][j-cols[0]]
			} else {
				g[i][j] = 0
			}
		}
	}

	return g
}

func main() {
	ex := CenturyExample()

	path := ex.name + ".gif"
	grid := patternToGrid(ex.p)

	encodeGIF(grid, path)
}
