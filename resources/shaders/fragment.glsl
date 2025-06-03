#version 330 core

in vec2 TexCoords;
in vec3 Normal;
in vec3 VertexPosWorld;

out vec4 FragColour;
uniform sampler2D diffuseTex1;
uniform sampler2D specularTex1;
uniform sampler2D diffuseTex2;
uniform sampler2D specularTex2;

uniform samplerCube skybox;

uniform vec3 viewPos;
uniform bool useTex;

struct Attenuation{

    float constant;
    float linear;
    float quadratic;
};
struct Material{

    vec3 baseColour;
    int specularRoughness;
    float shininess;
    vec3 specularTint;
};
uniform Material material;

struct Light{
    vec3 colour;
    vec3 position;
    float ambientStrength;
    float strength;
};

struct PointLight{

    Light parentLight;
    Attenuation attenuation;
};

struct DirectionLight{
    Light parentLight;
    vec3 direction;

};

struct SpotLight{

    Light parentLight;
    vec3 direction;

    Attenuation attenuation;

    float cutOffAngle;
    float outerCutOffAngle;
};

#define NR_DIRECTIONAL_LIGHT 1
uniform DirectionLight directionLight;

#define NR_POINT_LIGHTS 1
uniform PointLight pointLights[NR_POINT_LIGHTS];

#define NR_SPOT_LIGHTS 1
uniform SpotLight spotlights[NR_SPOT_LIGHTS];

vec3 calcLight(Light light, vec3 normal, vec3 viewDirection, vec3 fragPos, Attenuation attenuation){

    vec3 unitLightDirection = normalize(light.position - fragPos);
    vec3 specularReflectDirection = reflect(-unitLightDirection, normal);

    float distance = length(light.position - fragPos);
    float atten = 1.0f / (attenuation.constant + (attenuation.linear * distance) + (attenuation.quadratic * pow(distance, 2)));


    float diff = max(dot(normal, unitLightDirection), 0.0f);
    float spec = pow(max(dot(viewDirection, specularReflectDirection), 0.0f), material.specularRoughness);

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    if(!useTex){
        ambient  = light.ambientStrength * vec3(material.baseColour);
        diffuse  = light.colour * diff * (vec3(material.baseColour));
        specular = (material.specularTint * spec * vec3(material.baseColour)) * material.shininess;
    } else {

        ambient  = light.ambientStrength * vec3(texture(diffuseTex1, TexCoords));
        diffuse  = light.colour * diff * (vec3(texture(diffuseTex1, TexCoords)));
        specular = (material.specularTint * spec * vec3(texture(specularTex1, TexCoords))) * material.shininess;
    }

    ambient *= atten;
    diffuse *= atten;
    specular *= atten;

    return ambient + diffuse + specular;

}
vec3 calculateDirectionalLight(DirectionLight directionLight, vec3 normal, vec3 viewDirection){

    vec3 unitDirectionalLightDirection = normalize(-directionLight.direction);

    float diff = max(dot(normal, unitDirectionalLightDirection), 0.0f);

    vec3 specularReflectDirection = reflect(-unitDirectionalLightDirection, normal);

    float spec = pow(max(dot(viewDirection, specularReflectDirection), 0.0f), material.specularRoughness);

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    if(!useTex){
        ambient  = directionLight.parentLight.ambientStrength * vec3(material.baseColour);
        diffuse  = directionLight.parentLight.colour * diff * (vec3(material.baseColour));
        specular = (material.specularTint * spec * vec3(material.baseColour)) * material.shininess;
    } else {
        ambient  = directionLight.parentLight.ambientStrength * vec3(texture(diffuseTex1, TexCoords));
        diffuse  = directionLight.parentLight.strength * diff * vec3(texture(diffuseTex1, TexCoords));
        specular = (material.specularTint * spec * vec3(texture(specularTex1, TexCoords))) * material.shininess;
    }


    return ambient + diffuse + specular;
}

vec3 calculatePointLight(PointLight pointLight, vec3 normal, vec3 viewDirection, vec3 fragPos){

    return calcLight(pointLight.parentLight, normal, viewDirection, fragPos, pointLight.attenuation);

}
vec3 calculateSpotLight(SpotLight spotlight, vec3 normal, vec3 viewDirection, vec3 fragPos){

    vec3 unitLightDirection = normalize(spotlight.parentLight.position - fragPos);

    vec3 specularReflectDirection = reflect(-unitLightDirection, normal);

    float theta = dot(unitLightDirection, normalize(-spotlight.direction));

    float distance = length(spotlight.parentLight.position - fragPos);
    float attenuation = 1.0f / (spotlight.attenuation.constant + (spotlight.attenuation.linear * distance) + (spotlight.attenuation.quadratic * pow(distance, 2)));

    float diff = max(dot(normal, unitLightDirection), 0.0f);
    float spec = pow(max(dot(viewDirection, specularReflectDirection), 0.0f), material.specularRoughness);

    if(theta > spotlight.outerCutOffAngle){

        float epsilon = spotlight.cutOffAngle - spotlight.outerCutOffAngle;
        float fadeIntensity = smoothstep(0.0f, 1.0f, ((theta - spotlight.outerCutOffAngle) / epsilon));

        vec3 ambient;
        vec3 diffuse;
        vec3 specular;

        if(!useTex){
            ambient  = spotlight.parentLight.ambientStrength * vec3(material.baseColour);
            diffuse  = spotlight.parentLight.colour * diff * vec3(material.baseColour);
            specular = (material.specularTint * spec * vec3(material.baseColour)) * material.shininess;
        } else {
            ambient  = spotlight.parentLight.ambientStrength * vec3(texture(diffuseTex1, TexCoords));
            diffuse  = spotlight.parentLight.colour * diff * vec3(texture(diffuseTex1, TexCoords));
            specular = (material.specularTint * spec * vec3(texture(specularTex1, TexCoords))* material.shininess);
        }

        diffuse *= fadeIntensity;
        specular *= fadeIntensity;

        ambient *= attenuation;
        diffuse *= attenuation;
        specular *= attenuation;

        return ambient + diffuse + specular;
    }

    if(!useTex){
        return vec3(spotlight.parentLight.ambientStrength * vec3(material.baseColour)) * (material.specularTint * spec) * clamp(attenuation, 0.5f, 1.0f);
    } else{
        return vec3(spotlight.parentLight.ambientStrength * vec3(texture(diffuseTex1, TexCoords))) * clamp(attenuation, 0.5f, 1.0f);
    }
}

void main()
{
    vec3 unitNormal = normalize(Normal);
    vec3 viewDirection = normalize(VertexPosWorld - viewPos);

    vec3 result = calculateDirectionalLight(directionLight, unitNormal, viewDirection);

    for(int i = 0; i < NR_POINT_LIGHTS; i++){

        result += calculatePointLight(pointLights[i], unitNormal, viewDirection, VertexPosWorld);
    }
    for(int i = 0; i < NR_SPOT_LIGHTS; i++){

        result += calculateSpotLight(spotlights[i], unitNormal, viewDirection, VertexPosWorld);
    }

    float near = 0.1f;
    float far = 100.0f;
    float normalizedDeviceCoords = gl_FragCoord.z * 2.0f -1;
    float linearDepth = ((2.0 * near * far) / (far + near - normalizedDeviceCoords * (far - near))) / far;

    vec3 refraction = refract(viewDirection, unitNormal, 0.658);
    vec3 skyTex = vec3(texture(skybox, refraction).rgb);
    FragColour = vec4(result, 1.0);

}