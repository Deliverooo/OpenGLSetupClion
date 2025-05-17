#version 330 core

in vec2 TexCoord;
in vec3 Normal;
in vec3 VertexPosWorld;

out vec4 FragColour;

uniform sampler2D texture1;
uniform sampler2D texture2;
uniform float mixFactor;
uniform float ambientIntensity;
uniform vec3 lightColour;
uniform vec3 lightPos;


void main()
{
    vec3 unitLightDirection = normalize(lightPos - VertexPosWorld);
    float shadowIntensity = max(dot(normalize(Normal), unitLightDirection), 0.0f);

    vec3 diffuse = shadowIntensity * lightColour;
    vec3 ambient = ambientIntensity * lightColour;

    vec3 textureColour = mix(texture(texture1, TexCoord), texture(texture2, TexCoord), mixFactor).xyz;
    vec3 resultColour = (ambient + diffuse) * textureColour;

    FragColour = vec4(resultColour, 1.0f);

}