#version 330 core

in vec2 TexCoord;
in vec3 Normal;
in vec3 VertexPosWorld;

out vec4 FragColour;

uniform sampler2D loooow;

struct Material{

    sampler2D diffuse;
    sampler2D specular;
    int specularRoughness;
};
uniform Material material;

struct PointLight{
    vec3 lightPos;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};
#define NR_POINT_LIGHTS 2
uniform PointLight pointLights[NR_POINT_LIGHTS];

struct DirectionLight{
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

};
uniform DirectionLight directionLight;

struct SpotLight{

    vec3 direction;
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;

    float cutOffAngle;
    float outerCutOffAngle;

};
uniform SpotLight spotlight;

uniform float mixFactor;
uniform vec3 viewPos;

vec3 calculateDirectionalLight(DirectionLight directionLight, vec3 normal, vec3 viewDirection){

    vec3 unitDirectionalLightDirection = normalize(-directionLight.direction);

    float diff = max(dot(normal, unitDirectionalLightDirection), 0.0f);

    vec3 specularReflectDirection = reflect(-unitDirectionalLightDirection, normal);

    float spec = pow(max(dot(viewDirection, specularReflectDirection), 0.0f), material.specularRoughness);

    vec3 ambient  = directionLight.ambient * vec3(texture(material.diffuse, TexCoord));
    vec3 diffuse  = directionLight.diffuse * diff * vec3(texture(material.diffuse, TexCoord));
    vec3 specular = directionLight.specular * spec * vec3(texture(material.specular, TexCoord));

    return ambient + diffuse + specular;
}
vec3 calculatePointLight(PointLight pointLight, vec3 normal, vec3 viewDirection, vec3 fragPos){

    vec3 unitLightDirection = normalize(pointLight.lightPos - fragPos);

    vec3 specularReflectDirection = reflect(unitLightDirection, normal);

    float diff = max(dot(normal, unitLightDirection), 0.0f);
    float spec = pow(max(dot(viewDirection, specularReflectDirection), 0.0f), material.specularRoughness);

    vec3 ambient  = pointLight.ambient * vec3(texture(material.diffuse, TexCoord));
    vec3 diffuse  = pointLight.diffuse * diff * vec3(mix(texture(loooow, TexCoord), texture(material.diffuse, TexCoord), mixFactor));
    vec3 specular = pointLight.specular * spec * vec3(texture(material.specular, TexCoord));

    float distance = length(pointLight.lightPos - fragPos);
    float attenuation = 1.0f / (pointLight.constant + (pointLight.linear * distance) + (pointLight.quadratic * pow(distance, 2)));

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return ambient + diffuse + specular;
}
vec3 calculateSpotLight(SpotLight spotlight, vec3 normal, vec3 viewDirection, vec3 fragPos){

    vec3 unitLightDirection = normalize(spotlight.position - fragPos);

    vec3 specularReflectDirection = reflect(-unitLightDirection, normal);

    float theta = dot(unitLightDirection, normalize(-spotlight.direction));

    float distance = length(spotlight.position - fragPos);
    float attenuation = 1.0f / (spotlight.constant + (spotlight.linear * distance) + (spotlight.quadratic * pow(distance, 2)));

    if(theta > spotlight.outerCutOffAngle){

        float epsilon = spotlight.cutOffAngle - spotlight.outerCutOffAngle;
        float fadeIntensity = smoothstep(0.0f, 1.0f, (theta - spotlight.outerCutOffAngle) / epsilon);

        float diff = max(dot(normal, unitLightDirection), 0.0f);
        float spec = pow(max(dot(viewDirection, specularReflectDirection), 0.0f), material.specularRoughness);

        vec3 ambient  = spotlight.ambient * vec3(texture(material.diffuse, TexCoord)) * attenuation;
        vec3 diffuse  = spotlight.diffuse * diff * vec3(texture(material.diffuse, TexCoord)) * attenuation;
        vec3 specular = spotlight.specular * spec * vec3(texture(material.specular, TexCoord)) * attenuation;

        diffuse *= fadeIntensity;
        specular *= fadeIntensity;

        return ambient + diffuse + specular;
    }

    return vec3(spotlight.ambient * vec3(texture(material.diffuse, TexCoord))) * clamp(attenuation, 0.5f, 1.0f);
}
void main()
{

    vec3 unitNormal = normalize(Normal);
    vec3 viewDirection = normalize(viewPos - VertexPosWorld);

    vec3 result = calculateDirectionalLight(directionLight, unitNormal, viewDirection);
    result += calculateSpotLight(spotlight, unitNormal, viewDirection, VertexPosWorld);

    for(int i = 0; i < NR_POINT_LIGHTS; i++){

        result += calculatePointLight(pointLights[i], unitNormal, viewDirection, VertexPosWorld);
    }

    FragColour = vec4(result, 1.0f);

}