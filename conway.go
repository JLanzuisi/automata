package main

import (
	"flag"
	"fmt"
	"log"
	"maps"
	"math/rand"
	"os"
	"regexp"
	"strconv"
	"strings"
)

type Cell [2]int

type CellSet map[Cell]bool

type Grid struct {
	cells CellSet
	size  [2]int
}

func neighbors(grid Grid, cell Cell) int {
	total := 0
	deltas := []Cell{
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
		row := cell[0] + v[0]
		col := cell[1] + v[1]

		if row == -1 {
			row = grid.size[0] - 1
		}
		if row == grid.size[0] {
			row = 0
		}
		if col == -1 {
			col = grid.size[1] - 1
		}
		if col == grid.size[1] {
			col = 0
		}

		n := Cell{row, col}
		exists := grid.cells[n]

		if exists {
			total++
		}
	}

	return total
}

func NextGen(grid Grid) Grid {
	nextgen := Grid{make(CellSet), grid.size}

	maps.Copy(nextgen.cells, grid.cells)

	for row := 0; row < grid.size[0]; row++ {
		for col := 0; col < grid.size[1]; col++ {
			cell := Cell{row, col}
			count := neighbors(grid, cell)

			exists := grid.cells[cell]

			if exists {
				if count != 2 && count != 3 {
					delete(nextgen.cells, cell)
				}
			} else {
				if count == 3 {
					nextgen.cells[cell] = true
				}
			}
		}
	}

	return nextgen
}

func parseArgs() (string, bool, string) {
	//var ErrHelp = errors.New("flag: help requested")

	inputPathPtr := flag.String("input", "", "Path to input file.")
	outputPathPtr := flag.String("output", "", "Path to output file.")
	prettyPrintPtr := flag.Bool("print", false, "Print the grid using ascii characters.")

	flag.Parse()

	return *inputPathPtr, *prettyPrintPtr, *outputPathPtr
}

func check(e error) {
	if e != nil {
		log.Fatal(e)
	}
}

func checkStdin() bool {
	fi, err := os.Stdin.Stat()
	check(err)

	if (fi.Mode() & os.ModeCharDevice) == 0 {
		return true
	} else {
		return false
	}
}

func getInput(path string) string {
	const Bufsize = 1024 * 1
	inputbuf := make([]byte, Bufsize)
	var err error

	if checkStdin() {
		_, err = os.Stdin.Read(inputbuf)
		check(err)
	} else if len(path) > 0 {
		inputbuf, err = os.ReadFile(path)
		check(err)
	} else {
		return ""
	}

	return string(inputbuf)
}

func parseInput(input string) Grid {
	cells := CellSet{}
	var max int

	fields := strings.Split(input, ",")

	re, err := regexp.Compile("[0-9]+")
	check(err)

	for _, v := range fields {
		coords := re.FindAllString(v, -1)
		if len(coords) == 2 {
			x, err := strconv.Atoi(coords[0])
			check(err)
			y, err := strconv.Atoi(coords[1])
			check(err)

			if x > max || y > max {
				if x >= y {
					max = x
				} else {
					max = y
				}
			}

			cells[Cell{x, y}] = true
		}
	}

	return Grid{cells, [2]int{max + 1, max + 1}}
}

func printOutput(grid Grid) {
	for k := range grid.cells {
		fmt.Printf("%v %v,\n", k[0], k[1])
	}
}

func printGrid(grid Grid) {
	rows := grid.size[0]
	cols := grid.size[1]

	for x := 0; x < rows; x++ {
		for y := 0; y < cols; y++ {
			_, exists := grid.cells[Cell{x, y}]

			if exists {
				fmt.Printf("[X]")
			} else {
				fmt.Printf("[ ]")
			}

			if y == cols-1 {
				fmt.Printf("\n")
			}
		}
	}
}

func RandomGrid() Grid {
	const Size = 10
	cells := CellSet{}

	gridSize := [2]int{Size, Size}

	cellAmount := rand.Intn(Size * Size)

	for i := 0; i < cellAmount; i++ {
		x := rand.Intn(Size)
		y := rand.Intn(Size)
		cells[Cell{x, y}] = true
	}

	return Grid{cells, gridSize}
}

func writeOutput(path string) {
	//err := os.WriteFile()
}

func main() {
	var grid Grid
	var nextgen Grid

	infile, print, outfile := parseArgs()
	input := getInput(infile)

	if len(input) > 0 {
		grid = parseInput(input)
	} else {
		grid = RandomGrid()
	}

	if print {
		printGrid(grid)
		nextgen = grid
	} else {
		nextgen = NextGen(grid)
		printOutput(nextgen)
	}

	if len(outfile) > 0 {
		writeOutput(outfile)
	}
}
