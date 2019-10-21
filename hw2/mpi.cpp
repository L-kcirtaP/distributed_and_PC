#include <stdio.h>
#include <cstdlib>
#include <math.h>
#include <mpich/mpi.h>
#include <time.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <typeinfo>

#define ROOT 0
#define X_RESN 200
#define Y_RESN 200
#define MAX_ITER 100
#define MAX_LENGTH 12


typedef struct complextype {
    float real, imag;
} Compl;

typedef enum { DATA_TAG, TERM_TAG, RESULT_TAG } Tags;

int main (int argc, char* argv[]){

    int rank, num_p;
    struct timeval timeStart, timeEnd, timeSystemStart;
    double run_time = 0, systemRunTime;

    int* output = (int*) malloc(sizeof(int) * (X_RESN*Y_RESN));

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_p);


    Window          win;       
    char            *window_name = "test", *display_name = NULL;                     /* initialization for a window */
    Display         *display;
    GC              gc;   //this is a graphic content, it could be a pixel color
    unsigned long   valuemask = 0;
    XGCValues       values; //value of the graphics content
    XSizeHints      size_hints;
    Pixmap          bitmap;
    XSetWindowAttributes attr[1];
    int             width, height,                  /* window size */
                    x, y,                           /* window position */
                    border_width,                   /*border width in pixels */
                    display_width, display_height,  /* size of screen */
                    screen;                         /* which screen */

    if (rank == ROOT) {

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
        color.red=10000;    //range from 0~65535
        color.green=10000;
        color.blue=10000;

        Status rc1=XAllocColor(display,DefaultColormap(display, screen),&color);
        //set the color and attribute of the graphics content
        XSetForeground (display, gc, color.pixel);
        XSetBackground (display, gc, BlackPixel (display, screen));
        XSetLineAttributes (display, gc, 1, LineSolid, CapRound, JoinRound);

        Compl z, c;
        int i, j, process_count, row, working_p;

        gettimeofday(&timeStart, NULL);

        row = 0;
        working_p = 0;

        for (process_count = 1; process_count < num_p; process_count++) {
            MPI_Send(&row, 1, MPI_INT, process_count, DATA_TAG, MPI_COMM_WORLD);
            // printf("Sent row %d task to process %d.\n", row, process_count);
            row++;
            working_p++;
        }

        MPI_Status status;

        while (working_p > 0) {
            MPI_Recv(&output[row*X_RESN], X_RESN, MPI_INT, MPI_ANY_SOURCE, RESULT_TAG, MPI_COMM_WORLD, &status);

            // for (int i = 0; i < X_RESN; i++) {
            //     if (output[row*X_RESN+i] == 1){
            //         printf("row %d col %d\n", row, i);
            //     }
            // }

            row++;
            working_p--;
            
            if (row < Y_RESN) {
                MPI_Send(&row, 1, MPI_INT, status.MPI_SOURCE, DATA_TAG, MPI_COMM_WORLD);
                working_p++;
            } else {
                MPI_Send(&row, 1, MPI_INT, status.MPI_SOURCE, TERM_TAG, MPI_COMM_WORLD);
            }
        }

    } else {
        MPI_Status status;
        int row, loop_count;
        double temp, lengthsq;
        Compl c, z;
        
        MPI_Recv(&row, 1, MPI_INT, ROOT, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        // printf("Process %d receives from root\n", rank);

        while (status.MPI_TAG == DATA_TAG){
            c.imag = ((float)row - Y_RESN/2)/(Y_RESN/4);

            // printf("Process %d is computing row %d...\n", rank, row);

            for (int col = 0; col < X_RESN; col++) {
                z.real = 0;
                z.imag = 0;
                loop_count = 0;

                c.real = ((float)col - Y_RESN/2)/(Y_RESN/4);
                do {
                    temp = z.real*z.real - z.imag*z.imag + c.real;
                    z.imag = 2*z.real*z.real + c.imag;
                    z.real = temp;

                    lengthsq = z.real*z.real + z.imag*z.imag;
                    loop_count++;
                } while (lengthsq < MAX_LENGTH && loop_count < MAX_ITER);
                
                if (loop_count >= MAX_ITER) {
                    printf("(%d, %d)\n", row, col);
                    *(&output[row*X_RESN] + col) = 1;
                    // printf("(%d, %d): %d\n", row, col, output[row*X_RESN+col]);
                }
            }

            MPI_Send(&output[row*X_RESN], X_RESN, MPI_INT, ROOT, RESULT_TAG, MPI_COMM_WORLD);
            MPI_Recv(&row, 1, MPI_INT, ROOT, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);

    if (rank == ROOT) {
        gettimeofday(&timeEnd, NULL); 
        run_time = (timeEnd.tv_sec - timeStart.tv_sec ) + (double)(timeEnd.tv_usec - timeStart.tv_usec) / 1000000;  
        printf("Running time: %lf\n", run_time); 
        
        for (int i = 0; i < X_RESN; i++) {
            for (int j = 0; j < Y_RESN; j++){
                if (output[j*X_RESN + i] == 1) {
                    // printf("DRAW x = %d y = %d\n", j, i);
                    XDrawPoint(display, win, gc, j, i);
                    usleep(1);
                }
            }
        }
        XFlush(display);
        usleep(250000);
        printf("finish running\n");
        sleep(10);
    }

    MPI_Finalize();
    return 0;
}