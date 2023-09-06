/* 
File Name: "fshader53.glsl":
           Fragment Shader
*/

//#version 150  // YJC: Comment/un-comment this line to resolve compilation errors
                 //      due to different settings of the default GLSL version

in vec4 color;
flat in int fogMode;
in float fog_d;
in vec2 texCoord;
in vec2 lat_Coord;

out vec4 fColor;

uniform sampler2D texture_2D;
uniform sampler1D texture_1D;
uniform int ground_texture_flag;
uniform int f_sphere_flag;
uniform int f_sphere_check_flag;
uniform int enable_lat;
uniform int f_draw_shadow_lat;
in  float float_texture;


void main() 
{ 
	fColor = color;
    vec4 tex_color = color;

	if (ground_texture_flag==1 && f_sphere_flag==0)
	{
		tex_color = color*texture(texture_2D,texCoord);
	}

	if (f_sphere_flag==1 && ground_texture_flag==0 )
	{
		tex_color=color*texture(texture_1D,float_texture);
		if(enable_lat>0)
		{
			float s=lat_Coord[0];
			float t=lat_Coord[1];
			//if(fract(4*s)<0.35 && fract(4*s)>0 && fract(4*t)>0 && fract(4*t)<0.35) discard;
		}
	}

	if(f_sphere_check_flag==1  && ground_texture_flag==0)
	{
		vec4 tmp_color=texture(texture_2D,texCoord);
		if(tmp_color[0]==0)
		{
			tmp_color=vec4(0.9,0.1,0.1,1.0);
		}
		tex_color=color*tmp_color;
		if(enable_lat>0)
		{
			float s=lat_Coord[0];
			float t=lat_Coord[1];
			//if(fract(4*s)<0.35 && fract(4*s)>0 && fract(4*t)>0 && fract(4*t)<0.35) discard;
		}
	}

	if(f_draw_shadow_lat==1)
	{
		if(enable_lat>0)
		{
			float s=lat_Coord[0];
			float t=lat_Coord[1];
			//if(fract(4*s)<0.35 && fract(4*s)>0 && fract(4*t)>0 && fract(4*t)<0.35) discard;
		}
	}
    

	vec4 fogColor = vec4(0.7, 0.7, 0.7, 0.5); //fog color

	if (fogMode == 0){
		fColor =tex_color;
	}
	else if(fogMode == 1){
		float temp = clamp(fog_d, 0.0, 18.0);
		fColor=mix(tex_color,fogColor,temp/18);
	}
	else if (fogMode == 2){
		float temp=1/exp(0.09*fog_d);
		fColor=mix(tex_color,fogColor,1-temp);
	}
	else if (fogMode == 3){
		float temp =1/exp(0.09*0.09*fog_d*fog_d);
		fColor=mix(tex_color,fogColor,1-temp);
	}


	
	else{
	fColor = vec4(0, 0, 0, 1);
	}
	
	
} 

