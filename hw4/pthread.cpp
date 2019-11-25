#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#define ROOT 0
#define NUMBER_OF_ITERATIONS 3000


int X_RESN, Y_RESN, local_row_num, NUM_THREADS;
double *room, *temp;

void* heatSimulation(void* t) {
    int *tid = (int *)t;

    for (int i = 0; i < local_row_num; i++) {
        int row = (*tid)*local_row_num + i + 1;
        for (int j = 1; j < X_RESN; j++) {
            temp[row*(X_RESN+1)+j] = 0.25 * (room[(row-1)*(X_RESN+1)+j] + room[(row+1)*(X_RESN+1)+j] + room[row*(X_RESN+1)+j-1] + room[row*(X_RESN+1)+j+1]);
            // printf("Computed: row %d col %d TEMP %2f\n", row, j, temp[row*(X_RESN+1)+j]);
        }
    }

    for (int i = 0; i < local_row_num; i++) {
        int row = (*tid)*local_row_num + i + 1;
        for (int j = 1; j < X_RESN; j++) {
            room[row*(X_RESN+1)+j] = temp[row*(X_RESN+1)+j];
            // printf("Updated: col %d row %d TEMP %2f\n", row, j, room[row*(X_RESN+1)+j]);
        }
    }

    pthread_exit(NULL);
}


int main (int argc, char* argv[]) {

    NUM_THREADS = atoi(argv[1]);        /* Argument 1: the number of threads */
    X_RESN = atoi(argv[2]);             /* Argument 3: the width and height of the window */ 
    Y_RESN = atoi(argv[2]);
    
    // Window          win;       
    // char            *window_name = "test", *display_name = NULL;                     /* initialization for a window */
    // Display         *display;
    // GC              gc;   //this is a graphic content, it could be a pixel color
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
    //     fprintf (stderr, "drawon: cannot connect to X server %s\n",
    //                         XDisplayName (display_name) );
    //     exit (-1);
    // }

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
    // gc = XCreateGC (display, win, valuemask, &values);
    // XSetBackground (display, gc, BlackPixel (display, screen));
    // XSetForeground (display, gc, WhitePixel (display, screen));
    // XSetLineAttributes (display, gc, 1, LineSolid, CapRound, JoinRound);

    // attr[0].backing_store = Always;
    // attr[0].backing_planes = 1;
    // attr[0].backing_pixel = BlackPixel (display, screen);

    // XChangeWindowAttributes(display, win, CWBackingStore | CWBackingPlanes | CWBackingPixel, attr);

    // XMapWindow (display, win);
    // XSync(display, 0);

    // XFlush (display);

    // XColor color;
    // color.red=50000; //range from 0~65535
    // color.green=10000;
    // color.blue=10000;

    // Status rc1=XAllocColor(display,DefaultColormap(display, screen),&color);
    // //set the color and attribute of the graphics content
    // XSetForeground (display, gc, color.pixel);
    // XSetBackground (display, gc, BlackPixel (display, screen));
    // XSetLineAttributes (display, gc, 1, LineSolid, CapRound, JoinRound);


    int *thread_ids = (int *) malloc(sizeof(int) * NUM_THREADS);
    pthread_t threads[NUM_THREADS];

    local_row_num = (Y_RESN-1) / NUM_THREADS;
    if ((Y_RESN-1) % NUM_THREADS != 0) {
        local_row_num++;
    }
    
    temp = (double *) malloc ((X_RESN+1)*(local_row_num*NUM_THREADS+2)*sizeof(double));
    room = (double *) malloc ((X_RESN+1)*(local_row_num*NUM_THREADS+2)*sizeof(double));

    for (int i = 0; i < (X_RESN+1)*(Y_RESN+1); i++) {
        room[i] = 20;
    }
    // set fireplace
    for (int i = -X_RESN/20; i < X_RESN/20+1; i++ ){
        room[X_RESN/2+i] = 100;
    }

    struct timeval start_time, end_time;
    double run_time = 0;
    gettimeofday(&start_time, NULL);

    for (int count = 0; count < NUMBER_OF_ITERATIONS; count++) {
        
        for (int i = 0; i < NUM_THREADS; i++) {
            thread_ids[i] = i;
            pthread_create(&threads[i], NULL, heatSimulation, (void *)&thread_ids[i]);
        }

        for (int i = 0; i < NUM_THREADS; i++) {
            pthread_join(threads[i], NULL);
        }

        // for (int i = 0; i < Y_RESN; i++) {
        //     for (int j = 0; j < X_RESN + 1; j++) {
        //         if (room[i*(X_RESN+1)+j] > 25) 
        //             XDrawPoint(display, win, gc, j, i);
        //     }
        // }

        // // lower wall
        // for (int j = 0; j < X_RESN + 1; j++) {
        //     double temperature = 20;
        // }

        // XFlush(display);
    }
    
    gettimeofday(&end_time, NULL);

    run_time = (end_time.tv_sec - start_time.tv_sec ) + (double)(end_time.tv_usec - start_time.tv_usec)/1000000;  

    // printf("Name: Liu Yang\nStudent ID: 116010151\nAssignment 3, N-Body Simulation, Pthreads Implementation\n");
    printf("Pthreads %d Threads %d*%d Size RUN TIME is %lf\n", NUM_THREADS, X_RESN, Y_RESN, run_time);

    // XFlush(display);
    // usleep(250000);
    // sleep(10);
    return 0;
}
