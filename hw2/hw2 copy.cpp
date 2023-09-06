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

#include "Angel-yjc.h"

typedef Angel::vec3  color3;
typedef Angel::vec3  point3;

GLuint Angel::InitShader(const char* vShaderFile, const char* fShaderFile);

GLuint program; //buffers for program, sphere, floor and axis that is bound in display() function
GLuint tri_buffer;
GLuint floor_buffer;
GLuint axis_buffer;

GLfloat  fovy = 45.0;  // Field-of-view in Y direction angle (in degrees)
GLfloat  aspect;       // Viewport aspect ratio
GLfloat  zNear = 0.5, zFar = 20.0;

GLfloat angle = 0.0; // rotation angle in degrees
vec4 init_eye(7.0, 3.0, -10.0, 1.0); // initial viewer position
vec4 eye = init_eye;               // current viewer position

int startFlag = 0; // 1: starts animation. Toggled by key 'b' or 'B'
int lineFlag = 0; //Toggles between wireframe and color-filled object
int mouseFlag = 0; // 0: stops animation, 1: resume animation
int animationFlag = 0; // 0: animation from A to B. 1: animation from B to C. 2: animation from C to A

int tri_NumVertices; // 3 * 128 number of vertices for sphere

point3* sphere_points; //array to store sphere vertices
color3* sphere_colors; //array to store sphere colors

const int floor_NumVertices = 6; // 3 * 2
point3 floor_points[floor_NumVertices]; // array to store floor vertices
color3 floor_colors[floor_NumVertices]; // array to store floor colors

point3 axis_points[9]; // array to store x, y, z axis
color3 axis_colors[9]; // array to store axis colors

vec3 ab = vec3(3, 0, -8); //vector AB = vector B - vector A
vec3 bc = vec3(4, 0, 9); //vector BC
vec3 ca = vec3(-7, 0, -1); //vector CA

vec3 avec = vec3(-4, 1, 4); //Vector A
vec3 bvec = vec3(-1, 1, -4); //Vector B
vec3 cvec = vec3(3, 1, 5); //Vector C

vec3 ab_rotation_vec = vec3(-8, 0, -3); //rotation vectors for AB translation
vec3 bc_rotation_vec = vec3(9, 0, -4); //rotation vectors for BC translation
vec3 ca_rotation_vec = vec3(-1, 0, 7); //rotation vectors for CA translation

vec3 dist = avec; // translation vector.

int Index = 0; //index for floor_colors and floor_points

//Parameter: a vec3 object. Returns: a unit vector normalized.
vec3 get_distance(vec3 random_vec)
{
    float length;
    for(int i=0; i<3; i++)
    {
        length += pow(random_vec[i], 2);
    }
    length = sqrt(length);
    
    vec3 res = random_vec / length;
    return res;
}

//Initialization of floor array
void floor()
{
    floor_colors[Index] = color3(0.0, 1.0, 0.0); floor_points[Index] = point3(5.0, 0.0, -4.0); Index++;
    floor_colors[Index] = color3(0.0, 1.0, 0.0); floor_points[Index] = point3(5.0, 0.0, 8.0); Index++;
    floor_colors[Index] = color3(0.0, 1.0, 0.0); floor_points[Index] = point3(-5.0, 0.0, -4.0); Index++;

    floor_colors[Index] = color3(0.0, 1.0, 0.0); floor_points[Index] = point3(-5.0, 0.0, 8.0); Index++;
    floor_colors[Index] = color3(0.0, 1.0, 0.0); floor_points[Index] = point3(-5.0, 0.0, -4.0); Index++;
    floor_colors[Index] = color3(0.0, 1.0, 0.0); floor_points[Index] = point3(5.0, 0.0, 8.0); Index++;
}

//Initialization of axis array
void axis()
{
    axis_colors[0] = color3(1.0, 0.0, 0.0);
    axis_colors[1] = color3(1.0, 0.0, 0.0);
    axis_colors[2] = color3(1.0, 0.0, 0.0);
    
    axis_colors[3] = color3(1.0, 0.0, 1.0);
    axis_colors[4] = color3(1.0, 0.0, 1.0);
    axis_colors[5] = color3(1.0, 0.0, 1.0);
    
    axis_colors[6] = color3(0.0, 0.0, 1.0);
    axis_colors[7] = color3(0.0, 0.0, 1.0);
    axis_colors[8] = color3(0.0, 0.0, 1.0);
    
    axis_points[0] = point3(0.0, 0.02, 0.0);
    axis_points[1] = point3(10.0, 0.02, 0.0);
    axis_points[2] = point3(20.0, 0.02, 0.0);
    
    axis_points[3] = point3(0.0, 0.0, 0.0);
    axis_points[4] = point3(0.0, 10.0, 0.0);
    axis_points[5] = point3(0.0, 20.0, 0.0);
    
    axis_points[6] = point3(0.0, 0.02, 0.0);
    axis_points[7] = point3(0.0, 0.02, 10.0);
    axis_points[8] = point3(0.0, 0.02, 20.0);
}
//Reads in file and store data in vectors.
//Creates a point3 array and copies point3 object created by the data into the point3 array.
//point3 array is referenced by a global variable -- sphere_points and sphere_colors
void readfile(){
    
    ifstream infile("sphere.128.txt");
    
    if (!infile) {
        cerr << "Failed to open the file." << endl;
        exit(1);
    }

    int d;
    float f;
    infile >> d;
    
    tri_NumVertices = d * 3;
    
    vector<point3> tri_points;
    vector<color3> tri_colors;
    
    float x, y, z;
    
    while(infile >> d)
    {
        if(d == 3)
        {
            for(int j=0; j<3; j++)
            {
                infile >> x >> y >> z;
                tri_points.push_back(point3(x, y, z));
                tri_colors.push_back(color3(1.0, 0.84, 0.0));
            }
        }
    }
    
    sphere_points = new point3[tri_NumVertices];
    sphere_colors = new color3[tri_NumVertices];

    for(size_t i=0; i<tri_points.size(); i++)
    {
        sphere_points[i] = tri_points[i];
        sphere_colors[i] = tri_colors[i];
    }
}

void drawObj(GLuint buffer, int num_vertices)
{
    //--- Activate the vertex buffer object to be drawn ---//
    glBindBuffer(GL_ARRAY_BUFFER, buffer);

    /*----- Set up vertex attribute arrays for each vertex attribute -----*/
    GLuint vPosition = glGetAttribLocation(program, "vPosition");
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0,
              BUFFER_OFFSET(0) );

    GLuint vColor = glGetAttribLocation(program, "vColor");
    glEnableVertexAttribArray(vColor);
    glVertexAttribPointer(vColor, 3, GL_FLOAT, GL_FALSE, 0,
              BUFFER_OFFSET(sizeof(point3) * num_vertices) );
      // the offset is the (total) size of the previous vertex attribute array(s)

    /* Draw a sequence of geometric objs (triangles) from the vertex buffer
       (using the attributes specified in each enabled vertex attribute array) */
    if (lineFlag == 0){
        glDrawArrays(GL_TRIANGLES, 0, num_vertices);
    } else {
        glDrawArrays(GL_LINES, 0, num_vertices);
    }
    /*--- Disable each vertex attribute array being enabled ---*/
    glDisableVertexAttribArray(vPosition);
    glDisableVertexAttribArray(vColor);
}

void display( void )
{
  GLuint  model_view;  // model-view matrix uniform shader variable location
  GLuint  projection;  // projection matrix uniform shader variable location

    cout << "angle rotated: " << angle << endl;

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glUseProgram(program); // Use the shader program

    model_view = glGetUniformLocation(program, "model_view" );
    projection = glGetUniformLocation(program, "projection" );

/*---  Set up and pass on Projection matrix to the shader ---*/
    mat4  p = Perspective(fovy, aspect, zNear, zFar);
    glUniformMatrix4fv(projection, 1, GL_TRUE, p); // GL_TRUE: matrix is row-major

/*---  Set up and pass on Model-View matrix to the shader ---*/
    // eye is a global variable of vec4 set to init_eye and updated by keyboard()
    vec4    at(0.0, 0.0, 0.0, 1.0);
    vec4    up(0.0, 1.0, 0.0, 0.0);
    
    mat4  mv = LookAt(eye, at, up);
    //m is the accumulated rotational matrix. Initialized with identity matrix.
    mat4 m = mat4(vec4(1, 0, 0, 0), vec4(0, 1, 0, 0), vec4(0, 0, 1, 0), vec4(0, 0, 0, 1));
    
    //mv for point A to point B
    if (animationFlag == 0)
    {
        dist = avec;
        vec3 leng = get_distance(ab) * angle * 2 * M_PI / 360;
        dist[0] += leng[0];
        dist[1] += leng[1];
        dist[2] += leng[2];

        mv = mv * Translate(dist[0], dist[1], dist[2]) * Rotate(angle, ab_rotation_vec[0], ab_rotation_vec[1], ab_rotation_vec[2]) * m;
        
        //if the ball has reached point B
        if (dist[0] > bvec[0])
        {
            m = Rotate(angle, ab_rotation_vec[0], ab_rotation_vec[1], ab_rotation_vec[2]) * m;
            animationFlag = 1;
            angle = 0.0;
        }
    }
    //mv for point B to point C
    else if (animationFlag == 1)
    {
        dist = bvec;
        vec3 leng = get_distance(bc) * angle * 2 * M_PI / 360;
        
        dist[0] += leng[0];
        dist[1] += leng[1];
        dist[2] += leng[2];
        
        mv = mv * Translate(dist[0], dist[1], dist[2]) * Rotate(angle, bc_rotation_vec[0], bc_rotation_vec[1], bc_rotation_vec[2]) * m;
        
        //if the ball has reached point C
        if (dist[0] > cvec[0])
        {
            m = Rotate(angle, bc_rotation_vec[0], bc_rotation_vec[1], bc_rotation_vec[2]) * m;
            animationFlag = 2;
            angle = 0.0;
        }
    }
    //mv for point C to point A
    else if (animationFlag == 2)
    {
        dist = cvec;
        vec3 leng = get_distance(ca) * angle * 2 * M_PI / 360;
        
        dist[0] += leng[0];
        dist[1] += leng[1];
        dist[2] += leng[2];
        
        mv = mv * Translate(dist[0], dist[1], dist[2]) * Rotate(angle, ca_rotation_vec[0], ca_rotation_vec[1], ca_rotation_vec[2]) * m;
        
        //if the ball has reached point A
        if(dist[0] < avec[0])
        {
            m = Rotate(angle, ca_rotation_vec[0], ca_rotation_vec[1], ca_rotation_vec[2]) * m;
            animationFlag = 0;
            angle = 0.0;
        }
    }

    //drawing the sphere
    glUniformMatrix4fv(model_view, 1, GL_TRUE, mv); // GL_TRUE: matrix is row-major
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    drawObj(tri_buffer, tri_NumVertices);  // draw the cube
    
    mv = LookAt(eye, at, up);
    //drawing the floor
    glUniformMatrix4fv(model_view, 1, GL_TRUE, mv); // GL_TRUE: matrix is row-major
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    drawObj(floor_buffer, floor_NumVertices);  // draw the floor
    //drawing the axis
    glUniformMatrix4fv(model_view, 1, GL_TRUE, mv); // GL_TRUE: matrix is row-major
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    drawObj(axis_buffer, 9);
    
    glutSwapBuffers();
}

//----------------------------------------------------------------------------
void reshape(int width, int height)
{
    glViewport(0, 0, width, height);
    aspect = (GLfloat) width  / (GLfloat) height;
    glutPostRedisplay();
}
//----------------------------------------------------------------------------

//---------------------------------------------------------------------------
void idle (void)
{
    angle += 1.0f;//YJC: change this value to adjust the cube rotation speed.
    glutPostRedisplay();
}
//----------------------------------------------------------------------------

void init()
{
    //Create and initialize a vertex buffer object for sphere, to be used in display()
    glGenBuffers(1, &tri_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, tri_buffer);
    glBufferData(GL_ARRAY_BUFFER, 12 * tri_NumVertices * 2,
         NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, 12 * tri_NumVertices, sphere_points);
    glBufferSubData(GL_ARRAY_BUFFER, 12 * tri_NumVertices, 12 * tri_NumVertices, sphere_colors);

    floor();
    // Create and initialize a vertex buffer object for floor, to be used in display()
    glGenBuffers(1, &floor_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, floor_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(floor_points) + sizeof(floor_colors),
         NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(floor_points), floor_points);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(floor_points), sizeof(floor_colors),
                    floor_colors);
    
    axis();
    //Create and initialize x, y, and z axis
    glGenBuffers(1, &axis_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, axis_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(axis_points) + sizeof(axis_colors),
         NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(axis_points), axis_points);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(axis_points), sizeof(axis_colors),
                    axis_colors);
    
    program = InitShader("vshader42.glsl", "fshader42.glsl");
    
    glEnable( GL_DEPTH_TEST );
    glClearColor( 0.529, 0.807, 0.92, 0.0 );
    glLineWidth(2.0);
}

//when the user clicks right mouse button, animation stops.
//right click is enabled only when startFlag is 1
void myMouse(int button, int state, int x, int y)
{
    if(startFlag == 1)
    {
        //animation has started
        if(button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
        {
            mouseFlag = 1 - mouseFlag;
            if (mouseFlag == 1) glutIdleFunc(NULL);
            else                glutIdleFunc(idle);
        }
    }
}

//Process keyboard inputs.
//Pressing B starts animation by setting startFlag to 1
void keyboard(unsigned char key, int x, int y)
{
    switch(key) {
    case 033: // Escape Key
    case 'q': case 'Q':
        exit( EXIT_SUCCESS );
        break;

        case 'X': eye[0] += 1.0; break;
    case 'x': eye[0] -= 1.0; break;
        case 'Y': eye[1] += 1.0; break;
    case 'y': eye[1] -= 1.0; break;
        case 'Z': eye[2] += 1.0; break;
    case 'z': eye[2] -= 1.0; break;
            
    case 'b': case 'B': // Start animation
        startFlag = 1;
        glutIdleFunc(idle); //Start modifying model-view
        break;
    }
    glutPostRedisplay();
}

//Pop up menu for mouse left click
void menu(int id)
{
    switch(id)
    {
        case 1:
            eye = init_eye;
            break;
        case 2:
            exit(0);
            break;
    }
    glutPostRedisplay();
}

/*-----------------
The main function
------------------*/
int main(int argc, char **argv)
{
    //importing file and creating sphere
    readfile();
    
    glutInit(&argc, argv);
#ifdef __APPLE__ // Enable core profile of OpenGL 3.2 on macOS.
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_3_2_CORE_PROFILE);
#else
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
#endif
    glutInitWindowSize(512, 512);
    glutCreateWindow("hw2");

#ifdef __APPLE__ // on macOS
    // Core profile requires to create a Vertex Array Object (VAO).
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
#else           // on Linux or Windows, we still need glew
    /* Call glewInit() and error checking */
    int err = glewInit();
    if (GLEW_OK != err)
    {
        printf("Error: glewInit failed: %s\n", (char*) glewGetErrorString(err));
        exit(1);
    }
#endif
    
    glutCreateMenu(menu);
    glutAddMenuEntry("Default View Point", 1);
    glutAddMenuEntry("Quit", 2);
    glutAttachMenu(GLUT_LEFT_BUTTON);
    
    glutMouseFunc(myMouse);
    
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(NULL);
    glutKeyboardFunc(keyboard);
    
    init();
    glutMainLoop();
    return 0;
}
