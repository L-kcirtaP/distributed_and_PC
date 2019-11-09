#include <iostream>
#include <cstdlib>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <time.h>
#include <math.h>
#include <iostream>

using namespace std;

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

Body collide_border(Body body, int width, int height) {
    if (body.pos_y+body.diameter/2 >= height or body.pos_y-body.diameter/2 <= 0) {
        body.velocity_y = -body.velocity_y;
    }
    if (body.pos_x+body.diameter/2 >= width or body.pos_x-body.diameter/2 <= 0) {
        body.velocity_x = -body.velocity_x;
    }
    return body;
}

bool collide_body(Body b_1, Body b_2) {
    double dx = b_1.pos_x - b_2.pos_x;
    double dy = b_1.pos_y - b_2.pos_y;
    double dist = sqrt(pow(dx, 2) + pow(dy, 2));
    if (2*dist <= b_1.diameter + b_2.diameter) {
        return true;
    } else {
        return false;
    }
}

Body collide(Body b_1, Body b_2) {
    double dx = b_1.pos_x - b_2.pos_x;
    double dy = b_1.pos_y - b_2.pos_y;
    double dist = sqrt(pow(dx, 2) + pow(dy, 2));
    // 球1在球心连线方向上的速度分量
    double v_radius_1 = (b_1.velocity_x*dx + b_1.velocity_y*dy) / dist;
    // 球1在垂直于球心连线方向上的速度分量
    double v_tangent_1 = sqrt(pow(b_1.velocity_x, 2) + pow(b_1.velocity_x, 2) - pow(v_radius_1, 2));                        // 球1在球心连线方向上的速度分量
    // 球2在球心连线方向上的速度分量
    double v_radius_2 = -(b_2.velocity_x*dx + b_2.velocity_y*dy) / dist;

    // 碰撞后，球1在球心连线方向上的新速度分量
    v_radius_1 = (v_radius_1*(b_1.mass-b_2.mass) + 2*b_2.mass*v_radius_2) / (b_1.mass + b_2.mass);

    // 碰撞后，垂直于球心连线方向上的速度分量不变。求出球1新速度：\sqrt{v_radius^2+v_tangent^2}.
    double velocity_new = sqrt(pow(v_radius_1, 2) + pow(v_tangent_1, 2));
    // 将球1速度在水平、竖直方向分解
    b_1.velocity_x = (v_radius_1+v_tangent_1) * dx / dist;
    b_1.velocity_y = (v_tangent_1+v_radius_1) * dy / dist;

    return b_1;
}


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
    int X_RESN = atoi(argv[1]);
    int Y_RESN = atoi(argv[1]);
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

    Body *bodies = (Body *) malloc((NUMBER_OF_BODIES+1)*sizeof(Body));
    srand(time(NULL));

    for (int i = 0; i < NUMBER_OF_BODIES; i++) {
        // printf("%d\n", sizeof(bodies[i]));
        bodies[i].pos_x = rand() % 1200;
        bodies[i].pos_y = rand() % 1200;
        bodies[i].velocity_x = 0;
        bodies[i].velocity_y = 0;
        bodies[i].acceleration_x = 0;
        bodies[i].acceleration_y = 0;
        bodies[i].mass = rand() % 1000 + 2000;
        bodies[i].diameter = 15;
    }
    bodies[NUMBER_OF_BODIES].pos_x = 600;
    bodies[NUMBER_OF_BODIES].pos_y = 600;
    bodies[NUMBER_OF_BODIES].velocity_x = 0;
    bodies[NUMBER_OF_BODIES].velocity_y = 0;
    bodies[NUMBER_OF_BODIES].diameter = 100;
    bodies[NUMBER_OF_BODIES].mass = 500000;

    int iterations_num = 5000;
    for (int count = 0; count < iterations_num; count++) {
        for (int i = 0; i < NUMBER_OF_BODIES+1; i++) {
            // bodies[i] = collide_border(bodies[i], width, height);
            for (int j = 0; j < NUMBER_OF_BODIES+1; j++) {
                if (i != j) {
                    bodies[i] = applyForce(bodies[i], bodies[j]);
                }
            }
        }
        // cout << "Iteration Count: " << count << endl;

        for (int i = 0; i < NUMBER_OF_BODIES; i++) {
            bodies[i] = updatePosition(bodies[i], TIMESTAMP);
            // cout << "Position: " << bodies[i].pos_x << ", " << bodies[i].pos_y << endl; 
        }

        for (int i = 0; i < NUMBER_OF_BODIES; i++) {
            XDrawArc(display, win, gc, bodies[i].pos_x-(bodies[i].diameter/2), bodies[i].pos_y-(bodies[i].diameter/2), bodies[i].diameter, bodies[i].diameter, 0, 360*64);
            usleep(1);
        }
        XFlush(display);
        XClearWindow(display,win);
    }

    usleep(250000);
    XFlush(display);
    cout<<"finish running"<<endl;
    sleep(10);
    return 0;
}