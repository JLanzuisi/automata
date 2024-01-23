package main

type Grid [ROWS][COLUMNS]int

func Neighbors(grid Grid, x int, y int) int {
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
			count := Neighbors(grid, row, col)

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
