
#include <stdio.h>
#include <cstdlib>
#include <math.h>
#include <mpi.h>
#include <time.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#define G 500
#define TIMESTAMP 0.001
#define NUMBER_OF_BODIES 200

struct Body {
    double pos_x, pos_y;
    double velocity_x, velocity_y;
    double acceleration_x, acceleration_y;
    double mass;
    double diameter;
};

Body applyForce(Body subject, Body object) {
    double dx = subject.pos_x - object.pos_x;
    double dy = subject.pos_y - object.pos_y;
    double dist = sqrt(pow(dx, 2) + pow(dy, 2));
 
    if (2*dist <= subject.diameter + object.diameter) {
        return subject;
    }

    // F = ma, therefore a = F/m = G*M_2/R^2
    double acceleration = G * object.mass / (pow(dist, 2) + 1);     /* add a remainder 1 to avoid 0 divisor */
    subject.acceleration_x = -dx * acceleration / dist;
    subject.acceleration_y = -dy * acceleration / dist;

    return subject;
}

Body updatePosition(Body body, double duration) {
    body.velocity_x += body.acceleration_x * duration;
    body.velocity_y += body.acceleration_y * duration;
    body.pos_x += body.velocity_x * duration;
    body.pos_y += body.velocity_y * duration;

    return body;
}

int main (int argc, char* argv[]){
    int X_RESN = atoi(argv[1]);
    int Y_RESN = atoi(argv[1]);

    int rank, num_p;
    struct timeval timeStart, timeEnd, timeSystemStart;
    double run_time = 0, systemRunTime;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &num_p);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

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
    color.red=0;    //range from 0~65535
    color.green=0;
    color.blue=0;

    Status rc1 = XAllocColor(display, DefaultColormap(display, screen), &color);
    //set the color and attribute of the graphics content
    XSetForeground (display, gc, color.pixel);
    XSetBackground (display, gc, BlackPixel (display, screen));
    XSetLineAttributes (display, gc, 1, LineSolid, CapRound, JoinRound);
    

    // Computation part
    Compl z, c;
    int row, loop_count;
    double lengthsq, temp;

    double start_time = MPI_Wtime();

    for (int i = 0; i < local_row_num; i++) {
        row = rank * local_row_num + i;
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
                local_result[i*X_RESN + col] = 1;
            }
        }
    }

    if (rank == ROOT) global_result = (int*) malloc(sizeof(int) * local_result_sz * num_p);

    MPI_Gather(local_result, local_result_sz, MPI_INT, global_result, local_result_sz, MPI_INT, ROOT, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);

    double finish_time = MPI_Wtime();

    if (rank == ROOT) {
        printf("Name: Liu Yang\nStudent ID: 116010151\nAssignment 2, Mandelbrot Set, MPI Implementation\n");
        printf("%d process %d*%d resolution RUN TIME is %lf\n", num_p, X_RESN, Y_RESN, finish_time-start_time); 

        for (int i = 0; i < X_RESN; i++) {
            for (int j = 0; j < Y_RESN; j++){
                if (global_result[j*X_RESN + i] == 1) {
                    XDrawPoint(display, win, gc, j, i);
                    usleep(1);
                }
            }
        }
        XFlush(display);
        usleep(250000);
        sleep(10);
    }

    MPI_Finalize();
    return 0;
}