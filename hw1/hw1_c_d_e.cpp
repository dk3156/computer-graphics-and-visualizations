//By Dongje Kim, dk3156
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
using namespace std;

#ifdef __APPLE__  // include Mac OS X verions of headers
#include <GLUT/glut.h>
#else // non-Mac OS X operating systems
#include <GL/glut.h>
#endif

#define XOFF          50
#define YOFF          50
#define WINDOW_WIDTH  600
#define WINDOW_HEIGHT 600

float f = 0.0f;
int c = 0;
int num;
int x, y, r;
vector<vector<int>> pairs;
void partc(int, char**);
void partd(int, char**);
void parte(int, char**);
void display(void);
void myinit(void);
void file_in(vector<vector<int>>& pairs);
void draw_circle(int x, int y, int r);
void reshape(int w, int h);
void idle(void);
void scale_coordinates(vector<vector<int>>& pairs, int max_abs);
int get_max_abs(const vector<vector<int>>& pairs);

/*-----------------
The main function
------------------*/
int main(int argc, char **argv)
{
    
    cout << "Press 1 for part c, 2 for part d, or 3 for part e: " << endl;
    if(cin >> num){
        if (num == 1){
            partc(argc, argv);
        } else if (num == 2){
            partd(argc, argv);
        } else {
            parte(argc, argv);
        }
    }

    return 0;
}

void partc(int argc, char **argv){
    cout << "Specify x, y, r with whitespace: ";
    if (!(cin >> x >> y >> r)){
        cout << endl << "Wrong input format" << endl;
        exit(0);
    }
    glutInit(&argc, argv);
    /* Use both double buffering and Z buffer */
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

    glutInitWindowPosition(XOFF, YOFF);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutCreateWindow("Assignment 1");
    glutDisplayFunc(display);

    /* Function call to handle file input here */
    myinit();
    glutMainLoop();
}

void partd(int argc, char **argv){
    glutInit(&argc, argv);
    /* Use both double buffering and Z buffer */
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

    glutInitWindowPosition(XOFF, YOFF);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutCreateWindow("Assignment 1");
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(idle);

    file_in(pairs);
    myinit();
    glutMainLoop();
}


void parte(int argc, char **argv){
    glutInit(&argc, argv);
    /* Use both double buffering and Z buffer */
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

    glutInitWindowPosition(XOFF, YOFF);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutCreateWindow("Assignment 1");
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(idle);

    file_in(pairs);
    myinit();
    glutMainLoop();
}


//Function for drawing one circle -- origin at(x,y) and radius r, using Breshan's algorithm
void draw_circle(int x, int y, int r)
{
    glPointSize(1.0);                     /* size of each point */
    glBegin(GL_POINTS);
    /*drawing cicles
     xp represents the current x pixel
     yp represents the current y pixel
     d represents the determinant for chossing N or NW
     x, y represents the origin of the circle
     r represents the radius
     */
    int xp = r;
    int yp = 0;
    int d = (-4)*r + 5;
    while (xp != yp)
    {
        if (d >= 0)
        {
            d += (-8) * xp + 8 * yp + 20;
            xp--;
        }
        else
        {
            d += 8 * yp + 12;
        }
        yp++;
        glVertex2i(xp+x, yp+y);
        glVertex2i(-xp+x, yp+y);
        glVertex2i(xp+x, -yp+y);
        glVertex2i(-xp+x, -yp+y);
        
        glVertex2i(yp+x, xp+y);
        glVertex2i(yp+x, -xp+y);
        glVertex2i(-yp+x, xp+y);
        glVertex2i(-yp+x, -xp+y);

    }
    
    glEnd();

}

void read_pairs(vector<vector<int>>& pairs) {
    ifstream infile("input_circles.txt");
    if (!infile) {
        cerr << "Failed to open the file." << endl;
        exit(1);
    }
    
    // Skip the first line.
    string s;
    getline(infile, s);
    
    int x, y, r;
    while (infile >> x >> y >> r) {
        vector<int> pair = {x, y, r};
        pairs.push_back(pair);
    }
}

void scale_coordinates(vector<vector<int>>& pairs, int max_abs)
{
    int w = WINDOW_WIDTH / 2;
    for(int i=0; i<pairs.size(); i++){
        int x = abs(pairs[i][0]);
        int y = abs(pairs[i][1]);
        int r = pairs[i][2];
        if (max_abs > w)
        {
            x = (x * w) / max_abs;
            y = (y * w) / max_abs;
            r = (r * w) / max_abs;
            pairs[i][0] = x;
            pairs[i][1] = y;
            pairs[i][2] = r;
        }
        cout << "circle dimensions: " << x << " " << y << " " << r << endl;
    }
}

int get_max_abs(const vector<vector<int>>& pairs)
{
    int max_abs = 0;
    for(int i=0; i<pairs.size(); i++){
        int x = pairs[i][0];
        int y = pairs[i][1];
        int r = pairs[i][2];
        
        int max_x = max(x+r, abs(x-r));
        int max_y = max(y+r, abs(y-r));
        int max_pair = max(max_x, max_y);
        
        if(max_pair > max_abs){
            max_abs = max_pair;
        }
    }
    return max_abs;
}

/*----------
file_in(): file input function. Modify here.
------------*/
void file_in(vector<vector<int>>& pairs)
{
    read_pairs(pairs);
    int max_abs = get_max_abs(pairs);
    scale_coordinates(pairs, max_abs);
}

/*---------------------------------------------------------------------
display(): This function is called once for _every_ frame.
---------------------------------------------------------------------*/
void display(void)
{
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glColor3f(1.0f, 0.84f, 0.0f);         /* draw in golden yellow */
    
    c += 1;
    if (c >= 50) c = 0;
    if(num == 1){
        draw_circle(x,y,r);
    } else if (num == 2){
        for(int i=0; i<pairs.size(); i++){
            int x = pairs[i][0];
            int y = pairs[i][1];
            int r = pairs[i][2];
            
            draw_circle(x,y,r);
        }
    } else {
        for(int i=0; i<pairs.size(); i++){
            int x = pairs[i][0];
            int y = pairs[i][1];
            int r = pairs[i][2];
            r = c * r / 50;
            
            draw_circle(x,y,r);
        }
    }
    glFlush();                            /* render graphics */
    glutSwapBuffers();                    /* swap buffers */
}

/*---------------------------------------------------------------------
myinit(): Set up attributes and viewing
---------------------------------------------------------------------*/
void myinit()
{
    glClearColor(0.0f, 0.0f, 0.92f, 0.0f);    /* blue background*/

    /* set up viewing */
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, WINDOW_WIDTH, 0.0, WINDOW_HEIGHT);
    glMatrixMode(GL_MODELVIEW);
}

void reshape(int w, int h)
{
    glViewport(0, 0, w, h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, (GLdouble) w, 0.0, (GLdouble) h);
}

void idle(void)
{
    f += 0.02f;
    
    // f += 1.0f;  // smaller number gives a slower but smoother animation

    if (f > 180.0) f = 0.0f;
    
    glutPostRedisplay(); // or call display()
}

