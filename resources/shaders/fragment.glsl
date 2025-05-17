#version 330 core

in vec2 TexCoord;
in vec3 ourColour;

out vec4 FragColour;

uniform sampler2D texture1;
uniform sampler2D texture2;
uniform float mixFactor;

uniform vec3 objectColour;
uniform vec3 lightColour;

void main()
{
    vec4 mixColour = mix(vec4(ourColour, 1), mix(texture(texture1, TexCoord), texture(texture2, TexCoord), mixFactor), 0.9f);
    FragColour = vec4(lightColour * mixColour.xyz, 1.0f);
}