#version 330 core

in vec2 TexCoord;
in vec3 Normal;
in vec3 VertexPosWorld;

out vec4 FragColour;

struct Material{

    sampler2D diffuse;
    sampler2D specular;
    sampler2D emission;
    float emissionStrength;
    int specularRoughness;
};
uniform Material material;

struct Light{
    vec3 lightPos;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;

};
uniform Light light;

struct DirectionLight{
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

};
uniform DirectionLight directionLight;

uniform float mixFactor;
uniform vec3 viewPos;
uniform int lightType;


void main()
{
    if(lightType == 0){
        vec3 unitNormal = normalize(Normal);
        vec3 unitDirectionalLightDirection = normalize(-directionLight.direction);

        vec3 viewDirection = normalize(viewPos - VertexPosWorld);
        vec3 specularReflectDirection = reflect(-unitDirectionalLightDirection, unitNormal);

        float diff = max(dot(unitNormal, unitDirectionalLightDirection), 0.0f);
        float spec = pow(max(dot(viewDirection, specularReflectDirection), 0.0f), material.specularRoughness);

        vec3 emission = material.emissionStrength * vec3(texture(material.emission, TexCoord));
        vec3 ambient  = directionLight.ambient * vec3(texture(material.diffuse, TexCoord));
        vec3 diffuse  = directionLight.diffuse * diff * vec3(texture(material.diffuse, TexCoord));
        vec3 specular = directionLight.specular * spec * vec3(texture(material.specular, TexCoord));
        vec3 resultColour = ambient + diffuse + specular + emission;


        FragColour = vec4(resultColour,1.0f);
    }
    if(lightType == 1){
        vec3 unitNormal = normalize(Normal);
        vec3 unitLightDirection = normalize(light.lightPos - VertexPosWorld);

        vec3 viewDirection = normalize(viewPos - VertexPosWorld);
        vec3 specularReflectDirection = reflect(-unitLightDirection, unitNormal);

        float diff = max(dot(unitNormal, unitLightDirection), 0.0f);
        float spec = pow(max(dot(viewDirection, specularReflectDirection), 0.0f), material.specularRoughness);

        vec3 emission = material.emissionStrength * vec3(texture(material.emission, TexCoord));
        vec3 ambient  = light.ambient * vec3(texture(material.diffuse, TexCoord));
        vec3 diffuse  = light.diffuse * diff * vec3(texture(material.diffuse, TexCoord));
        vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoord));

        float distance = length(light.lightPos - VertexPosWorld);
        float attenuation = 1.0f / (light.constant + (light.linear * distance) + (light.quadratic * pow(distance, 2)));

        ambient *= attenuation;
        diffuse *= attenuation;
        specular *= attenuation;

        vec3 resultColour = ambient + diffuse + specular + emission;


        FragColour = vec4(resultColour,1.0f);
    }



}