#include <iostream>
#include <cstdlib>
#include <omp.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <time.h>
#include <math.h>

#define G 500
#define TIMESTAMP 0.001
#define NUMBER_OF_ITERATIONS 100

struct Body {
    double pos_x, pos_y;
    double velocity_x, velocity_y;
    double acceleration_x, acceleration_y;
    double mass;
};

Body applyForce(Body subject, Body object) {
    double dx = subject.pos_x - object.pos_x;
    double dy = subject.pos_y - object.pos_y;
    double dist = sqrt(pow(dx, 2) + pow(dy, 2));
 
    if (dist <= 10) {
        return subject;
    }

    // F = ma, therefore a = F/m = G*M_2/R^2
    double acceleration = G * object.mass / pow(dist, 2);     /* add a remainder 1 to avoid 0 divisor */
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
    int num_t = atoi(argv[1]);
    int NUMBER_OF_BODIES = atoi(argv[2]);   /* Argument 1: the number of bodies  */ 
    int X_RESN = atoi(argv[3]);             /* Argument 2: the width and height of the window */ 
    int Y_RESN = atoi(argv[3]);

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
    //                       WhitePixel (display, screen), BlackPixel (display, screen)); //Change to WhitePixel (display, screen) if you want a white background

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
    // attr[0].backing_pixel = WhitePixel (display, screen);

    // XChangeWindowAttributes(display, win, CWBackingStore | CWBackingPlanes | CWBackingPixel, attr);

    // XMapWindow (display, win);
    // XSync(display, 0);

    // XFlush (display);

    // XColor color;
    // color.red=10000; //range from 0~65535
    // color.green=10000;
    // color.blue=10000;

    // Status rc1=XAllocColor(display,DefaultColormap(display, screen),&color);
    // //set the color and attribute of the graphics content
    // XSetForeground (display, gc, color.pixel);
    // XSetBackground (display, gc, WhitePixel (display, screen));
    // XSetLineAttributes (display, gc, 1, LineSolid, CapRound, JoinRound);

    Body *bodies = (Body *) malloc((NUMBER_OF_BODIES+1)*sizeof(Body));
    srand(time(NULL));

    for (int i = 0; i < NUMBER_OF_BODIES; i++) {
        // printf("%d\n", sizeof(bodies[i]));
        bodies[i].pos_x = rand() % X_RESN;
        bodies[i].pos_y = rand() % Y_RESN;
        bodies[i].mass = rand() % 3000 + 2000;
    }
    bodies[NUMBER_OF_BODIES].pos_x = X_RESN/2;
    bodies[NUMBER_OF_BODIES].pos_y = Y_RESN/2;
    bodies[NUMBER_OF_BODIES].mass = 400000;

    struct timeval timeStart, timeEnd, timeSystemStart; 
    double runTime=0, systemRunTime; 

    omp_set_num_threads(num_t);

    gettimeofday(&timeStart, NULL );

    for (int count = 0; count < NUMBER_OF_ITERATIONS; count++) {
        
        #pragma omp parallel for
        for (int i = 0; i < NUMBER_OF_BODIES+1; i++) {
            // bodies[i] = collide_border(bodies[i], width, height);
            for (int j = 0; j < NUMBER_OF_BODIES+1; j++) {
                if (i != j) {
                    bodies[i] = applyForce(bodies[i], bodies[j]);
                }
            }
        }

        #pragma omp parallel for
        for (int i = 0; i < NUMBER_OF_BODIES; i++) {
            bodies[i] = updatePosition(bodies[i], TIMESTAMP);
        }

        // for (int i = 0; i < NUMBER_OF_BODIES; i++) {
        //     XDrawArc(display, win, gc, bodies[i].pos_x-5, bodies[i].pos_y-5, 10, 10, 0, 360*64);
        //     usleep(1);
        // }
        // XFlush(display);
        // XClearWindow(display,win);
    }

    gettimeofday( &timeEnd, NULL ); 
    runTime = (timeEnd.tv_sec - timeStart.tv_sec ) + (double)(timeEnd.tv_usec -timeStart.tv_usec)/1000000;  

    printf("Name: Liu Yang\nStudent ID: 116010151\nAssignment 3, N-Body Simulation, OpenMP Implementation\n");
    printf("OpenMP %d Threads %d Bodies RUN TIME is %lf\n", num_t, NUMBER_OF_BODIES, runTime); 

    // usleep(250000);
    // XFlush(display);
    // sleep(10);
    return 0;
}