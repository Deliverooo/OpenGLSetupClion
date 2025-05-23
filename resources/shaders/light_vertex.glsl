#version 330 core
layout (location = 0) in vec3 aPos;   // the position variable has attribute position 0

uniform mat4 light;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * light * vec4(aPos, 1.0);
}