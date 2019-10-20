#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#define MAX_ITER 100
#define MAX_LENGTH 12

typedef struct complextype {
    float real, imag;
} Compl;

pthread_mutex_t mutex_lock;				/* mutual exclusive lock */ 
int X_RESN, Y_RESN, local_row_num, *results;


void* cal_row(void* t) {

    Compl z, c;
    int row, loop_count;
    double lengthsq, temp;

    int *tid = (int *)t;

    for (int i = 0; i < local_row_num; i++) {
        int row = *tid * local_row_num + i;

        for (int col = 0; col < X_RESN; col++) {
            z.real = z.imag = 0.0;
            c.real = ((float) row - Y_RESN/2)/(Y_RESN/4);                //scale factors for 800 x 800 window 
            c.imag = ((float) col - X_RESN/2)/(X_RESN/4);
            loop_count = 0;

            do  {                                             // iterate for pixel color
                temp = z.real*z.real - z.imag*z.imag + c.real;
                z.imag = 2.0*z.real*z.imag + c.imag;
                z.real = temp;

                lengthsq = z.real*z.real + z.imag*z.imag;
                loop_count++;
            } while (lengthsq < MAX_LENGTH && loop_count < MAX_ITER); //lengthsq and loop_count are the threshold

            if (loop_count >= MAX_ITER) {
                results[row*X_RESN + col] = 1;
            }
        }
    }

    pthread_exit(NULL);
}


int main (int argc, char* argv[]) {

    int NUM_THREADS = atoi(argv[1]);
    X_RESN = atoi(argv[2]);
    Y_RESN = atoi(argv[2]);
    
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
    width = X_RESN;
    height = Y_RESN;

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

    XFlush (display);

    XColor color;
    color.red=10000; //range from 0~65535
    color.green=10000;
    color.blue=10000;

    Status rc1=XAllocColor(display,DefaultColormap(display, screen),&color);
    //set the color and attribute of the graphics content
    XSetForeground (display, gc, color.pixel);
    XSetBackground (display, gc, BlackPixel (display, screen));
    XSetLineAttributes (display, gc, 1, LineSolid, CapRound, JoinRound);

    int *thread_ids = (int *) malloc(sizeof(int) * NUM_THREADS);

    pthread_t thread[NUM_THREADS];
	pthread_mutex_init(&mutex_lock, NULL);

    local_row_num = Y_RESN / NUM_THREADS;
    if (Y_RESN % NUM_THREADS) local_row_num++;

    results = (int *) malloc(sizeof(int) * X_RESN * local_row_num * NUM_THREADS);

    struct timeval start_time, end_time; 
    double run_time = 0;
    gettimeofday(&start_time, NULL);

    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        int rc = pthread_create(&thread[i], NULL, cal_row, (void *)&thread_ids[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(thread[i], NULL);
    }
    
    gettimeofday(&end_time, NULL);

    run_time = (end_time.tv_sec - start_time.tv_sec ) + (double)(end_time.tv_usec - start_time.tv_usec)/1000000;  

    // printf("Name: Liu Yang\nStudent ID: 116010151\nAssignment 2, Mandelbrot Set, Pthread Implementation\n");
    printf("%d processes %d*%d resolution RUN TIME is %lf\n", NUM_THREADS, X_RESN, Y_RESN, run_time); 

    // for (int i = 0; i < X_RESN; i++) {
    //     for (int j = 0; j < Y_RESN; j++){
    //         if (results[j*X_RESN + i] == 1) {
    //             XDrawPoint(display, win, gc, j, i);
    //             usleep(1);
    //         }
    //     }
    // }

    // XFlush(display);
    // usleep(250000);
    // sleep(10);

    return 0;
}
