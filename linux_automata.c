// automata.c - Cellular automata in C.
// Copyright 2024 Jhonny Lanzuisi.
// See LICENSE at end of file.
#include "automata.h"
#include <X11/Xlib.h>
#include <unistd.h> // sleep()

int main(void) {
    Grid curr_grid = {0};
    Grid next_grid = {0};
    CA ca = {0};

    GoL(&ca);

    // Grid pattern = {
    //     3,
    //     3,
    //     {
    //         {0, 1, 0},
    //         {1, 1, 2},
    //         {0, 1, 0},
    //     },
    // };

    // init_grid(30, &pattern, &curr_grid);

    random_grid(25, 25, 20, ca.state_amount, &curr_grid);

    // encode_gif(100, "test.gif", &curr_grid, &next_grid, &ca);

    Display *dpy = XOpenDisplay(NULL);
    if (dpy == NULL) {
        //error here
        return 1;
    }

    int blackColor = BlackPixel(dpy, DefaultScreen(dpy));
    int whiteColor = WhitePixel(dpy, DefaultScreen(dpy));

    Window w = XCreateSimpleWindow(dpy, DefaultRootWindow(dpy), 0, 0,
                                   800, 600, 0, blackColor, blackColor);

    XSelectInput(dpy, w, StructureNotifyMask);

    XMapWindow(dpy, w);

    GC gc = XCreateGC(dpy, w, 0, NULL);

    XSetForeground(dpy, gc, whiteColor);

    while(TRUE) {
	    XEvent e;
	    XNextEvent(dpy, &e);
	    if (e.type == MapNotify)
            break;
    }

    XDrawLine(dpy, w, gc, 10, 60, 180, 20);

    XFlush(dpy);

    sleep(10);

    return 0;
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
