#version 330 core

in vec2 TexCoords;
in vec3 Normal;
in vec3 VertexPosWorld;

out vec4 FragColour;

uniform sampler2D diffuseTex1;
uniform sampler2D specularTex1;
uniform sampler2D diffuseTex2;
uniform sampler2D specularTex2;
uniform sampler2D normalTex1;
uniform sampler2D normalTex2;
uniform sampler2D heightTex1;
uniform sampler2D heightTex2;

uniform vec3 viewPos;
uniform bool useTex;

uniform float shininess;

uniform samplerCube skybox;

struct Light{
    vec3 colour;
    vec3 position;

    float linear;
    float quadratic;
};
#define NR_LIGHTS 2
uniform Light lights[NR_LIGHTS];

vec3 calcBlinnPhong();

void main()
{
    vec3 unitNormal = normalize(Normal);
    vec3 viewDirection = normalize(VertexPosWorld - viewPos);

    vec3 result = vec3(0.0f);

    for(int i = 0; i < NR_LIGHTS; i++){

        vec3 unitLightDirection = normalize(lights[i].position - VertexPosWorld);
        vec3 specularReflectDirection = reflect(-unitLightDirection, unitNormal);

        float distance = length(lights[i].position - VertexPosWorld);
        float atten = 1.0f / (1 + (lights[i].linear * distance) + (lights[i].quadratic * pow(distance, 2)));

        float diff = max(dot(unitNormal, unitLightDirection), 0.0f);
        float spec = pow(max(dot(viewDirection, specularReflectDirection), 0.0f), 4);

        vec3 diffuse  = lights[i].colour * diff * (vec3(texture(diffuseTex1, TexCoords)));
        vec3 specular = spec * vec3(texture(specularTex1, TexCoords)) * 0.3f;
        vec3 ambient = vec3(texture(diffuseTex1, TexCoords)) * 0.2f;

        diffuse *= atten;
        specular *= atten;

        result += (diffuse + specular + ambient);
    }

    vec3 refraction = reflect(viewDirection, unitNormal);
    vec3 skyTex = vec3(texture(skybox, refraction).rgb);

    FragColour = vec4(result, 1.0f);
}