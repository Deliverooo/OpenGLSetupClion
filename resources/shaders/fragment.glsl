#version 330 core

in vec2 TexCoord;
in vec3 ourColour;

out vec4 FragColour;

uniform sampler2D texture1;
uniform sampler2D texture2;

void main()
{
    FragColour = mix(texture(texture1, TexCoord), texture(texture2, TexCoord), 1.0f);
}