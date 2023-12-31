// by Dongje Kim, dk3156
#include "Angel-yjc.h"

typedef Angel::vec4  color4;
typedef Angel::vec4  point4;
typedef Angel::vec3  color3;
typedef Angel::vec3  point3;

#include <fstream>
#include <string>
#include <math.h>
#include <iostream>
using namespace std;

GLuint program;
GLuint program_2;
GLuint cube_buffer;
GLuint floor_light_buffer;
GLuint floor_buffer;
GLuint line_buffer;
GLuint sphere_buffer;
GLuint sphere_shadow_buffer;
GLuint s_flat_buffer;
GLuint s_smooth_buffer;
GLuint f_tex_buffer;
GLuint firework_buffer;
static GLuint texName;
static GLuint line_texName;
#define ImageWidth  64
#define ImageHeight 64
GLubyte Image[ImageHeight][ImageWidth][4];
#define    stripeImageWidth 32
GLubyte stripeImage[4 * stripeImageWidth];

// Projection transformation parameters
GLfloat  fovy = 45.0;  // Field-of-view in Y direction angle (in degrees)
GLfloat  aspect;       // Viewport aspect ratio
GLfloat  zNear = 0.5, zFar = 13.0;
GLfloat radius;
GLfloat angle = 0.0; // rotation angle in degrees
vec4 init_eye(7.0, 3.0, -10.0, 1.0); // initial viewer position
vec4 eye = init_eye;               // current viewer position
int animationFlag = 1; // 1: animation; 0: non-animation. Toggled by key 'a' or 'A'
int floorFlag = 1;  // 1: solid floor; 0: wireframe floor. Toggled by key 'f' or 'F'
int fog_flag = 1;
int solid_Flag = 1;
float shade_flag = 1.0;
int text_ground_flag = 1;
int blend_shadow_flag = 1;
int shadow = 1;
int flatshade = 0;
int smoothshade = 1;
int lighting = 1;
int pointsource = 1;
int spotlight = 0;
int firework_flag = 1;
int vertical_flag = 1;
int slant_flag = 0;
int eye_space_flag = 0;
int object_space_flag = 1;
int text_sphere_flag = 1;
int    text_sphere_line_flag = 1;
int text_sphere_check_flag = 0;
float t_sub = 0.0;
int enable_lat = 1;
int upright_lat_flag = 0;
point3 track[] = { point3(-4.0, 1.0, 4.0), point3(-1.0, 1.0, -4.0), point3(3.0, 1.0, 5.0) };
int currentSegment = 0, totalSegments = 3;
GLfloat theta = 0.0, delta = 0.06;//moving and rotation speed
point3 centerPos = track[currentSegment];
point3*vectors;
point3*rotationAxis;

#define PI 3.14159265358979323846
#define sqrt3f(x,y,z) sqrt(x*x+y*y+z*z)
float distanceAt(point3 p1, point3 p2);
mat4 acc_matrix = Angel::identity();
bool begin1 = false, rolling = false;

point4 floor_light_points[6];
vec3   floor_normals[6];

const int floor_NumVertices = 6; //(1 face)*(2 triangles/face)*(3 vertices/triangle)
point4 floor_points[floor_NumVertices]; // positions for all vertices
color3 floor_colors[floor_NumVertices]; // normals for all vertices

point4 f_vertices[4] = {
    point4( 5, 0,  8, 1),
    point4( 5, 0,  -4, 1),
    point4(  -5,  0,  -4, 1),
    point4(  -5,  0,  8, 1),
};

point4 f_l_vertices[4] = {
    point4( 5, 0,  8, 1.0),
    point4( 5, 0,  -4, 1.0),
    point4(  -5,  0,  -4, 1.0),
    point4(  -5,  0,  8, 1.0),
};

point4 f_tex_vertices[6] = {
    point4(  -5,  0,  -4, 1),
    point4(  -5,  0,  8, 1),
    point4( 5, 0,  8, 1),

    point4( 5, 0,  8, 1),
    point4( 5, 0,  -4, 1),
    point4(  -5,  0,  -4, 1),
};

point3 f_tex_normals[6] = {
    point3(  0,  1,  0),
    point3(  0,  1,  0),
    point3(  0,  1,  0),
    point3(  0,  1,  0),
    point3(  0,  1,  0),
    point3(  0,  1,  0),
};

vec2 f_tex_coord[6] = {
    vec2(0.0, 0.0),
    vec2(0.0, 6 / 4.0),
    vec2(5 / 4.0, 6 / 4.0),

    vec2(5 / 4.0, 6 / 4.0),
    vec2(5 / 4.0, 0.0),
    vec2(0.0, 0.0),
};

// RGBA colors
color3 vertex_colors[8] = {
    color3( 0.0, 0.0, 0.0),  // black
    color3( 1.0, 0.0, 0.0),  // red
    color3( 1.0, 0.84, 0.0),  //golden_yellow
    color3( 0.0, 1.0, 0.0),  // green
    color3( 0.0, 0.0, 1.0),  // blue
    color3( 1.0, 0.0, 1.0),  // magenta
    color3( 1.0, 1.0, 1.0),  // white
    color3( 0.0, 1.0, 1.0)   // cyan
};

// Model-view and projection matrices uniform location
GLuint  ModelView, Projection;
GLuint  ModelView_2, Projection_2;
/*----- Shader Lighting Parameters -----*/
void set_point_light(mat4 mv)
{
    color4 light_ambient( 1.0, 1.0, 1.0, 1.0 );
    color4 light_diffuse( 0.8, 0.8, 0.8, 1.0 );
    color4 light_specular( 0.2, 0.2, 0.2, 1.0 );
    float const_att = 2.0;
    float linear_att = 0.01;
    float quad_att = 0.001;
    point4 light_position(-14.0, 12.0, -3.0, 1.0 );
    // In World frame.
    // Needs to transform it to Eye Frame
    // before sending it to the shader(s).
    vec4 light_position_eyeFrame = mv * light_position;
    glUniform4fv( glGetUniformLocation(program, "LightPosition"),
                  1, light_position_eyeFrame);

    glUniform1f(glGetUniformLocation(program, "ConstAtt"),
                const_att);
    glUniform1f(glGetUniformLocation(program, "LinearAtt"),
                linear_att);
    glUniform1f(glGetUniformLocation(program, "QuadAtt"),
                quad_att);
    glUniform4fv(glGetUniformLocation(program, "input_light_ambient"), 1,
                 light_ambient);
    glUniform4fv(glGetUniformLocation(program, "input_light_diffuse"), 1,
                 light_diffuse);
    glUniform4fv(glGetUniformLocation(program, "input_light_specular"), 1,
                 light_specular);
    glUniform1f(glGetUniformLocation(program, "point_flag"), pointsource * 1.0);
    glUniform1f(glGetUniformLocation(program, "spot_flag"), 0.0);
}

void set_spot_light(mat4 mv)
{
    color4 light_ambient( 0, 0, 0, 1.0 );
    color4 light_diffuse( 1.0, 1.0, 1.0, 1.0 );
    color4 light_specular( 1.0, 1.0, 1.0, 1.0 );
    float const_att = 2.0;
    float linear_att = 0.01;
    float quad_att = 0.001;
    point4 light_position(-14, 12.0, -3, 1.0 );

    float spot_exp = 15.0;
    float spot_ang = cos(20.0 * PI / 180);
    // In World frame.
    // Needs to transform it to Eye Frame
    // before sending it to the shader(s).
    vec4 light_position_eyeFrame = mv * light_position;
    glUniform4fv( glGetUniformLocation(program, "LightPosition"),
                  1, light_position_eyeFrame);

    glUniform1f(glGetUniformLocation(program, "ConstAtt"),
                const_att);
    glUniform1f(glGetUniformLocation(program, "LinearAtt"),
                linear_att);
    glUniform1f(glGetUniformLocation(program, "QuadAtt"),
                quad_att);
    glUniform4fv(glGetUniformLocation(program, "input_light_ambient"), 1,
                 light_ambient);
    glUniform4fv(glGetUniformLocation(program, "input_light_diffuse"), 1,
                 light_diffuse);
    glUniform4fv(glGetUniformLocation(program, "input_light_specular"), 1,
                 light_specular);
    glUniform1f(glGetUniformLocation(program, "point_flag"), 0.0);
    glUniform1f(glGetUniformLocation(program, "spot_flag"), 1.0);
    point4 spot_direction = mv * vec4(-6.0, 0.0, -4.5, 1.0);
    glUniform4fv(glGetUniformLocation(program, "spot_direction"), 1,
                 spot_direction);
    glUniform1f(glGetUniformLocation(program, "spot_exp"), spot_exp);
    glUniform1f(glGetUniformLocation(program, "spot_ang"), spot_ang);

}

void set_floor_m()
{
    color4 material_ambient( 0.2, 0.2, 0.2, 1.0 );
    color4 material_diffuse( 0, 1.0, 0, 1.0 );
    color4 material_specular( 0, 0, 0, 1.0 );
    float  material_shininess = 125.0;
    glUniform4fv(glGetUniformLocation(program, "ini_material_ambient"), 1,
                 material_ambient);
    glUniform4fv(glGetUniformLocation(program, "ini_material_diffuse"), 1,
                 material_diffuse);
    glUniform4fv(glGetUniformLocation(program, "ini_material_specular"), 1,
                 material_specular);
    glUniform1f(glGetUniformLocation(program, "Shininess"), 1.0);
}

void set_sphere_m()
{
    color4 material_ambient( 0.2, 0.2, 0.2, 1.0 );
    color4 material_diffuse( 1.0, 0.84, 0, 1.0 );
    color4 material_specular( 1.0, 0.84, 0, 1 );
    float  material_shininess = 125;
    glUniform4fv(glGetUniformLocation(program, "ini_material_ambient"), 1,
                 material_ambient);
    glUniform4fv(glGetUniformLocation(program, "ini_material_diffuse"), 1,
                 material_diffuse);
    glUniform4fv(glGetUniformLocation(program, "ini_material_specular"), 1,
                 material_specular);
    glUniform1f(glGetUniformLocation(program, "Shininess"), material_shininess);

}

void set_fog()
{
    glUniform1f(glGetUniformLocation(program, "fog_flag"), fog_flag * 1.0);
}

int Index = 0;

void floor()
{
    floor_colors[Index] = vertex_colors[3]; floor_points[0] = f_vertices[0]; Index++;
    floor_colors[Index] = vertex_colors[3]; floor_points[1] = f_vertices[1]; Index++;
    floor_colors[Index] = vertex_colors[3]; floor_points[2] = f_vertices[2]; Index++;
    floor_colors[Index] = vertex_colors[3]; floor_points[3] = f_vertices[0]; Index++;
    floor_colors[Index] = vertex_colors[3]; floor_points[4] = f_vertices[3]; Index++;
    floor_colors[Index] = vertex_colors[3]; floor_points[5] = f_vertices[2]; Index++;
}

point4 line_points[9];
color3 line_colors[9];

void xyz_line()
{
    line_colors[0] = vertex_colors[1]; line_points[0] = point4(0, 0.02, 0, 1);
    line_colors[1] = vertex_colors[1]; line_points[1] = point4(10.0, 0.02, 0, 1);
    line_colors[2] = vertex_colors[1]; line_points[2] = point4(20.0, 0.02, 0, 1);

    line_colors[3] = vertex_colors[5]; line_points[3] = point4(0.0, 0.0, 0.0, 1);
    line_colors[4] = vertex_colors[5]; line_points[4] = point4(0, 10.0, 0.0, 1);
    line_colors[5] = vertex_colors[5]; line_points[5] = point4(0, 20.0, 0.0, 1);

    line_colors[6] = vertex_colors[4]; line_points[6] = point4(0, 0.02, 0, 1);
    line_colors[7] = vertex_colors[4]; line_points[7] = point4(0, 0.02, 10, 1);
    line_colors[8] = vertex_colors[4]; line_points[8] = point4(0, 0.02, 20, 1);
}



void my_quad( int a, int b, int c, int d )
{
    // Initialize temporary vectors along the quad's edges to
    //   compute its face normal
    vec4 u = f_l_vertices[b] - f_l_vertices[a];
    vec4 v = f_l_vertices[d] - f_l_vertices[a];
    
    Index = 0;

    vec3 tmp_normal = normalize( cross(v, u) );
    floor_normals[Index] = tmp_normal; floor_light_points[Index] = f_l_vertices[a]; Index++;
    floor_normals[Index] = tmp_normal; floor_light_points[Index] = f_l_vertices[b]; Index++;
    floor_normals[Index] = tmp_normal; floor_light_points[Index] = f_l_vertices[c]; Index++;
    floor_normals[Index] = tmp_normal; floor_light_points[Index] = f_l_vertices[a]; Index++;
    floor_normals[Index] = tmp_normal; floor_light_points[Index] = f_l_vertices[c]; Index++;
    floor_normals[Index] = tmp_normal; floor_light_points[Index] = f_l_vertices[d]; Index++;
}

void floor_light()
{
    Index = 0;
    my_quad(1, 0, 3, 2);

}

point3 calculateDirection(point3 from, point3 to) {
    point3 v;
    v.x = to.x - from.x;
    v.y = to.y - from.y;
    v.z = to.z - from.z;

    //convert v to unit-length
    float d = sqrt3f(v.x, v.y, v.z);
    v.x = v.x / d;
    v.y = v.y / d;
    v.z = v.z / d;

    return v;
}

point3 crossProduct(point3 u, point3 v) {
    point3 n;
    n.x = u.y * v.z - u.z * v.y;
    n.y = u.z * v.x - u.x * v.z;
    n.z = u.x * v.y - u.y * v.x;
    return n;
}

void image_set_up(void)
{
    int i, j, c;
    /* --- Generate checkerboard image to the image array ---*/
    for (i = 0; i < ImageHeight; i++)
        for (j = 0; j < ImageWidth; j++)
        {
            c = (((i & 0x8) == 0) ^ ((j & 0x8) == 0));

            if (c == 1) /* white */
            {
                c = 255;
                Image[i][j][0] = (GLubyte) c;
                Image[i][j][1] = (GLubyte) c;
                Image[i][j][2] = (GLubyte) c;
            }
            else  /* green */
            {
                Image[i][j][0] = (GLubyte) 0;
                Image[i][j][1] = (GLubyte) 150;
                Image[i][j][2] = (GLubyte) 0;
            }

            Image[i][j][3] = (GLubyte) 255;
        }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    /*--- Generate 1D stripe image to array stripeImage[] ---*/
    for (j = 0; j < stripeImageWidth; j++) {
        
        stripeImage[4 * j] = (GLubyte)    255;
        stripeImage[4 * j + 1] = (GLubyte) ((j > 4) ? 255 : 0);
        stripeImage[4 * j + 2] = (GLubyte) 0;
        stripeImage[4 * j + 3] = (GLubyte) 255;
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

}

//File in
point4 *sphereData;
color3 *sphere_color;

int sphere_NumVertices;
int col;

point4 *sphere_shadow_data;
color4 *sphere_shadow_color;

point4 *sphere_shade_d;
vec3 *sphere_flat_shade;
vec3 *sphere_smooth_shade;

void ini_flat_normal()
{
    int flat_index = 0;
    for (int j = 0; j < col; j++)
    {

        vec4 curr_p1 = sphere_shade_d[3 * j];
        vec4 curr_p2 = sphere_shade_d[3 * j + 1];
        vec4 curr_p3 = sphere_shade_d[3 * j + 2];


        vec4 u = curr_p2 - curr_p1;
        vec4 v = curr_p3 - curr_p1;

        vec3 tmp_normal = normalize( cross(u, v) );
        sphere_flat_shade[flat_index] = tmp_normal;  flat_index++;
        sphere_flat_shade[flat_index] = tmp_normal;  flat_index++;
        sphere_flat_shade[flat_index] = tmp_normal;  flat_index++;

    }
};

void ini_smooth_normal()
{
    for (int i = 0; i < col * 3; i++)
    {
        vec4 curr_p = sphere_shade_d[i];
        vec3 tmp = vec3(curr_p[0], curr_p[1], curr_p[2]);
        tmp = normalize(tmp);
        sphere_smooth_shade[i] = tmp;
    }
};

void readFiles()
{
    ifstream fp;
    char* filename = new char[100];

    int points;
    float temp[3];

    int count = 0;
    cout << "Enter filename -- sphere.1024 or sphere.256. Press ENTER to use sphere.1024" << endl;

    cin.getline(filename, 100);
    if (strlen(filename) == 0) {
        filename = (char*)"sphere.1024";
    }
    fp.open(filename);
    if (!fp) {
        cerr << "Failed to open " << filename << " !" << endl;
        exit(0);
    }
    fp >> col;
    cout << col << endl;
    sphereData = new point4[col * 3];
    sphere_shadow_data = new point4[col * 3];

    sphere_color = new point3[col * 3];
    sphere_shadow_color = new point4[col * 3];

    sphere_shade_d = new point4[col * 3];
    sphere_flat_shade = new point3[col * 3];
    sphere_smooth_shade = new point3[col * 3];

    for (int i = 0; i < col; i++) {
        fp >> points;
        for (int j = 0; j < points; j++) {
            for (int k = 0; k < 3; k++) {
                fp >> temp[k];

            }
            point4 tmp = vec4( temp[0], temp[1], temp[2], 1);
            point4 tmp2 = vec4( temp[0], temp[1], temp[2], 1);
            point4 tmp_shade = vec4(temp[0], temp[1], temp[2], 1);

            sphereData[count] = tmp;
            sphere_shadow_data[count] = tmp2;
            sphere_shade_d[count] = tmp_shade;
            count++;
            //    cout << sphereData[count - 1] << endl;
        }
    }
    sphere_NumVertices = col * 3;
    fp.close();
    ini_flat_normal();
    ini_smooth_normal();
}

void colorsphere()
{
    for (int i = 0; i < col * 3; i++) {
        sphere_color[i] = color3(1.0, 0.84, 0.0);
        sphere_shadow_color[i] = color4(0.25, 0.25, 0.25, 0.65);
    }

}

void init_tex_ground()
{
    image_set_up();
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    /*--- Create and Initialize a texture object ---*/
    glGenTextures(1, &texName);      // Generate texture obj name(s)

    glActiveTexture( GL_TEXTURE0 );  // Set the active texture unit to be 0
    glBindTexture(GL_TEXTURE_2D, texName); // Bind the texture to this texture unit

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ImageWidth, ImageHeight,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, Image);

    glGenBuffers(1, &f_tex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, f_tex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(f_tex_vertices) + sizeof(f_tex_normals) + sizeof(f_tex_coord),
                 NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(f_tex_vertices), f_tex_vertices);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(f_tex_vertices), sizeof(f_tex_normals), f_tex_normals);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(f_tex_vertices)    + sizeof(f_tex_normals),
                    sizeof(f_tex_coord), f_tex_coord);
}

point3 firework_v[300];
point3 firework_c[300];

void init_firework()
{
    for (int i = 0; i < 300; i++)
    {
        vec3 tmp_v = vec3( 2.0 * ((rand() % 256) / 256.0 - 0.5), 2.4 * (rand() % 256) / 256.0, 2.0 * ((rand() % 256) / 256.0 - 0.5));
        firework_v[i] = tmp_v;
        vec3 tmp_c = vec3( (rand() % 256) / 256.0, (rand() % 256) / 256.0, (rand() % 256) / 256.0 );
        firework_c[i] = tmp_c;
    }
}

void init_line_tex()
{
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    /*--- Create and Initialize a texture object ---*/
    glGenTextures(1, &line_texName);      // Generate texture obj name(s)

    glActiveTexture( GL_TEXTURE1 );  // Set the active texture unit to be 0
    glBindTexture(GL_TEXTURE_1D, line_texName); // Bind the texture to this texture unit

    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, 32,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, stripeImage);
}

//----------------------------------------------------------------------------
// OpenGL initialization
void init()
{
    init_tex_ground();
    init_line_tex();

    radius = 1;
    colorsphere();
    //calculate the rolling directions
    totalSegments = sizeof(track) / sizeof(point3);
    vectors = new point3[totalSegments];
    rotationAxis = new point3[totalSegments];
    for (int i = 0; i < totalSegments - 1; i++) {
        vectors[i] = calculateDirection(track[i], track[i + 1]);
    }
    //and the last point to the first one
    vectors[totalSegments - 1] = calculateDirection(track[totalSegments - 1], track[0]);

    //calculate the rotating axis vectors
    point3 y_axis(0, 1, 0 );
    for (int i = 0; i < totalSegments; i++) {
        rotationAxis[i] = crossProduct(y_axis, vectors[i]);
    }

    // Create and initialize a vertex buffer object for floor, to be used in display()

    floor();
    glGenBuffers(1, &floor_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, floor_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(floor_points) + sizeof(floor_colors),
                 NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(floor_points), floor_points);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(floor_points), sizeof(floor_colors),
                    floor_colors);

    floor_light();
    glGenBuffers( 1, &floor_light_buffer );
    glBindBuffer( GL_ARRAY_BUFFER, floor_light_buffer );
    glBufferData( GL_ARRAY_BUFFER, sizeof(floor_light_points) + sizeof(floor_normals),
                  NULL, GL_STATIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(floor_light_points), floor_light_points );
    glBufferSubData( GL_ARRAY_BUFFER, sizeof(floor_light_points),
                     sizeof(floor_normals), floor_normals );

    glGenBuffers( 1, &s_flat_buffer );
    glBindBuffer( GL_ARRAY_BUFFER, s_flat_buffer );
    glBufferData( GL_ARRAY_BUFFER, 3 * col * sizeof(point4) + 3 * col * sizeof(color3),
                  NULL, GL_STATIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, 0, 3 * col * sizeof(point4), sphere_shade_d );
    glBufferSubData( GL_ARRAY_BUFFER, 3 * col * sizeof(point4),
                     3 * col * sizeof(color3), sphere_flat_shade );

    glGenBuffers( 1, &s_smooth_buffer );
    glBindBuffer( GL_ARRAY_BUFFER, s_smooth_buffer );
    glBufferData( GL_ARRAY_BUFFER, 3 * col * sizeof(point4) + 3 * col * sizeof(color3),
                  NULL, GL_STATIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, 0, 3 * col * sizeof(point4), sphere_shade_d );
    glBufferSubData( GL_ARRAY_BUFFER, 3 * col * sizeof(point4),
                     3 * col * sizeof(color3), sphere_smooth_shade );

    xyz_line();
    glGenBuffers(1, &line_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, line_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(line_points) + sizeof(line_colors),
                 NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(line_points), line_points);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(line_points), sizeof(line_colors),
                    line_colors);

    init_firework();
    glGenBuffers(1, &firework_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, firework_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(firework_v) + sizeof(firework_c),
                 NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(firework_v), firework_v);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(firework_v), sizeof(firework_c),
                    firework_c);

    // create and initialize a vertex buffer object for sphere.
    glGenBuffers(1, &sphere_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, sphere_buffer);
    glBufferData(GL_ARRAY_BUFFER, 3 * col * sizeof(point4) + 3 * col * sizeof(color3),
                 NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, 3 * col * sizeof(point4), sphereData);
    glBufferSubData(GL_ARRAY_BUFFER, 3 * col * sizeof(point4), 3 * col * sizeof(color3),
                    sphere_color);

    glGenBuffers(1, &sphere_shadow_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, sphere_shadow_buffer);
    glBufferData(GL_ARRAY_BUFFER, 3 * col * sizeof(point4) + 3 * col * sizeof(color3),
                 NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, 3 * col * sizeof(point4), sphere_shadow_data);
    glBufferSubData(GL_ARRAY_BUFFER, 3 * col * sizeof(point4), 3 * col * sizeof(color3),
                    sphere_shadow_color);
    // Load shaders and create a shader program (to be used in display())

    glEnable( GL_DEPTH_TEST );
    glClearColor( 0.0, 0.0, 0.0, 1.0 );
    glLineWidth(2.0);

    program = InitShader( "vshader53.glsl", "fshader53.glsl" );
    program_2 = InitShader( "dj_vshader.glsl", "dj_fshader.glsl" );
}

void drawObj(GLuint buffer, int num_vertices)
{
    //--- Activate the vertex buffer object to be drawn ---//
    glBindBuffer(GL_ARRAY_BUFFER, buffer);

    /*----- Set up vertex attribute arrays for each vertex attribute -----*/
    GLuint vPosition = glGetAttribLocation( program, "vPosition" );
    glEnableVertexAttribArray( vPosition );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,
                           BUFFER_OFFSET(0) );

    GLuint vNormal = glGetAttribLocation( program, "vNormal" );
    glEnableVertexAttribArray( vNormal );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0,
                           BUFFER_OFFSET( num_vertices * sizeof(point4)) );

    glDrawArrays(GL_TRIANGLES, 0, num_vertices);

    /*--- Disable each vertex attribute array being enabled ---*/
    glDisableVertexAttribArray(vPosition);
    glDisableVertexAttribArray(vNormal);
}

void drawObj_2(GLuint buffer, int num_vertices)
{
    glBindBuffer(GL_ARRAY_BUFFER, buffer);

    /*----- Set up vertex attribute arrays for each vertex attribute -----*/
    GLuint vPosition = glGetAttribLocation(program, "vPosition");
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0,
                          BUFFER_OFFSET(0));

    GLuint vColor = glGetAttribLocation(program, "vColor");
    glEnableVertexAttribArray(vColor);
    glVertexAttribPointer(vColor, 3, GL_FLOAT, GL_FALSE, 0,
                          BUFFER_OFFSET(sizeof(point4) * num_vertices));
    // the offset is the (total) size of the previous vertex attribute array(s)

    /* Draw a sequence of geometric objs (triangles) from the vertex buffer
    (using the attributes specified in each enabled vertex attribute array) */
    glDrawArrays(GL_TRIANGLES, 0, num_vertices);

    /*--- Disable each vertex attribute array being enabled ---*/
    glDisableVertexAttribArray(vPosition);
    glDisableVertexAttribArray(vColor);
}

void drawObj_3(GLuint buffer, int num_vertices)
{
    //--- Activate the vertex buffer object to be drawn ---//
    glBindBuffer(GL_ARRAY_BUFFER, buffer);

    /*----- Set up vertex attribute arrays for each vertex attribute -----*/
    GLuint vPosition = glGetAttribLocation( program, "vPosition" );
    glEnableVertexAttribArray( vPosition );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,
                           BUFFER_OFFSET(0) );

    GLuint vNormal = glGetAttribLocation( program, "vNormal" );
    glEnableVertexAttribArray( vNormal );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0,
                           BUFFER_OFFSET( num_vertices * sizeof(point4)) );

    GLuint vTexCoord = glGetAttribLocation( program, "vTexCoord" );
    glEnableVertexAttribArray( vTexCoord );
    glVertexAttribPointer( vTexCoord, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(num_vertices * (sizeof(point4) + sizeof(point3)) ));

    // the offset is the (total) size of the previous vertex attribute array(s)

    /* Draw a sequence of geometric objs (triangles) from the vertex buffer
       (using the attributes specified in each enabled vertex attribute array) */
    glDrawArrays(GL_TRIANGLES, 0, num_vertices);
    glDisableVertexAttribArray(vTexCoord);
    /*--- Disable each vertex attribute array being enabled ---*/
    glDisableVertexAttribArray(vPosition);
    glDisableVertexAttribArray(vNormal);
}

void drawObj_4(GLuint buffer, int num_vertices)
{
    //--- Activate the vertex buffer object to be drawn ---//
    glBindBuffer(GL_ARRAY_BUFFER, buffer);

    /*----- Set up vertex attribute arrays for each vertex attribute -----*/
    GLuint velocity = glGetAttribLocation(program_2, "velocity");
    glEnableVertexAttribArray(velocity);
    glVertexAttribPointer(velocity, 3, GL_FLOAT, GL_FALSE, 0,
                          BUFFER_OFFSET(0));

    GLuint vColor = glGetAttribLocation(program_2, "vColor");
    glEnableVertexAttribArray(vColor);
    glVertexAttribPointer(vColor, 3, GL_FLOAT, GL_FALSE, 0,
                          BUFFER_OFFSET(sizeof(point3) * num_vertices));
    glDrawArrays(GL_TRIANGLES, 0, num_vertices);
    /*--- Disable each vertex attribute array being enabled ---*/
    glDisableVertexAttribArray(velocity);
    glDisableVertexAttribArray(vColor);
}
//----------------------------------------------------------------------------

mat4 trans_p_to_q(12, 0, 0, 0, 14, 0, 3, -1, 0, 0, 12, 0, 0, 0, 0, 12);

void ini_set_menu_flag()
{
    shade_flag = 0;
    if (solid_Flag == 0 || lighting == 0)
    {
        flatshade = 0;
        smoothshade = 0;
    }
    if (lighting == 1)
    {
        shade_flag = 1.0;
    }
    glUniform1f(glGetUniformLocation(program, "shade_flag"), shade_flag * 1.0);

    glUniform1i(glGetUniformLocation(program, "text_ground_flag"), 0);
    glUniform1i(glGetUniformLocation(program, "f_sphere_flag"), 0);
    glUniform1i(glGetUniformLocation(program, "vertical_flag"), 1);
    glUniform1i(glGetUniformLocation(program, "slant_flag"), 0);
    glUniform1i(glGetUniformLocation(program, "eye_space_flag"), 0);
    glUniform1i(glGetUniformLocation(program, "object_space_flag"), 1);

}

void draw_floor(mat4 mv, mat4 eye_frame)
{
    if (lighting == 0)
    {
        glUniformMatrix4fv(ModelView, 1, GL_TRUE, mv );
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        drawObj_2(floor_buffer, floor_NumVertices);  // draw the floor
    }
    else
    {
        if (pointsource)
        {
            set_point_light(eye_frame);
        }
        else
        {
            set_spot_light(eye_frame);
        }
        set_floor_m();
        mat4  model_view = mv ;
        glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view );
        mat3 normal_matrix = NormalMatrix(model_view, 1);
        glUniformMatrix3fv(glGetUniformLocation(program, "Normal_Matrix"),
                           1, GL_TRUE, normal_matrix );
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        if (text_ground_flag == 1)
        {
            glUniform1i( glGetUniformLocation(program, "texture_1D"), 1 );
            glUniform1i( glGetUniformLocation(program, "texture_2D"), 0 );
            glUniform1i(glGetUniformLocation(program, "text_ground_flag"), 1);
            drawObj_3(f_tex_buffer, 6);
        }
        else
        {drawObj(floor_light_buffer, 6);}
        glUniform1i(glGetUniformLocation(program, "text_ground_flag"), 0);
    }
};

void draw_shadow(mat4 mv)
{
    glUniform1i( glGetUniformLocation(program, "draw_shadow_lat"), 1 );
    glUniform1i( glGetUniformLocation(program, "f_draw_shadow_lat"), 1 );
    glUniform1i(glGetUniformLocation(program, "upright_lat_flag"), upright_lat_flag);
    if (shadow == 1)
    {
        glUniformMatrix4fv(ModelView, 1, GL_TRUE, mv);
        if (solid_Flag == 0)
        {glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);}
        else
        {glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);}
        if (blend_shadow_flag == 1)
        {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glUniform1f(glGetUniformLocation(program, "shade_flag"), 0);
            glUniform1f(glGetUniformLocation(program, "shadow_semi_flag"), 1.0);
            drawObj_2(sphere_shadow_buffer, sphere_NumVertices);
            glDisable(GL_BLEND);
        }
        else
        {
            glUniform1f(glGetUniformLocation(program, "shade_flag"), 0);
            glUniform1f(glGetUniformLocation(program, "shadow_semi_flag"), 0);
            drawObj_2(sphere_shadow_buffer, sphere_NumVertices);
        }
    }
    glUniform1i( glGetUniformLocation(program, "draw_shadow_lat"), 0 );
    glUniform1i( glGetUniformLocation(program, "f_draw_shadow_lat"), 0 );
};

void set_flag_sphere_tex()
{
    glUniform1i(glGetUniformLocation(program, "vertical_flag"), vertical_flag);
    glUniform1i(glGetUniformLocation(program, "slant_flag"), slant_flag);
    glUniform1i(glGetUniformLocation(program, "eye_space_flag"), eye_space_flag);
    glUniform1i(glGetUniformLocation(program, "object_space_flag"), object_space_flag);
    glUniform1i(glGetUniformLocation(program, "sphere_line_flag"), text_sphere_line_flag);
    glUniform1i(glGetUniformLocation(program, "f_sphere_flag"), text_sphere_line_flag);
    glUniform1i(glGetUniformLocation(program, "sphere_check_flag"), text_sphere_check_flag);
    glUniform1i(glGetUniformLocation(program, "f_sphere_check_flag"), text_sphere_check_flag);
    glUniform1i(glGetUniformLocation(program, "upright_lat_flag"), upright_lat_flag);
    glUniform1i(glGetUniformLocation(program, "enable_lat"), enable_lat);


};

void cancel_flag_sphere()
{
    glUniform1i(glGetUniformLocation(program, "f_sphere_flag"), 0);
    glUniform1i(glGetUniformLocation(program, "f_sphere_check_flag"), 0);
    glUniform1i(glGetUniformLocation(program, "eye_space_flag"), 0);
    glUniform1i(glGetUniformLocation(program, "object_space_flag"), 0);
    glUniform1i(glGetUniformLocation(program, "sphere_line_flag"), 0);
    glUniform1i(glGetUniformLocation(program, "sphere_check_flag"), 0);
}

void draw_sphere(mat4 mv, mat4 eye_frame)
{

    if (flatshade == 1)
    {
        if (pointsource)
        {
            set_point_light(eye_frame);
        }
        else
        {
            set_spot_light(eye_frame);
        }
        set_sphere_m();
        mat4  model_view = mv ;
        glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view );
        mat3 normal_matrix = NormalMatrix(model_view, 1);
        glUniformMatrix3fv(glGetUniformLocation(program, "Normal_Matrix"),
                           1, GL_TRUE, normal_matrix );
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        if (text_sphere_flag == 1)
        {
            set_flag_sphere_tex();
            glUniform1i( glGetUniformLocation(program, "texture_1D"), 1 );
            glUniform1i( glGetUniformLocation(program, "texture_2D"), 0 );
            drawObj(s_flat_buffer, col * 3);
            cancel_flag_sphere();
        }
        else
        {drawObj(s_flat_buffer, col * 3);}
    }

    else if (smoothshade == 1)
    {
        if (pointsource)
        {
            set_point_light(eye_frame);
        }
        else
        {
            set_spot_light(eye_frame);
        }
        set_sphere_m();
        mat4  model_view = mv ;
        glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view );
        mat3 normal_matrix = NormalMatrix(model_view, 1);
        glUniformMatrix3fv(glGetUniformLocation(program, "Normal_Matrix"),
                           1, GL_TRUE, normal_matrix );
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        if (text_sphere_flag == 1)
        {
            set_flag_sphere_tex();
            glUniform1i( glGetUniformLocation(program, "texture_1D"), 1 );
            glUniform1i( glGetUniformLocation(program, "texture_2D"), 0 );
            glUniform1i( glGetUniformLocation(program, "my_texture_2D"), 2 );
            drawObj(s_smooth_buffer, col * 3);
            cancel_flag_sphere();
        }
        else
        {drawObj(s_smooth_buffer, col * 3);}
    }

    else
    {
        glUniform1f(glGetUniformLocation(program, "shade_flag"), 0);
        glUniformMatrix4fv(ModelView, 1, GL_TRUE, mv);
        if (solid_Flag == 0)
        {glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);}
        else
        {glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);}
        drawObj_2(sphere_buffer, sphere_NumVertices);
    }
}


void display( void )
{

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.529, 0.807, 0.92, 0.0);
    /*---  Set up and pass on Projection matrix to the shader ---*/
    mat4  p = Perspective(fovy, aspect, zNear, zFar);

    /*---  Set up and pass on Model-View matrix to the shader ---*/
    // eye is a global variable of vec4 set to init_eye and updated by keyboard()
    //at=eye+VPN
    vec4    at(0.0, 0.0, 0.0, 1.0);
    vec4    up(0.0, 1.0, 0.0, 0.0);
    mat4  mv = LookAt(eye, at, up);
    mat4 eye_frame = LookAt(eye, at, up);

    glUseProgram( program );

    ini_set_menu_flag();
    set_fog();

    ModelView = glGetUniformLocation( program, "ModelView" );
    Projection = glGetUniformLocation( program, "Projection" );
    glUniformMatrix4fv(Projection, 1, GL_TRUE, p); // GL_TRUE: matrix is row-major
    glEnable( GL_DEPTH_TEST );

    if (blend_shadow_flag == 1)
    {
        glDepthMask(GL_FALSE);
        draw_floor(mv, eye_frame);
        mv = LookAt(eye, at, up) * trans_p_to_q * Translate(centerPos.x, centerPos.y, centerPos.z) * acc_matrix;
        if (eye[1] > 0) draw_shadow(mv);

        glDepthMask(GL_TRUE);
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        mv = LookAt(eye, at, up);
        draw_floor(mv, eye_frame);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    }
    else
    {
        glDepthMask(GL_FALSE);
        draw_floor(mv, eye_frame);

        glDepthMask(GL_TRUE);
        mv = LookAt(eye, at, up) * trans_p_to_q * Translate(centerPos.x, centerPos.y, centerPos.z) * acc_matrix;
        if (eye[1] > 0) draw_shadow(mv);

        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        mv = LookAt(eye, at, up);
        draw_floor(mv, eye_frame);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    }

    glUniform1f(glGetUniformLocation(program, "shade_flag"), 0);
    glUniformMatrix4fv(ModelView, 1, GL_TRUE, eye_frame); // GL_TRUE: matrix is row-major
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    drawObj_2(line_buffer, 9); //draw xyz line

    if (lighting == 1) glUniform1f(glGetUniformLocation(program, "shade_flag"), 1.0);

    acc_matrix =  Rotate(delta, rotationAxis[currentSegment].x, rotationAxis[currentSegment].y, rotationAxis[currentSegment].z) * acc_matrix;
    mv = LookAt(eye, at, up) * Translate(centerPos.x, centerPos.y, centerPos.z) * acc_matrix;

    draw_sphere(mv, eye_frame);

    glUseProgram( program_2 );
    ModelView_2 = glGetUniformLocation( program_2, "model_view" );
    Projection_2 = glGetUniformLocation( program_2, "projection" );
    glUniformMatrix4fv(Projection_2, 1, GL_TRUE, p);
    glUniformMatrix4fv(ModelView_2, 1, GL_TRUE, eye_frame);
    glPointSize(3.0);
    glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
    float t = glutGet(GLUT_ELAPSED_TIME);
    int time = int((t - t_sub)) % 5000;
    if (begin1 == true)
        glUniform1f(glGetUniformLocation(program_2, "time"), time );
    else
        glUniform1f(glGetUniformLocation(program_2, "time"), 0 );

    if (firework_flag == 1) drawObj_4(firework_buffer, 300);

    glutSwapBuffers();
}

/* Compute distance between two points */
float distanceAt(point3 p1, point3 p2) {
    float dx = p1.x - p2.x;
    float dy = p1.y - p2.y;
    float dz = p1.z - p2.z;
    return sqrt3f(dx, dy, dz);
}

int nextModel() {
    int next = currentSegment + 1;
    return (next == totalSegments) ? 0 : next;
}
/* distance between current position and next point is greater than one between current track point and the next point*/
bool isTrespass() {
    int next = nextModel();
    point3 from = track[currentSegment];
    point3 to = track[next];
    float d1 = distanceAt(centerPos, from);
    float d2 = distanceAt(to, from);

    return d1 > d2;
}

void idle( void )
{
    //angle += 0.2;
    // angle += 1.0;    //YJC: change this value to adjust the cube rotation speed.
    //rotate by constant speed
    theta += delta;
    if (theta > 360.0)
        theta -= 360.0;
    //translate on direction
    float offset = (radius * delta * PI) / 180;
    centerPos.x = centerPos.x + vectors[currentSegment].x * offset;
    centerPos.y = centerPos.y + vectors[currentSegment].y * offset;
    centerPos.z = centerPos.z + vectors[currentSegment].z * offset;
    if (isTrespass()) {
        currentSegment = nextModel();
        centerPos = track[currentSegment];
    }

    //display
    glutPostRedisplay();
}
//----------------------------------------------------------------------------
void mouse( int button, int state, int x, int y )
{
    if (button == GLUT_RIGHT_BUTTON && state == GLUT_UP && begin1) {
        rolling = !rolling;
    }
    if (rolling) {
        // Stop rolling
        glutIdleFunc(idle);
    }
    else {
        glutIdleFunc(NULL);
    }
}

//----------------------------------------------------------------------------
void keyboard( unsigned char key, int x, int y )
{
    switch (key) {
    case 033: // Escape Key
    case 'q': case 'Q':
        exit(EXIT_SUCCESS);
        break;
    case 'b': case'B': //start Rolling
        t_sub = glutGet(GLUT_ELAPSED_TIME);
        begin1 = true;
        glutIdleFunc(idle);
        break;
    case 'X': eye[0] += 1.0; break;
    case 'x': eye[0] -= 1.0; break;
    case 'Y': eye[1] += 1.0; break;
    case 'y': eye[1] -= 1.0; break;
    case 'Z': eye[2] += 1.0; break;
    case 'z': eye[2] -= 1.0; break;

    case 'v': case'V':
        vertical_flag = 1;
        slant_flag = 0;
        break;
    case 's': case'S':
        vertical_flag = 0;
        slant_flag = 1;
        break;
    case 'o': case'O':
        object_space_flag = 1;
        eye_space_flag = 0;
        break;
    case 'e': case'E':
        object_space_flag = 0;
        eye_space_flag = 1;
        break;
    case 'u': case'U':
        upright_lat_flag = 0;
        break;
    case 't': case'T':
        upright_lat_flag = 1;
        break;
    case 'l': case'L':
        enable_lat = 1 - enable_lat;
        break;


    case 'a': case 'A': // Toggle between animation and non-animation
        animationFlag = 1 - animationFlag;
        if (animationFlag == 1) glutIdleFunc(idle);
        else                    glutIdleFunc(NULL);
        break;

    case ' ':  // reset to initial viewer/eye position
        eye = init_eye;
        break;
    }
    glutPostRedisplay();
}

void setDefaultView() {
    // initial viewer position
    eye = init_eye;
}
void quit() {
    delete[] vectors;
    delete[] rotationAxis;
    delete[]sphereData;
    delete[]sphere_color;
    exit(1);
}
void main_menu(int index)
{
    switch (index)
    {
    case (0) :
    {
        setDefaultView();
        break;
    }
    case (1) :
    {
        quit();
        break;
    }
    case (2) :
    {
        solid_Flag = !solid_Flag;
        break;
    }
    }
    display();
}

void shadow_menu(int index) {
    shadow = (index == 1) ? false : true;
    display();
};

void shade_menu(int index) {
    flatshade = (index == 1) ? true : false;
    if (flatshade) {
        flatshade = flatshade;
    } else {
        smoothshade = !smoothshade;
        if (smoothshade == 1) {flatshade = 0;}
    }
    solid_Flag = 1;
    display();
};

void lighting_menu(int index) {
    lighting = (index == 1) ? false : true;
    display();
};

void spotlight_menu(int index) {
    spotlight = (index == 1) ? true : false;
    if (spotlight) {
        spotlight = 1;
        pointsource = 0;
    }
    else
    {
        spotlight = 0;
        pointsource = 1;
    }
    display();
};

void fog_menu(int index)
{
    if (index >= 0) fog_flag = index;
    else fog_flag = 1;
    display();
};

void tex_ground_menu(int index)
{
    text_ground_flag = (index == 2) ? 1 : 0;
    display();
}

void tex_sphere_menu(int index)
{
    if (index == 1)
    {
        text_sphere_flag = 0;
    }
    if (index == 2)
    {
        text_sphere_flag = 1;
        text_sphere_line_flag = 1;
        text_sphere_check_flag = 0;
    }
    if (index == 3)
    {
        text_sphere_flag = 1;
        text_sphere_line_flag = 0;
        text_sphere_check_flag = 1;
    }
    display();
}

void blend_shadow_menu(int index)
{
    blend_shadow_flag = (index == 2) ? 1 : 0;
    display();
}
void firework_menu(int index)
{
    firework_flag = (index == 2) ? 1 : 0;
    display();
}
/* Add menu to mouse left button*/
void addMenu() {
    int shadow = glutCreateMenu(shadow_menu);
    glutAddMenuEntry("No", 1);
    glutAddMenuEntry("Yes", 2);

    int shade = glutCreateMenu(shade_menu);
    glutAddMenuEntry("flat shading", 1);
    glutAddMenuEntry("smooth shading", 2);

    int lighting = glutCreateMenu(lighting_menu);
    glutAddMenuEntry("No", 1);
    glutAddMenuEntry("Yes", 2);

    int text_ground_tmp = glutCreateMenu(tex_ground_menu);
    glutAddMenuEntry("No", 1);
    glutAddMenuEntry("Yes", 2);

    int text_sphere_tmp = glutCreateMenu(tex_sphere_menu);
    glutAddMenuEntry("No", 1);
    glutAddMenuEntry("Yes-Contour Lines", 2);
    glutAddMenuEntry("Yes-Checkerboard", 3);

    int blend_shadow_tmp = glutCreateMenu(blend_shadow_menu);
    glutAddMenuEntry("No", 1);
    glutAddMenuEntry("Yes", 2);

    int firework_tmp = glutCreateMenu(firework_menu);
    glutAddMenuEntry("No", 1);
    glutAddMenuEntry("Yes", 2);

    int spotlight = glutCreateMenu(spotlight_menu);
    glutAddMenuEntry("Spot light", 1);
    glutAddMenuEntry("Point light", 2);

    int fog_option = glutCreateMenu(fog_menu);
    glutAddMenuEntry("No fog", 1);
    glutAddMenuEntry("Linear fog", 2);
    glutAddMenuEntry("Exponential fog", 3);
    glutAddMenuEntry("Exponential square fog", 4);

    glutCreateMenu(main_menu);
    glutAddMenuEntry("Default View Point", 0);
    glutAddMenuEntry("Quit", 1);
    glutAddMenuEntry("Wire Frame Sphere", 2);
    glutAddSubMenu("Enable Lighting", lighting);
    glutAddSubMenu("Shadow", shadow);
    glutAddSubMenu("Shading", shade);
    glutAddSubMenu("Lighting", spotlight);
    glutAddSubMenu("Fog option", fog_option);
    glutAddSubMenu("Texture Mapped Ground", text_ground_tmp);
    glutAddSubMenu("Texture Mapped Sphere", text_sphere_tmp);
    glutAddSubMenu("Blending Shadow", blend_shadow_tmp);
    glutAddSubMenu("Fireworks", firework_tmp);

    glutAttachMenu(GLUT_LEFT_BUTTON);
}
//----------------------------------------------------------------------------
void reshape( int width, int height )
{
    glViewport( 0, 0, width, height );
    aspect = (GLfloat) width  / (GLfloat) height;
    glutPostRedisplay();
}
//----------------------------------------------------------------------------
int main( int argc, char **argv )
{    int err;

    glutInit( &argc, argv );
    glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );
    glutInitWindowSize( 512, 512 );
    glutCreateWindow( "hw4" );

    readFiles();
    addMenu();
    glutDisplayFunc( display );
    glutReshapeFunc( reshape );
    glutKeyboardFunc( keyboard );

    init();
    glutMainLoop();

    delete[]sphereData;
    delete[]sphere_color;
    return 0;
}
