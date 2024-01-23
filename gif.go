package main

import (
	"image"
	"image/color"
	"image/gif"
	"os"
)

func EncodeGIF(g Grid, path string) {
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
