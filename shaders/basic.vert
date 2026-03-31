#version 330 core

layout (location = 0) in vec2 aPos;

void main() 
{
	gl_position = vec4(aPos, 0.0, 1.0);

}