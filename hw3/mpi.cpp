#include <stdio.h>
#include <cstdlib>
#include <math.h>
#include <mpi.h>
#include <time.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#define ROOT 0
#define G 500
#define TIMESTAMP 0.001
#define NUMBER_OF_ITERATIONS 1000

struct Body {
    double pos_x, pos_y;
    double velocity_x, velocity_y;
    double acceleration_x, acceleration_y;
    double mass;
};

Body applyForce(Body subject, double obj_mass, double obj_pos_x, double obj_pos_y) {
    double dx = subject.pos_x - obj_pos_x;
    double dy = subject.pos_y - obj_pos_y;
    double dist = sqrt(pow(dx, 2) + pow(dy, 2));
 
    if (dist <= 10) {
        return subject;
    }

    // F = ma, therefore a = F/m = G*M_2/R^2
    double acceleration = G * obj_mass / pow(dist, 2);     /* add a remainder 1 to avoid 0 divisor */
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

    int NUMBER_OF_BODIES = atoi(argv[1]);   /* Argument 1: the number of bodies  */ 
    int X_RESN = atoi(argv[2]);             /* Argument 2: the width and height of the window */ 
    int Y_RESN = atoi(argv[2]);

    int rank, num_p;
    Body *bodies;
    double *global_body_array, *local_body_array;

    struct timeval timeStart, timeEnd, timeSystemStart;
    double run_time = 0, systemRunTime;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &num_p);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Window          win;       
    // char            *window_name = "test", *display_name = NULL;                     /* initialization for a window */
    // Display         *display;
    // GC              gc;   //this is a graphic content, it could be a pixel color
    // unsigned long   valuemask = 0;
    // XGCValues       values; //value of the graphics content
    // XSizeHints      size_hints;
    // Pixmap          bitmap;
    // XSetWindowAttributes attr[1];
    // int             width, height,                  /* window size */
    //                 x, y,                           /* window position */
    //                 border_width,                   /*border width in pixels */
    //                 display_width, display_height,  /* size of screen */
    //                 screen;                         /* which screen */

    // width = X_RESN;
    // height = Y_RESN;

    // if (rank == ROOT) {
    //     if (  (display = XOpenDisplay (display_name)) == NULL ) {
    //         fprintf (stderr, "drawon: cannot connect to X server %s\n",
    //                                 XDisplayName (display_name) );
    //         exit (-1);
    //     }

    //     /* get screen size */
    //     screen = DefaultScreen (display);
    //     display_width = DisplayWidth (display, screen);
    //     display_height = DisplayHeight (display, screen);

    //     /* set window size */
    //     width = X_RESN;
    //     height = Y_RESN;

    //     /* set window position */

    //     x = 0;
    //     y = 0;

    //     /* create opaque window */

    //     border_width = 4;
    //     win = XCreateSimpleWindow (display, RootWindow (display, screen),
    //                         x, y, width, height, border_width, 
    //                         WhitePixel (display, screen), WhitePixel (display, screen)); //Change to WhitePixel (display, screen) if you want a white background

    //     size_hints.flags = USPosition|USSize;
    //     size_hints.x = x;
    //     size_hints.y = y;
    //     size_hints.width = width;
    //     size_hints.height = height;
    //     size_hints.min_width = 300;
    //     size_hints.min_height = 300;

    //     XSetNormalHints (display, win, &size_hints);
    //     XStoreName(display, win, window_name);

    //     /* create graphics context */
    //     gc = XCreateGC (display, win, valuemask, &values);
    //     XSetBackground (display, gc, BlackPixel (display, screen));
    //     XSetForeground (display, gc, WhitePixel (display, screen));
    //     XSetLineAttributes (display, gc, 1, LineSolid, CapRound, JoinRound);

    //     attr[0].backing_store = Always;
    //     attr[0].backing_planes = 1;
    //     attr[0].backing_pixel = BlackPixel (display, screen);

    //     XChangeWindowAttributes(display, win, CWBackingStore | CWBackingPlanes | CWBackingPixel, attr);

    //     XMapWindow (display, win);
    //     XSync(display, 0);

    //     XFlush (display);

    //     XColor color;
    //     color.red=0;    //range from 0~65535
    //     color.green=0;
    //     color.blue=0;

    //     Status rc1 = XAllocColor(display, DefaultColormap(display, screen), &color);
    //     //set the color and attribute of the graphics content
    //     XSetForeground (display, gc, color.pixel);
    //     XSetBackground (display, gc, BlackPixel (display, screen));
    //     XSetLineAttributes (display, gc, 1, LineSolid, CapRound, JoinRound);
    // }

    int local_body_num = NUMBER_OF_BODIES / num_p;
    if (NUMBER_OF_BODIES % num_p) local_body_num++;
    
    Body * local_bodies = (Body *) malloc (local_body_num*sizeof(Body));
    local_body_array = (double *) malloc (3*local_body_num*sizeof(double));
    global_body_array = (double *) malloc (3*local_body_num*num_p*sizeof(double));

    srand((unsigned)time(NULL)+rank*num_p);
    for (int i = 0; i < local_body_num; i++) {
        local_bodies[i].mass = rand() % 3000 + 2000;
        local_bodies[i].pos_x = rand() % width;
        local_bodies[i].pos_y = rand() % height;

        local_body_array[3*i] = local_bodies[i].mass;
        local_body_array[3*i+1] = local_bodies[i].pos_x;
        local_body_array[3*i+2] = local_bodies[i].pos_y;
    }

    MPI_Allgather(local_body_array, 3*local_body_num, MPI_DOUBLE, global_body_array, 3*local_body_num, MPI_DOUBLE, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);

    double start_time = MPI_Wtime();

    // start iteration
    for (int count = 0; count < NUMBER_OF_ITERATIONS; count++) {
        for (int i = 0; i < local_body_num; i++) {
            for (int j = 0; j < local_body_num*num_p; j++) {
                if (i != j) {
                    double obj_mass = global_body_array[3*j];
                    double obj_pos_x = global_body_array[3*j+1];
                    double obj_pos_y = global_body_array[3*j+2];
                    local_bodies[i] = applyForce(local_bodies[i], obj_mass, obj_pos_x, obj_pos_y);
                }
            }
            local_bodies[i] = applyForce(local_bodies[i], 400000, width/2, height/2);
        }

        for (int i = 0; i < local_body_num; i++) {
            local_bodies[i] = updatePosition(local_bodies[i], TIMESTAMP);
            local_body_array[3*i+1] = local_bodies[i].pos_x;
            local_body_array[3*i+2] = local_bodies[i].pos_y;
        }

        MPI_Allgather(local_body_array, 3*local_body_num, MPI_DOUBLE, global_body_array, 3*local_body_num, MPI_DOUBLE, MPI_COMM_WORLD);
        MPI_Barrier(MPI_COMM_WORLD);

        // if (rank == ROOT) {
        //     for (int i = 0; i < NUMBER_OF_BODIES; i++) {
        //         XDrawArc(display, win, gc, global_body_array[3*i+1]-5, global_body_array[3*i+2]-5, 10, 10, 0, 360*64);
        //         usleep(1);
        //     }
        //     XFlush(display);
        //     XClearWindow(display,win);
        // }
    }

    double finish_time = MPI_Wtime();

    if (rank == ROOT) {
        printf("Name: Liu Yang\nStudent ID: 116010151\nAssignment 3, N-Body Simulation, MPI Implementation\n");
        printf("MPI %d Processes %d Bodies RUN TIME is %lf\n", num_p, NUMBER_OF_BODIES, finish_time-start_time); 
    }

    MPI_Finalize();
    return 0;
}