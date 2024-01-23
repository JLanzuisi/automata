package main

const (
	ROWS        = 60
	COLUMNS     = 60
	GENERATIONS = 150
	GFACTOR     = 10
	GWIDTH      = ROWS * GFACTOR
	GHEIGHT     = COLUMNS * GFACTOR
	GDELAY      = 20
)

func main() {
	ex := CenturyExample()

	path := ex.name + ".gif"
	grid := PatternToGrid(ex.p)

	EncodeGIF(grid, path)
}
