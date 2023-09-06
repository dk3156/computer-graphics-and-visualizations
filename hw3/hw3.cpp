//Dongje Kim, dk3156, Computer Graphics, Assignment 3
#include "Angel-yjc.h"
#include <math.h>
#include <iostream>
#include <fstream>
#include <string>
using namespace std;

typedef Angel::vec3  color3;
typedef Angel::vec3  point3;
typedef Angel::vec4  color4;
typedef Angel::vec4  point4;

GLuint program;      
GLuint program_2;
GLuint sphere_buffer;
GLuint sphere_shadow_buffer;
GLuint floor_light_buffer;
GLuint floor_buffer;
GLuint line_buffer;
GLuint flat_buffer;
GLuint smooth_buffer;

GLfloat  fovy = 45.0;
GLfloat  aspect;
GLfloat  zNear = 0.5, zFar = 13.0;

GLfloat radius;

GLfloat angle = 0.0; 
vec4 init_eye(7.0, 3.0, -10.0, 1.0);
vec4 eye = init_eye;

int animationFlag = 1; // 1: animation; 0: non-animation. Toggled by key 'a' or 'A'
int solid_Flag = 0;

int flatshade = 0;
int shadow = 1;
int lighting = 0;
int smoothshade = 0;
int spotlight = 0;
int pointsource = 1;

point3 track[] = { point3(3.0, 1.0, 5.0), point3(-2.0, 1.0, -2.5), point3(2.0, 1.0, -4.0) };
int currentSegment = 0, totalSegments = 3;
GLfloat theta = 0.0, delta = 0.5;
point3 centerPos = track[currentSegment];
point3*vectors;
point3*rotationAxis;
#define PI 3.1415926536
#define sqrt3f(x,y,z) sqrt(x*x+y*y+z*z)
mat4 accumulated_m = Angel::identity();
bool begin1 = false, rolling = false;

point4 floor_light_points[6];
vec3   floor_normals[6];

const int floor_NumVertices = 6; 
point3 floor_points[floor_NumVertices];
color3 floor_colors[floor_NumVertices];

point3 floor_vertices[4] = {
	point3( 5, 0,  8),
	point3( 5, 0,  -4),
	point3(  -5,  0,  -4),
	point3(  -5,  0,  8),
};

point4 floor_vertices2[4] = {
	point4( 5, 0,  8, 1.0),
	point4( 5, 0,  -4, 1.0),
	point4(  -5,  0,  -4, 1.0),
	point4(  -5,  0,  8, 1.0),
};

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

GLuint  ModelView, Projection;
GLuint  model_view_2; 
GLuint  projection_2;

//Initialization of lighting shader paramters
void point_light_source(mat4 mv)
{
	color4 ambient_light( 0, 0, 0, 1.0 );
	color4 diffuse_light( 1.0, 1.0, 1.0, 1.0 );
	color4 specular_light( 1.0, 1.0, 1.0, 1.0 );
	float const_val = 2.0;
	float linear_val = 0.01;
	float quad_val = 0.001;
	point4 light_position(-14.0, 12.0, -3, 1.0 );

	vec4 eye_frame_lightpos = mv * light_position;
	glUniform4fv( glGetUniformLocation(program, "LightPosition"),
	              1, eye_frame_lightpos);

	glUniform1f(glGetUniformLocation(program, "ConstVal"),
	            const_val);
	glUniform1f(glGetUniformLocation(program, "LinearVal"),
	            linear_val);
	glUniform1f(glGetUniformLocation(program, "QuadVal"),
	            quad_val);
	glUniform4fv(glGetUniformLocation(program, "input_ambient_light"), 1,
	             ambient_light);
	glUniform4fv(glGetUniformLocation(program, "input_diffuse_light"), 1,
	             diffuse_light);
	glUniform4fv(glGetUniformLocation(program, "input_specular_light"), 1,
	             specular_light);
	glUniform1f(glGetUniformLocation(program, "point_flag"), pointsource * 1.0);
	glUniform1f(glGetUniformLocation(program, "spot_flag"), 0.0);
}

void spot_light_source(mat4 mv)
{
	color4 ambient_light( 0, 0, 0, 1.0 );
	color4 diffuse_light( 1.0, 1.0, 1.0, 1.0 );
	color4 specular_light( 1.0, 1.0, 1.0, 1.0 );
	float const_val = 2.0;
	float linear_val = 0.01;
	float quad_val = 0.001;
	point4 light_position(-14, 12.0, -3, 1.0 );

	float spot_exp = 15.0;
	float spot_ang = cos(20.0 * PI / 180);

	vec4 eye_frame_lightpos = mv * light_position;
	glUniform4fv( glGetUniformLocation(program, "LightPosition"),
	              1, eye_frame_lightpos);

	glUniform1f(glGetUniformLocation(program, "ConstVal"),
	            const_val);
	glUniform1f(glGetUniformLocation(program, "LinearVal"),
	            linear_val);
	glUniform1f(glGetUniformLocation(program, "QuadVal"),
	            quad_val);
	glUniform4fv(glGetUniformLocation(program, "input_ambient_light"), 1,
	             ambient_light);
	glUniform4fv(glGetUniformLocation(program, "input_diffuse_light"), 1,
	             diffuse_light);
	glUniform4fv(glGetUniformLocation(program, "input_specular_light"), 1,
	             specular_light);
	glUniform1f(glGetUniformLocation(program, "point_flag"), 0.0);
	glUniform1f(glGetUniformLocation(program, "spot_flag"), 1.0);
	point4 spot_direction = mv * vec4(-6.0, 0.0, -4.5, 1.0);
	glUniform4fv(glGetUniformLocation(program, "spot_direction"), 1,
	             spot_direction);
	glUniform1f(glGetUniformLocation(program, "spot_exp"), spot_exp);
	glUniform1f(glGetUniformLocation(program, "spot_ang"), spot_ang);

}

void init_floor_matrix()
{
	color4 ambient_texture( 0.2, 0.2, 0.2, 1.0 );
	color4 diffuse_texture( 0, 1.0, 0, 1 );
	color4 specular_texture( 0, 0, 0, 1 );
	float  shine_texture = 125.0;
	glUniform4fv(glGetUniformLocation(program, "ini_ambient_texture"), 1,
	             ambient_texture);
	glUniform4fv(glGetUniformLocation(program, "ini_diffuse_texture"), 1,
	             diffuse_texture);
	glUniform4fv(glGetUniformLocation(program, "ini_specular_texture"), 1,
	             specular_texture);
	glUniform1f(glGetUniformLocation(program, "Shininess"), 1.0);
}

void init_sphere_matrix()
{
	color4 ambient_texture( 0.2, 0.2, 0.2, 1.0 );
	color4 diffuse_texture( 1.0, 0.84, 0, 1 );
	color4 specular_texture( 1.0, 0.84, 0, 1 );
	float  shine_texture = 125;
	glUniform4fv(glGetUniformLocation(program, "ini_ambient_texture"), 1,
	             ambient_texture);
	glUniform4fv(glGetUniformLocation(program, "ini_diffuse_texture"), 1,
	             diffuse_texture);
	glUniform4fv(glGetUniformLocation(program, "ini_specular_texture"), 1,
	             specular_texture);
	glUniform1f(glGetUniformLocation(program, "Shininess"), shine_texture);

}

void floor()
{
	floor_colors[0] = vertex_colors[3]; floor_points[0] = floor_vertices[0];
	floor_colors[1] = vertex_colors[3]; floor_points[1] = floor_vertices[1];
	floor_colors[2] = vertex_colors[3]; floor_points[2] = floor_vertices[2];
	floor_colors[3] = vertex_colors[3]; floor_points[3] = floor_vertices[0];
	floor_colors[4] = vertex_colors[3]; floor_points[4] = floor_vertices[3];
	floor_colors[5] = vertex_colors[3]; floor_points[5] = floor_vertices[2];
}

point3 line_points[9];
color3 line_colors[9];
void axis()
{
	line_colors[0] = vertex_colors[1]; line_points[0] = point3(0, 0, 0);
	line_colors[1] = vertex_colors[1]; line_points[1] = point3(10, 0, 0);
	line_colors[2] = vertex_colors[1]; line_points[2] = point3(20, 0, 0);

	line_colors[3] = vertex_colors[5]; line_points[3] = point3(0, 0, 0);
	line_colors[4] = vertex_colors[5]; line_points[4] = point3(0, 10, 0);
	line_colors[5] = vertex_colors[5]; line_points[5] = point3(0, 20, 0);

	line_colors[6] = vertex_colors[4]; line_points[6] = point3(0, 0, 0);
	line_colors[7] = vertex_colors[4]; line_points[7] = point3(0, 0, 10);
	line_colors[8] = vertex_colors[4]; line_points[8] = point3(0, 0, 20);
}
int Index = 0;

void quad( int a, int b, int c, int d )
{
	vec4 u = floor_vertices2[b] - floor_vertices2[a];
	vec4 v = floor_vertices2[d] - floor_vertices2[a];

	vec3 normal_temp = normalize( cross(v, u) );
	floor_normals[Index] = normal_temp; floor_light_points[Index] = floor_vertices2[a]; Index++;
	floor_normals[Index] = normal_temp; floor_light_points[Index] = floor_vertices2[b]; Index++;
	floor_normals[Index] = normal_temp; floor_light_points[Index] = floor_vertices2[c]; Index++;
	floor_normals[Index] = normal_temp; floor_light_points[Index] = floor_vertices2[a]; Index++;
	floor_normals[Index] = normal_temp; floor_light_points[Index] = floor_vertices2[c]; Index++;
	floor_normals[Index] = normal_temp; floor_light_points[Index] = floor_vertices2[d]; Index++;
}

void floor_light()
{
	Index = 0;
	quad(1, 0, 3, 2);
}

point3 calculateDirection(point3 from, point3 to) {
	point3 v;
	v.x = to.x - from.x;
	v.y = to.y - from.y;
	v.z = to.z - from.z;

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

point3 *sphere_data_a;
color3 *sphere_color;
int sphere_NumVertices;
int col;
point3 *sphere_shadow_data;
color3 *sphere_shadow_color;

point4 *sphere_shade_d;
vec3 *sphere_flat_shade;
vec3 *sphere_smooth_shade;

//Initialziation of flat shading
void flat_init()
{
	int flat_index = 0;
	for (int j = 0; j < col; j++)
	{

		vec4 curr_p1 = sphere_shade_d[3 * j];
		vec4 curr_p2 = sphere_shade_d[3 * j + 1];
		vec4 curr_p3 = sphere_shade_d[3 * j + 2];


		vec4 u = curr_p2 - curr_p1;
		vec4 v = curr_p3 - curr_p1;

		vec3 normal_temp = normalize( cross(u, v) );
		sphere_flat_shade[flat_index] = normal_temp;  flat_index++;
		sphere_flat_shade[flat_index] = normal_temp;  flat_index++;
		sphere_flat_shade[flat_index] = normal_temp;  flat_index++;

	}
};

//Initialization of smooth shading
void smooth_init()
{
	for (int i = 0; i < col * 3; i++)
	{
		vec4 curr_p = sphere_shade_d[i];
		vec3 tmp = vec3(curr_p[0], curr_p[1], curr_p[2]);
		tmp = normalize(tmp);
		sphere_smooth_shade[i] = tmp;
	}
};

//Reading in files and initialzing buffers
void read_files()
{
	ifstream fp;
	char* filename = new char[100];

	int points;
	float temp[3];

	int count = 0;
    cout << "Type in sphere.1024 or sphere.256." << endl;

	cin.getline(filename, 100);

	if (strlen(filename) == 0) {
		filename = (char*)"sphere.1024";
	}
	fp.open(filename);
	if (!fp) {
		cerr << "Failed to open file " << filename << " !" << endl;
		exit(0);
	}
	fp >> col;
	sphere_data_a = new point3[col * 3];
	sphere_shadow_data = new point3[col * 3];
	sphere_color = new point3[col * 3];
	sphere_shadow_color = new point3[col * 3];
	sphere_shade_d = new point4[col * 3];
	sphere_flat_shade = new point3[col * 3];
	sphere_smooth_shade = new point3[col * 3];

	for (int i = 0; i < col; i++) {
		fp >> points;
		for (int j = 0; j < points; j++) {
			for (int k = 0; k < 3; k++) {
				fp >> temp[k];

			}
			point3 tmp( temp[0], temp[1], temp[2]);
			point3 tmp2( temp[0], temp[1], temp[2]);
			point4 tmp_shade = vec4(temp[0], temp[1], temp[2], 1);

			sphere_data_a[count] = tmp;
			sphere_shadow_data[count] = tmp2;
			sphere_shade_d[count] = tmp_shade;
			count++;
		}
	}
	sphere_NumVertices = col * 3;
	fp.close();
	flat_init();
	smooth_init();
}

//Initialziation of sphere colors
void colorsphere()
{
	for (int i = 0; i < col * 3; i++) {
		sphere_color[i] = color3(1.0, 0.84, 0.0);
		sphere_shadow_color[i] = color3(0.25, 0.25, 0.25);
	}

}

void init()
{
	radius = 1;
	colorsphere();

	totalSegments = sizeof(track) / sizeof(point3);
	vectors = new point3[totalSegments];
	rotationAxis = new point3[totalSegments];
	for (int i = 0; i < totalSegments - 1; i++) {
		vectors[i] = calculateDirection(track[i], track[i + 1]);
	}

	vectors[totalSegments - 1] = calculateDirection(track[totalSegments - 1], track[0]);

	point3 y_axis(0, 1, 0 );
	for (int i = 0; i < totalSegments; i++) {
		rotationAxis[i] = crossProduct(y_axis, vectors[i]);
	}

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

	glGenBuffers( 1, &flat_buffer );
	glBindBuffer( GL_ARRAY_BUFFER, flat_buffer );
	glBufferData( GL_ARRAY_BUFFER, 3 * col * sizeof(point4) + 3 * col * sizeof(color3),
	              NULL, GL_STATIC_DRAW );
	glBufferSubData( GL_ARRAY_BUFFER, 0, 3 * col * sizeof(point4), sphere_shade_d );
	glBufferSubData( GL_ARRAY_BUFFER, 3 * col * sizeof(point4),
	                 3 * col * sizeof(color3), sphere_flat_shade );

	glGenBuffers( 1, &smooth_buffer );
	glBindBuffer( GL_ARRAY_BUFFER, smooth_buffer );
	glBufferData( GL_ARRAY_BUFFER, 3 * col * sizeof(point4) + 3 * col * sizeof(color3),
	              NULL, GL_STATIC_DRAW );
	glBufferSubData( GL_ARRAY_BUFFER, 0, 3 * col * sizeof(point4), sphere_shade_d );
	glBufferSubData( GL_ARRAY_BUFFER, 3 * col * sizeof(point4),
	                 3 * col * sizeof(color3), sphere_smooth_shade );

	axis();
	glGenBuffers(1, &line_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, line_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(line_points) + sizeof(line_colors),
	             NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(line_points), line_points);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(line_points), sizeof(line_colors),
	                line_colors);

	glGenBuffers(1, &sphere_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, sphere_buffer);
	glBufferData(GL_ARRAY_BUFFER, 3 * col * sizeof(point3) + 3 * col * sizeof(color3),
	             NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, 3 * col * sizeof(point3), sphere_data_a);
	glBufferSubData(GL_ARRAY_BUFFER, 3 * col * sizeof(point3), 3 * col * sizeof(color3),
	                sphere_color);

	glGenBuffers(1, &sphere_shadow_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, sphere_shadow_buffer);
	glBufferData(GL_ARRAY_BUFFER, 3 * col * sizeof(point3) + 3 * col * sizeof(color3),
	             NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, 3 * col * sizeof(point3), sphere_shadow_data);
	glBufferSubData(GL_ARRAY_BUFFER, 3 * col * sizeof(point3), 3 * col * sizeof(color3),
	                sphere_shadow_color);

    program = InitShader( "vshader53.glsl", "fshader53.glsl" );
    program_2 = InitShader( "vshader42.glsl", "fshader42.glsl" );

	glEnable( GL_DEPTH_TEST );
	glClearColor( 0.0, 0.0, 0.0, 1.0 );
	glLineWidth(2.0);
}


void drawObj(GLuint buffer, int num_vertices)
{
	glBindBuffer(GL_ARRAY_BUFFER, buffer);

	GLuint vPosition = glGetAttribLocation( program, "vPosition" );
	glEnableVertexAttribArray( vPosition );
	glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,
	                       BUFFER_OFFSET(0) );

	GLuint vNormal = glGetAttribLocation( program, "vNormal" );
	glEnableVertexAttribArray( vNormal );
	glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0,
	                       BUFFER_OFFSET( num_vertices * sizeof(point4)) );
	glDrawArrays(GL_TRIANGLES, 0, num_vertices);

	glDisableVertexAttribArray(vPosition);
	glDisableVertexAttribArray(vNormal);
}

void drawObj_2(GLuint buffer, int num_vertices)
{
	glBindBuffer(GL_ARRAY_BUFFER, buffer);

	GLuint vPosition = glGetAttribLocation(program_2, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0,
	                      BUFFER_OFFSET(0));

	GLuint vColor = glGetAttribLocation(program_2, "vColor");
	glEnableVertexAttribArray(vColor);
	glVertexAttribPointer(vColor, 3, GL_FLOAT, GL_FALSE, 0,
	                      BUFFER_OFFSET(sizeof(point3) * num_vertices));

	glDrawArrays(GL_TRIANGLES, 0, num_vertices);

	glDisableVertexAttribArray(vPosition);
	glDisableVertexAttribArray(vColor);
}

mat4 transpose_p(12, 0, 0, 0, 14, 0, 3, -1, 0, 0, 12, 0, 0, 0, 0, 12);

void set_menu_flag()
{
	if (solid_Flag == 0 || lighting == 0)
	{
		flatshade = 0;
		smoothshade = 0;
	}
}

void display( void )
{
	set_menu_flag();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.529, 0.807, 0.92, 0.0);
	mat4  p = Perspective(fovy, aspect, zNear, zFar);

	vec4    at(0.0, 0.0, 0.0, 1.0);
	vec4    up(0.0, 1.0, 0.0, 0.0);
	mat4  mv = LookAt(eye, at, up);
	mat4 eye_frame = LookAt(eye, at, up);
	glDisable( GL_DEPTH_TEST );

	if (lighting == 0)
	{
		glUseProgram(program_2); 
		model_view_2 = glGetUniformLocation(program_2, "model_view");
		projection_2 = glGetUniformLocation(program_2, "projection");
		glUniformMatrix4fv(projection_2, 1, GL_TRUE, p);
		glUniformMatrix4fv(model_view_2, 1, GL_TRUE, mv);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		drawObj_2(floor_buffer, floor_NumVertices);
	}
	else
	{
		glUseProgram( program );
		ModelView = glGetUniformLocation( program, "ModelView" );
		Projection = glGetUniformLocation( program, "Projection" );
		glUniformMatrix4fv(Projection, 1, GL_TRUE, p);
		if (pointsource)
		{
			point_light_source(eye_frame);
		}
		else
		{
			spot_light_source(eye_frame);
		}
		init_floor_matrix();
		mat4  model_view = mv ;
		glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view );
		mat3 normal_matrix = NormalMatrix(model_view, 1);
		glUniformMatrix3fv(glGetUniformLocation(program, "Normal_Matrix"),
		                   1, GL_TRUE, normal_matrix );
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		drawObj(floor_light_buffer, 6);
	}

	glUseProgram(program_2);
	glEnable( GL_DEPTH_TEST );

	glUniformMatrix4fv(model_view_2, 1, GL_TRUE, mv);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	drawObj_2(line_buffer, 9);

	accumulated_m =  Rotate(delta, rotationAxis[currentSegment].x, rotationAxis[currentSegment].y, rotationAxis[currentSegment].z) * accumulated_m;
	mv = LookAt(eye, at, up) * Translate(centerPos.x, centerPos.y, centerPos.z) * accumulated_m;

	if (flatshade == 1)
	{
		glUseProgram( program );
		ModelView = glGetUniformLocation( program, "ModelView" );
		Projection = glGetUniformLocation( program, "Projection" );
		glUniformMatrix4fv(Projection, 1, GL_TRUE, p);

		if (pointsource)
		{
			point_light_source(eye_frame);
		}
		else
		{
			spot_light_source(eye_frame);
		}

		init_sphere_matrix();
		mat4  model_view = mv ;
		glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view );
		mat3 normal_matrix = NormalMatrix(model_view, 1);
		glUniformMatrix3fv(glGetUniformLocation(program, "Normal_Matrix"),
		                   1, GL_TRUE, normal_matrix );
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		drawObj(flat_buffer, col * 3);
	}
	else if (smoothshade == 1)
	{
		glUseProgram( program );
		ModelView = glGetUniformLocation( program, "ModelView" );
		Projection = glGetUniformLocation( program, "Projection" );
		glUniformMatrix4fv(Projection, 1, GL_TRUE, p);

		if (pointsource)
		{
			point_light_source(eye_frame);
		}
		else
		{
			spot_light_source(eye_frame);
		}
		init_sphere_matrix();
		mat4  model_view = mv ;
		glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view );
		mat3 normal_matrix = NormalMatrix(model_view, 1);
		glUniformMatrix3fv(glGetUniformLocation(program, "Normal_Matrix"),
		                   1, GL_TRUE, normal_matrix );
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		drawObj(smooth_buffer, col * 3);
	}
	else
	{
		glUseProgram(program_2);
		model_view_2 = glGetUniformLocation(program_2, "model_view");
		projection_2 = glGetUniformLocation(program_2, "projection");
		glUniformMatrix4fv(projection_2, 1, GL_TRUE, p);
		glUniformMatrix4fv(model_view_2, 1, GL_TRUE, mv);
		if (solid_Flag == 0)
		{glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);}
		else
		{glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);}
		drawObj_2(sphere_buffer, sphere_NumVertices);
	}

	glUseProgram( program_2 );
	if (shadow == 1)
	{
		mv = LookAt(eye, at, up) * transpose_p * Translate(centerPos.x, centerPos.y, centerPos.z) * accumulated_m;
		glUniformMatrix4fv(model_view_2, 1, GL_TRUE, mv);
		if (solid_Flag == 0)
		{glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);}
		else
		{glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);}
		drawObj_2(sphere_shadow_buffer, sphere_NumVertices);
	}


	glutSwapBuffers();
}

//Get distance between two points
float distance_at(point3 p1, point3 p2) {
	float dx = p1.x - p2.x;
	float dy = p1.y - p2.y;
	float dz = p1.z - p2.z;
	return sqrt3f(dx, dy, dz);
}

int next_model() {
	int next = currentSegment + 1;
	return (next == totalSegments) ? 0 : next;
}

//returns True if the distance between current point and the next point is larger than one between current track point and the next
bool model_check() {
	int next = next_model();
	point3 from = track[currentSegment];
	point3 to = track[next];
	float d1 = distance_at(centerPos, from);
	float d2 = distance_at(to, from);

	return d1 > d2;
}

void idle( void )
{
	theta += delta;
	if (theta > 360.0)
		theta -= 360.0;

	float offset = (radius * delta * PI) / 180;
	centerPos.x = centerPos.x + vectors[currentSegment].x * offset;
	centerPos.y = centerPos.y + vectors[currentSegment].y * offset;
	centerPos.z = centerPos.z + vectors[currentSegment].z * offset;
	if (model_check()) {
		currentSegment = next_model();
		centerPos = track[currentSegment];
	}

	glutPostRedisplay();
}

//mouse handler
void mouse( int button, int state, int x, int y )
{
	if (button == GLUT_RIGHT_BUTTON && state == GLUT_UP && begin1) {
		rolling = !rolling;
	}
	if (rolling) {
		glutIdleFunc(idle);
	}
	else {
		glutIdleFunc(NULL);
	}
}

//keyboard handler
void keyboard( unsigned char key, int x, int y )
{
	switch (key) {
	case 'q': case 'Q':
		exit(EXIT_SUCCESS);
		break;
	case 'b': case'B':
		begin1 = true;
		glutIdleFunc(idle);
		break;
	case 'X': eye[0] += 1.0; break;
	case 'x': eye[0] -= 1.0; break;
	case 'Y': eye[1] += 1.0; break;
	case 'y': eye[1] -= 1.0; break;
	case 'Z': eye[2] += 1.0; break;
	case 'z': eye[2] -= 1.0; break;

	case 'a': case 'A': 
		animationFlag = 1 - animationFlag;
		if (animationFlag == 1) glutIdleFunc(idle);
		else                    glutIdleFunc(NULL);
		break;

	case ' ':
		eye = init_eye;
		break;
	}
	glutPostRedisplay();
}

void setDefaultView() {
	eye = init_eye;
}
void quit() {
	delete[] vectors;
	delete[] rotationAxis;
	delete[]sphere_data_a;
	delete[]sphere_color;
	exit(1);
}

//main menu handler
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

//menu for toggle shadow
void shadow_menu(int index) {
	shadow = (index == 1) ? false : true;
	display();
};

//menu for shading
void shade_menu(int index) {
	flatshade = (index == 1) ? true : false;
	if (flatshade) {
		flatshade = flatshade;
	} else {
		smoothshade = !smoothshade;
		if (smoothshade == 1) {flatshade = 0;}
	}
	display();
};

//menu for lighting
void lighting_menu(int index) {
	lighting = (index == 1) ? false : true;
	display();
};

//menu for spotlight source
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

//Menu handler
void addMenu() {
	int shadow = glutCreateMenu(shadow_menu);
	glutAddMenuEntry(" No ", 1);
	glutAddMenuEntry(" Yes ", 2);

	int shade = glutCreateMenu(shade_menu);
	glutAddMenuEntry(" flat shading ", 1);
	glutAddMenuEntry(" smooth shading ", 2);

	int lighting = glutCreateMenu(lighting_menu);
	glutAddMenuEntry(" No ", 1);
	glutAddMenuEntry(" Yes ", 2);

	int spotlight = glutCreateMenu(spotlight_menu);
	glutAddMenuEntry(" Spot light ", 1);
	glutAddMenuEntry(" Point light ", 2);

	glutCreateMenu(main_menu);
	glutAddMenuEntry(" Default View Point ", 0);


	glutAddMenuEntry(" Quit ", 1);
	glutAddMenuEntry(" Wire Frame Sphere ", 2);
	glutAddSubMenu(" Enable Lighting ", lighting);
	glutAddSubMenu(" Shadow ", shadow);
	glutAddSubMenu(" Shading ", shade);
	glutAddSubMenu(" Lighting ", spotlight);

	glutAttachMenu(GLUT_LEFT_BUTTON);
}

void reshape( int width, int height )
{
	glViewport( 0, 0, width, height );
	aspect = (GLfloat) width  / (GLfloat) height;
	glutPostRedisplay();
}

int main( int argc, char **argv )
{	int err;

	glutInit( &argc, argv );
	glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );
	glutInitWindowSize( 512, 512 );
	glutCreateWindow( "DJ - Homework3" );

	read_files();
	addMenu();
	glutDisplayFunc( display );
	glutReshapeFunc( reshape );
	glutKeyboardFunc( keyboard );

	init();
	glutMainLoop();

	delete[]sphere_data_a;
	delete[]sphere_color;
	return 0;
}
