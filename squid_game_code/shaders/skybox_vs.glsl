#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 texcoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	vec4 pos = projection * view * vec4(aPos, 1.0);
	gl_Position = vec4(pos.x,pos.y,pos.w,pos.w);
	texcoord = vec3(aPos.x,aPos.y,aPos.z);
}