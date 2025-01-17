#include <cstdlib>
#include <math.h>
#include <stdio.h>
#include <mpi.h>
#include <time.h>
#include <assert.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#define ROOT 0
#define NUMBER_OF_ITERATIONS 50000


int main (int argc, char* argv[]){

    int X_RESN = atoi(argv[1]);             /* Argument 2: the width and height of the window */ 
    int Y_RESN = atoi(argv[1]);

    int rank, num_p;
    double *global_array, *local_array, *local_temp, *up_row, *down_row;

    struct timeval timeStart, timeEnd, timeSystemStart;
    double run_time = 0, systemRunTime;
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &num_p);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    Window          win;       
    char            *window_name = "test", *display_name = NULL;                     /* initialization for a window */
    Display         *display;
    GC              gc;   //this is a graphic content, it could be a pixel color
    GC*             gcs;
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

    width = X_RESN;
    height = Y_RESN;

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
        gcs = (GC *) malloc ((100-20)/10*sizeof(GC));
        for (int i = 0; i < (100-20)/10; i++) {
            gc = XCreateGC (display, win, valuemask, &values);
            gcs[i] = gc;
        }

        attr[0].backing_store = Always;
        attr[0].backing_planes = 1;
        attr[0].backing_pixel = BlackPixel (display, screen);

        XChangeWindowAttributes(display, win, CWBackingStore | CWBackingPlanes | CWBackingPixel, attr);

        XMapWindow (display, win);
        XSync(display, 0);

        XFlush (display);

        XColor color;
        for (int i = 0; i < (100-20)/10; i++) {
            if (i == 0) {
                color.red = 0;
                color.green = 0;
                color.blue = 65535;  
            } else if (i == 7) {
                color.red = 65535;
                color.green = 0;
                color.blue = 0;
            } else {
                color.red = i*8191;
                color.green = i*7000;
                color.blue = 28000-i*4000;
            }
            Status rc1=XAllocColor(display, DefaultColormap(display, screen), &color);
            XSetForeground (display, gcs[i], color.pixel);
            XSetBackground (display, gcs[i], BlackPixel (display, screen));
            XSetLineAttributes (display, gcs[i], 1, LineSolid, CapRound, JoinRound);
        }
    }

    int local_row_num = Y_RESN-1 / num_p;
    if ((Y_RESN-1) % num_p) local_row_num++;
    
    local_temp = (double *) malloc (local_row_num*(Y_RESN+1)*sizeof(double));
    local_array = (double *) malloc (local_row_num*(Y_RESN+1)*sizeof(double));
    global_array = (double *) malloc (local_row_num*(Y_RESN+1)*num_p*sizeof(double));

    up_row = (double *) malloc ((Y_RESN+1)*sizeof(double));
    down_row = (double *) malloc ((Y_RESN+1)*sizeof(double));

    for (int i = 0; i < local_row_num; i++) {
        for (int j = 0; j < X_RESN+1; j++) {
            local_array[i*(X_RESN+1)+j] = 20;
        }
    }

    double start_time = MPI_Wtime();

    if (rank == ROOT) {
        for (int i = 0; i < X_RESN+1; i++) {
            up_row[i] = 20;
        }

        for (int i = -X_RESN/6; i < X_RESN/6+1; i++ ){
            up_row[X_RESN/2+i] = 100;
        }
    }

    if (rank == num_p-1) {
        for (int i = 0; i < X_RESN+1; i++) {
            down_row[i] = 20;
        }
    }

    // start iteration
    for (int count = 0; count < NUMBER_OF_ITERATIONS; count++) {

        // communicate with boundary row of neighbor processes
        if (rank > ROOT) {
            MPI_Send(&local_array[0], X_RESN+1, MPI_DOUBLE, rank-1, 0, MPI_COMM_WORLD);
        }

        if (rank < num_p-1) {
            MPI_Send(&local_array[(local_row_num-1)*(X_RESN+1)], X_RESN+1, MPI_DOUBLE, rank+1, 0, MPI_COMM_WORLD);
            MPI_Recv(&down_row[0], X_RESN+1, MPI_DOUBLE, rank+1, 0, MPI_COMM_WORLD, &status);
        }

        if (rank > ROOT) {
            MPI_Recv(&up_row[0], X_RESN+1, MPI_DOUBLE, rank-1, 0, MPI_COMM_WORLD, &status);
        }

        MPI_Barrier(MPI_COMM_WORLD);

        // calculate temperature
        for (int i = 0; i < local_row_num; i++) {
            for (int j = 1; j < X_RESN; j++) {
                if (i == 0) {
                    local_temp[i*(X_RESN+1)+j] = (up_row[j] + local_array[(i+1)*(X_RESN+1)+j] + local_array[i*(X_RESN+1)+j-1] + local_array[i*(X_RESN+1)+j+1])/4;
                } else if (i == local_row_num - 1) {
                    local_temp[i*(X_RESN+1)+j] = (local_array[(i-1)*(X_RESN+1)+j] + down_row[j] + local_array[i*(X_RESN+1)+j-1] + local_array[i*(X_RESN+1)+j+1])/4;
                } else {
                    local_temp[i*(X_RESN+1)+j] = (local_array[(i-1)*(X_RESN+1)+j] + local_array[(i+1)*(X_RESN+1)+j] + local_array[i*(X_RESN+1)+j-1] + local_array[i*(X_RESN+1)+j+1])/4;
                }
            }
        }

        // update temperature
        for (int i = 0; i < local_row_num; i++) {
            for (int j = 1; j < X_RESN; j++) {
                local_array[i*(X_RESN+1)+j] = local_temp[i*(X_RESN+1)+j];
            }
        }

        if (count % 5 == 0) {
            MPI_Gather(local_array, local_row_num*(Y_RESN+1), MPI_DOUBLE, global_array, local_row_num*(Y_RESN+1), MPI_DOUBLE, ROOT, MPI_COMM_WORLD);
            MPI_Barrier(MPI_COMM_WORLD);

            if (rank == ROOT) {

                // upper wall
                for (int i = 0; i < X_RESN+1; i++) {
                    double temperature = 20;
                    if (i == (int)(floor((X_RESN+1)/2))) {
                        temperature = 100;
                    }
                    int level = (temperature - 20) / 10;
                    if (level > 7) {
                        level--;
                    }
                    if (level < 0) {
                        level = 0;
                    }
                    XDrawPoint (display, win, gcs[level], i%(X_RESN+1), floor(i/(X_RESN+1)));
                }

                for (int i = 0; i < Y_RESN - 1; i++) {
                    for (int j = 0; j < X_RESN + 1; j++) {
                        double temperature = global_array[i*(X_RESN+1)+j];
                        int level = (temperature - 20) / 10;
                        if (level > 7) {
                            level--;
                        }
                        if (level < 0) {
                            level = 0;
                        }
                        XDrawPoint (display, win, gcs[level], j, i);
                    }
                }

                // lower wall
                if (local_row_num*num_p == Y_RESN-1) {
                    for (int i = 0; i < X_RESN+1; i++) {
                        int temperature = 20;
                        XDrawPoint (display, win, gcs[0], Y_RESN, i);
                    }
                }
                XFlush(display);
            }
        }
    }

    double finish_time = MPI_Wtime();

    if (rank == ROOT) {
        // printf("Name: Liu Yang\nStudent ID: 116010151\nAssignment 3, N-Body Simulation, MPI Implementation\n");
        printf("MPI %d Processes %d*%d Size RUN TIME is %lf\n", num_p, X_RESN, Y_RESN, finish_time-start_time); 
    }

    // MPI_Finalize();
    return 0;
}