#version 330 core

layout (triangles) in;
layout (triangle_strip, max_vertices=3) out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

in vec3 outCol[];

out vec3 triCol;

const vec3 light = normalize(vec3(0.3, 0.3, 0.8));

vec3 calc_normal()
{
	vec3 tan; 
	vec3 bitan; 
	if (gl_in[0].gl_Position.y == gl_in[1].gl_Position.y)
	{
		bitan = gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz;
		tan = gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz;
	}
	else
	{
		tan = gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz;
		bitan = gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz;
	}
	vec3 normal = normalize(cross(tan, bitan));
	return normal;
}

void main()
{
	vec3 normal = calc_normal();
	vec3 color = (outCol[0].xyz + outCol[1].xyz + outCol[2].xyz) / 3.0;
	triCol = color * max(dot(-light, normal), 0.3);

	gl_Position = proj * view * model * gl_in[0].gl_Position;
	EmitVertex();

	gl_Position = proj * view * model * gl_in[1].gl_Position;
	EmitVertex();

	gl_Position = proj * view * model * gl_in[2].gl_Position;
	EmitVertex();

	EndPrimitive();
}