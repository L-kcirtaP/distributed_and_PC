#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#include <iostream>
using namespace std;

#define ROOT 0
#define G 500
#define TIMESTAMP 0.001
#define NUMBER_OF_BODIES 300
#define NUMBER_OF_ITERATIONS 3000

struct Body {
    double pos_x, pos_y;
    double velocity_x, velocity_y;
    double acceleration_x, acceleration_y;
    double mass;
};

int X_RESN, Y_RESN, local_body_num, NUM_THREADS;
Body * bodies;
double *global_body_array;

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

void* nBodySimulation(void* t) {

    int *tid = (int *)t;
    cout << "enter thread " << endl;

    for (int i = 0; i < local_body_num; i++) {
        int body_idx = (*tid)*local_body_num + i;
        for (int j = 0; j < local_body_num*NUM_THREADS; j++) {
            if (i != j) {
                double obj_mass = global_body_array[3*j];
                double obj_pos_x = global_body_array[3*j+1];
                double obj_pos_y = global_body_array[3*j+2];
                bodies[body_idx] = applyForce(bodies[body_idx], obj_mass, obj_pos_x, obj_pos_y);
            }
        }
        bodies[body_idx] = applyForce(bodies[body_idx], 400000, X_RESN/2, Y_RESN/2);
    }

    for (int i = 0; i < local_body_num; i++) {
        int body_idx = (*tid)*local_body_num + i;
        cout << "Update " << body_idx << endl;
        bodies[body_idx] = updatePosition(bodies[body_idx], TIMESTAMP);
        global_body_array[3*body_idx+1] = bodies[body_idx].pos_x;
        global_body_array[3*body_idx+2] = bodies[body_idx].pos_y;
        cout << *tid << "  " << body_idx << ": " << global_body_array[3*body_idx+1] << ", " << global_body_array[3*body_idx+2] << endl;
    }
    
    pthread_exit(NULL);
}


int main (int argc, char* argv[]) {

    NUM_THREADS = atoi(argv[1]);
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

    local_body_num = NUMBER_OF_BODIES / NUM_THREADS;
    if (NUMBER_OF_BODIES % NUM_THREADS) local_body_num++;

    global_body_array = (double *) malloc (3*local_body_num*NUM_THREADS*sizeof(double));
    bodies = (Body *) malloc (local_body_num*NUM_THREADS);

    srand((unsigned)time(NULL));
    for (int i = 0; i < local_body_num*NUM_THREADS; i++) {
        bodies[i].mass = rand() % 3000 + 2000;
        bodies[i].pos_x = rand() % width;
        bodies[i].pos_y = rand() % height;

        global_body_array[3*i] = bodies[i].mass;
        global_body_array[3*i+1] = bodies[i].pos_x;
        global_body_array[3*i+2] = bodies[i].pos_y;
    }

    pthread_t threads[NUM_THREADS];

    struct timeval start_time, end_time; 
    double run_time = 0;
    gettimeofday(&start_time, NULL);

    for (int count = 0; count < NUMBER_OF_ITERATIONS; count++) {

        for (int i = 0; i < NUM_THREADS; i++) {
            thread_ids[i] = i;
            int rc = pthread_create(&threads[i], NULL, nBodySimulation, (void *)&thread_ids[i]);
        }

        for (int i = 0; i < NUM_THREADS; i++) {
            pthread_join(threads[i], NULL);
        }

        for (int i = 0; i < NUMBER_OF_BODIES; i++) {
            // cout << i << ": " << global_body_array[3*i+1] << ", " << global_body_array[3*i+2] << endl;
            XDrawArc(display, win, gc, global_body_array[3*i+1]-5, global_body_array[3*i+2]-5, 10, 10, 0, 360*64);
            usleep(1);
        }

        XFlush(display);
        XClearWindow(display,win);
    }
    
    gettimeofday(&end_time, NULL);

    run_time = (end_time.tv_sec - start_time.tv_sec ) + (double)(end_time.tv_usec - start_time.tv_usec)/1000000;  

    printf("Name: Liu Yang\nStudent ID: 116010151\nAssignment 2, Mandelbrot Set, Pthread Implementation\n");
    printf("%d processes %d*%d resolution RUN TIME is %lf\n", NUM_THREADS, X_RESN, Y_RESN, run_time);

    XFlush(display);
    usleep(250000);
    sleep(10);

    return 0;
}
