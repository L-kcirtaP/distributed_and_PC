#include <iostream>
#include <cstdlib>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
using namespace std;

int main (int argc, char* argv[]){
	Window          win;       
	char            *window_name = "test", *display_name = NULL;                     /* initialization for a window */
	Display         *display;
	GC              gc;   //this is a graphic content, it could be a pixel color
	unsigned long            valuemask = 0;
    XGCValues       values; //value of the graphics content
	XSizeHints      size_hints;
	Pixmap          bitmap;
	XSetWindowAttributes attr[1];
	int             width, height,                  /* window size */
                  x, y,                           /* window position */
                  border_width,                   /*border width in pixels */
                  display_width, display_height,  /* size of screen */
                  screen;                         /* which screen */

	if (  (display = XOpenDisplay (display_name)) == NULL ) {
     fprintf (stderr, "drawon: cannot connect to X server %s\n",
                          XDisplayName (display_name) );
    exit (-1);
    }

    /* get screen size */
    screen = DefaultScreen (display);
    display_width = DisplayWidth (display, screen);
    display_height = DisplayHeight (display, screen);

    /* set window size */

	width = 300;
	height = 300;

	/* set window position */

	x = 0;
	y = 0;

	/* create opaque window */

	border_width = 4;
	win = XCreateSimpleWindow (display, RootWindow (display, screen),
	                      x, y, width, height, border_width, 
	                      WhitePixel (display, screen), WhitePixel (display, screen)); //Change to WhitePixel (display, screen) if you want a white background

	size_hints.flags = USPosition|USSize;
	size_hints.x = x;
	size_hints.y = y;
	size_hints.width = width;
	size_hints.height = height;
	size_hints.min_width = 300;
	size_hints.min_height = 300;

	XSetNormalHints (display, win, &size_hints);
    XStoreName(display, win, window_name);

    /* create graphics context */
    gc = XCreateGC (display, win, valuemask, &values);
    XSetBackground (display, gc, BlackPixel (display, screen));
    XSetForeground (display, gc, WhitePixel (display, screen));
    XSetLineAttributes (display, gc, 1, LineSolid, CapRound, JoinRound);

    attr[0].backing_store = Always;
    attr[0].backing_planes = 1;
    attr[0].backing_pixel = BlackPixel (display, screen);

    XChangeWindowAttributes(display, win, CWBackingStore | CWBackingPlanes | CWBackingPixel, attr);

    XMapWindow (display, win);
    XSync(display, 0);

    XColor color;
    color.red=0; //range from 0~65535
    color.green=65535;
    color.blue=0;

    Status rc1=XAllocColor(display,DefaultColormap(display, screen),&color);
    //set the color and attribute of the graphics content
    XSetForeground (display, gc, color.pixel);
    XSetBackground (display, gc, BlackPixel (display, screen));
    XSetLineAttributes (display, gc, 1, LineSolid, CapRound, JoinRound);
    for (int iter=0;iter<200;iter++){
        XClearWindow(display, win);
        for (int j=0;j<30;j++){
            for (int i=0;i<30;i++){
    	       XDrawPoint (display, win, gc, iter+i, iter+j);
    	       usleep(1); //XDrawPoint cannot draw too fast, otherwise the image cannot be drawn
                //normally you could try to not use the usleep(1), it only black images are shown, try to use this
            }
        }
        XFlush (display);
    }
    sleep(10);
    return 0;
}