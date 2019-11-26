#include <iostream>
#include <cstdlib>
#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <time.h>
#include <math.h>

#define NUMBER_OF_ITERATIONS 10000


int main (int argc, char* argv[]){

    int X_RESN = atoi(argv[1]);             /* Argument 2: the width and height of the window */ 
    int Y_RESN = atoi(argv[1]);

    // Window          win;       
    // char            *window_name = "test", *display_name = NULL;                     /* initialization for a window */
    // Display         *display;
    // GC              gc;   //this is a graphic content, it could be a pixel color
    // GC*             gcs;
    // unsigned long            valuemask = 0;
    // XGCValues       values; //value of the graphics content
    // XSizeHints      size_hints;
    // Pixmap          bitmap;
    // XSetWindowAttributes attr[1];
    // int             width, height,                  /* window size */
    //                 x, y,                           /* window position */
    //                 border_width,                   /*border width in pixels */
    //                 display_width, display_height,  /* size of screen */
    //                 screen;                         /* which screen */

    // if (  (display = XOpenDisplay (display_name)) == NULL ) {
    //    fprintf (stderr, "drawon: cannot connect to X server %s\n",
    //                         XDisplayName (display_name) );
    //   exit (-1);
    //   }

    // /* get screen size */
    // screen = DefaultScreen (display);
    // display_width = DisplayWidth (display, screen);
    // display_height = DisplayHeight (display, screen);

    // /* set window size */
    // width = X_RESN;
    // height = Y_RESN;

    // /* set window position */
    // x = 0;
    // y = 0;

    // /* create opaque window */

    // border_width = 4;
    // win = XCreateSimpleWindow (display, RootWindow (display, screen),
    //                       x, y, width, height, border_width, 
    //                       WhitePixel (display, screen), WhitePixel (display, screen)); //Change to WhitePixel (display, screen) if you want a white background

    // size_hints.flags = USPosition|USSize;
    // size_hints.x = x;
    // size_hints.y = y;
    // size_hints.width = width;
    // size_hints.height = height;
    // size_hints.min_width = 300;
    // size_hints.min_height = 300;

    // XSetNormalHints (display, win, &size_hints);
    // XStoreName(display, win, window_name);

    // /* create graphics context */
    // gcs = (GC *) malloc ((100-20)/10*sizeof(GC));
    // for (int i = 0; i < (100-20)/10; i++) {
    //     gc = XCreateGC (display, win, valuemask, &values);
    //     gcs[i] = gc;
    // }

    // attr[0].backing_store = Always;
    // attr[0].backing_planes = 1;
    // attr[0].backing_pixel = BlackPixel (display, screen);

    // XChangeWindowAttributes(display, win, CWBackingStore | CWBackingPlanes | CWBackingPixel, attr);

    // XMapWindow (display, win);
    // XSync(display, 0);

    // XFlush (display);

    // XColor color;
    // for (int i = 0; i < (100-20)/10; i++) {
    //     if (i == 0) {
    //         color.red = 0;
    //         color.green = 0;
    //         color.blue = 65535;  
    //     } else if (i == 7) {
    //         color.red = 65535;
    //         color.green = 0;
    //         color.blue = 0;
    //     } else {
    //         color.red = i*8191;
    //         color.green = i*7000;
    //         color.blue = 28000-i*4000;
    //     }
    //     Status rc1=XAllocColor(display, DefaultColormap(display, screen), &color);
    //     XSetForeground (display, gcs[i], color.pixel);
    //     XSetBackground (display, gcs[i], BlackPixel (display, screen));
    //     XSetLineAttributes (display, gcs[i], 1, LineSolid, CapRound, JoinRound);
    // }

    //set the color and attribute of the graphics content

    double * temp = (double *) malloc ((X_RESN+1)*(Y_RESN+1)*sizeof(double));
    double * room = (double *) malloc ((X_RESN+1)*(Y_RESN+1)*sizeof(double));

    for (int i = 0; i < (X_RESN+1)*(Y_RESN+1); i++) {
        room[i] = 20;
    }
    // set fireplace
    for (int i = -X_RESN/6; i < X_RESN/6+1; i++ ){
        room[X_RESN/2+i] = 100;
    }

    struct timeval timeStart, timeEnd, timeSystemStart; 
    double runTime=0, systemRunTime; 
    gettimeofday(&timeStart, NULL );

    for (int count = 0; count < NUMBER_OF_ITERATIONS; count++) {
        int data_index = 0;

        // calculate temperature for each point
        for (int i = 1; i < Y_RESN; i++) {
            for (int j = 1; j < X_RESN; j++) {
                data_index = i*(X_RESN+1) + j;        
                temp[data_index] = (room[(i-1)*(X_RESN+1)+j] + room[(i+1)*(X_RESN+1)+j] + room[data_index-1] + room[data_index+1]) / 4;
            }
        }

        // update temperature
        for (int i = 1; i < Y_RESN; i++) {
            for (int j = 1; j < X_RESN; j++) {
                data_index = i*(X_RESN+1) + j;
                room[data_index] = temp[data_index];
            }
        }

        // if (count % 5 == 0) {
        //     // draw the diagram
        //     for (int i = 0; i < (X_RESN+1)*(Y_RESN+1); i++) {
        //         double temperature = room[i];
        //         int level = (temperature - 20) / 10;
        //         if (level > 7) {
        //             level--;
        //         }
        //         if (level < 0) {
        //             level = 0;
        //         }
        //         XDrawPoint (display, win, gcs[level], i%(X_RESN+1), floor(i/(X_RESN+1)));
        //     }
        // }
        // XFlush (display);
    }

    gettimeofday( &timeEnd, NULL ); 
    runTime = (timeEnd.tv_sec - timeStart.tv_sec ) + (double)(timeEnd.tv_usec -timeStart.tv_usec)/1000000;  

    // printf("Name: Liu Yang\nStudent ID: 116010151\nAssignment 3, N-Body Simulation, Sequential Implementation\n");
    printf("Sequential %d*%d Size RUN TIME is %lf\n", X_RESN, Y_RESN, runTime); 

    // usleep(250000);
    // XFlush(display);
    // printf("finish running\n");
    // sleep(10);
    return 0;
}