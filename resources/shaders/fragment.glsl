#version 330 core

in vec2 TexCoords;
in vec3 Normal;
in vec3 VertexPosWorld;
in vec3 BaseColour;

out vec4 FragColour;

uniform sampler2D diffuseTex1;
uniform sampler2D diffuseTex2;
uniform sampler2D specularTex1;
uniform sampler2D specularTex2;

uniform vec3 viewPos;

struct Material{

    int specularRoughness;
    float specularIntensity;
    vec3 specularTint;

};
uniform Material material;

struct PointLight{
    vec3 lightPos;

    vec3 ambient;
    vec3 lightColour;

    float constant;
    float linear;
    float quadratic;
};
#define NR_POINT_LIGHTS 1
uniform PointLight pointLights[NR_POINT_LIGHTS];

struct DirectionLight{
    vec3 direction;
    vec3 ambient;
    vec3 lightColour;

};
uniform DirectionLight directionLight;

struct SpotLight{

    vec3 direction;
    vec3 position;

    vec3 ambient;
    vec3 lightColour;

    float constant;
    float linear;
    float quadratic;

    float cutOffAngle;
    float outerCutOffAngle;

};
#define NR_SPOT_LIGHTS 1
uniform SpotLight spotlights[NR_SPOT_LIGHTS];

vec3 calculateDirectionalLight(DirectionLight directionLight, vec3 normal, vec3 viewDirection){

    vec3 unitDirectionalLightDirection = normalize(-directionLight.direction);

    float diff = max(dot(normal, unitDirectionalLightDirection), 0.0f);

    vec3 specularReflectDirection = reflect(-unitDirectionalLightDirection, normal);

    float spec = pow(max(dot(viewDirection, specularReflectDirection), 0.0f), material.specularRoughness);

    vec3 ambient  = directionLight.ambient * vec3(texture(diffuseTex1, TexCoords));
    vec3 diffuse  = directionLight.lightColour * diff * vec3(texture(diffuseTex1, TexCoords));
    vec3 specular = (material.specularTint * spec * vec3(texture(specularTex1, TexCoords))) * material.specularIntensity;

    return ambient + diffuse + specular;
}
vec3 calculatePointLight(PointLight pointLight, vec3 normal, vec3 viewDirection, vec3 fragPos){

    vec3 unitLightDirection = normalize(pointLight.lightPos - fragPos);

    vec3 specularReflectDirection = reflect(-unitLightDirection, normal);

    float diff = max(dot(normal, unitLightDirection), 0.0f);
    float spec = pow(max(dot(viewDirection, specularReflectDirection), 0.0f), material.specularRoughness);

    vec3 ambient  = pointLight.ambient * vec3(texture(diffuseTex1, TexCoords));
    vec3 diffuse  = pointLight.lightColour * diff * (vec3(texture(diffuseTex1, TexCoords)) + BaseColour);
    vec3 specular = (material.specularTint * spec * vec3(texture(specularTex1, TexCoords))) * material.specularIntensity;

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
        float fadeIntensity = smoothstep(0.0f, 1.0f, ((theta - spotlight.outerCutOffAngle) / epsilon));

        float diff = max(dot(normal, unitLightDirection), 0.0f);
        float spec = pow(max(dot(viewDirection, specularReflectDirection), 0.0f), material.specularRoughness);

        vec3 ambient  = spotlight.ambient * vec3(texture(diffuseTex1, TexCoords));
        vec3 diffuse  = spotlight.lightColour * diff * vec3(texture(diffuseTex1, TexCoords));
        vec3 specular = (material.specularTint * spec * vec3(texture(specularTex1, TexCoords)) * material.specularIntensity);

        diffuse *= fadeIntensity;
        specular *= fadeIntensity;

        ambient *= attenuation;
        diffuse *= attenuation;
        specular *= attenuation;

        return ambient + diffuse + specular;
    }

    return vec3(spotlight.ambient * vec3(texture(diffuseTex1, TexCoords))) * clamp(attenuation, 0.5f, 1.0f);
}

void main()
{

    vec3 unitNormal = normalize(Normal);
    vec3 viewDirection = normalize(viewPos - VertexPosWorld);

    vec3 result = vec3(0.0f);

    for(int i = 0; i < NR_POINT_LIGHTS; i++){

        result += calculatePointLight(pointLights[i], unitNormal, viewDirection, VertexPosWorld);
    }
    for(int i = 0; i < NR_SPOT_LIGHTS; i++){

        result += calculateSpotLight(spotlights[i], unitNormal, viewDirection, VertexPosWorld);
    }

    FragColour = vec4(result, 1.0f);
}