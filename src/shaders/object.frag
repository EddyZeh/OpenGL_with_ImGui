#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
} fs_in;

struct Material{
    sampler2D diffuseTexture;
    sampler2D specularTexture;

    vec3 diffuseColor;
    vec3 specularColor;

    float shininess;
};
uniform Material material;

struct DirLight{
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
uniform DirLight dirLight;
uniform bool hasDirLight;
uniform sampler2D dirDepthMap;

#define MAX_POINT_LIGHTS 16
struct PointLight{
    vec3 position;
        
    float k0;
    float k1;
    float k2;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform int noPointLights;
uniform samplerCube pointDepthMaps[MAX_POINT_LIGHTS];

#define MAX_SPOT_LIGHTS 5
struct SpotLight{
    vec3 position;
    vec3 direction;

    float cutOff;
    float outerCutOff;

    float k0;
    float k1;
    float k2;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
uniform SpotLight spotLights[MAX_SPOT_LIGHTS];
uniform int noSpotLights;
uniform sampler2D spotDepthMaps[MAX_SPOT_LIGHTS];
uniform mat4 spotLightSpaceMatrix[MAX_SPOT_LIGHTS];

uniform vec3 viewPos;

uniform float far_plane;
uniform bool shadows;
uniform bool noTex;

float DirShadowCalculation(vec4 fragPosLightSpace);
vec3 DirLightCalculation(vec3 norm, vec3 diffuseColor, vec3 specularColor);

float PointShadowCalculation(int i, vec3 fragPos);
vec3 PointLightCalc(int i, vec3 norm, vec3 diffuseColor, vec3 specularColor);

float SpotShadowCalculation(int i, vec4 fragPosLightSpace);
vec3 SpotLightCalculation(int i, vec3 norm, vec3 diffuseColor, vec3 specularColor);

vec3 diffuseColor;
vec3 specularColor;

void main(){  
    if(noTex){
        diffuseColor = material.diffuseColor;
        specularColor = material.specularColor;
    }
    else{
        diffuseColor = texture(material.diffuseTexture, fs_in.TexCoords).rgb;
        specularColor = texture(material.specularTexture, fs_in.TexCoords).rgb;
    }

    vec3 norm = normalize(fs_in.Normal);
   vec3 result = vec3(0.0);

   for(int i = 0; i < noPointLights; i++){
        result += PointLightCalc(i, norm, diffuseColor, specularColor);
   }
    
    FragColor =vec4(result, 1.0f);
}

float DirShadowCalculation(vec4 fragPosLightSpace){
    // perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

    // transform to [0,1]
    projCoords = projCoords * 0.5 + 0.5;

    // outside shadow map
    if(projCoords.z > 1.0)
        return 0.0;

    float currentDepth = projCoords.z;

    float bias = max(0.005 * (1.0 - dot(fs_in.Normal, -dirLight.direction)), 0.001);

    // PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(dirDepthMap, 0);

    for(int x = -1; x <= 1; ++x){
        for(int y = -1; y <= 1; ++y){
            float closestDepth = texture(dirDepthMap, projCoords.xy + vec2(x,y) * texelSize).r;

            if(currentDepth - bias > closestDepth)
                shadow += 1.0;
        }
    }

    shadow /= 9.0;
    return shadow;
}

vec3 DirLightCalculation(vec3 norm, vec3 diffuseColor, vec3 specularColor){
    vec3 lightDir = normalize(-dirLight.direction);
   
    // AMBIENT
    vec3 ambient = dirLight.ambient * diffuseColor;

    // DIFFUSE
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = dirLight.diffuse * (diff * diffuseColor);

    // SPECULAR
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), material.shininess);
    vec3 specular = dirLight.specular * (spec * specularColor);

    float shadow = shadows? DirShadowCalculation(fs_in.FragPosLightSpace) : 0.0;

    return (ambient + (1.0 - shadow) * (diffuse + specular));
}

float PointShadowCalculation(int i, vec3 fragPos){
    vec3 fragToLight = fragPos - pointLights[i].position;
    float currentDepth = length(fragToLight);

    float shadow  = 0.0;
    float bias    = 0.05; 
    float samples = 4.0;
    float offset  = 0.1;
    for(float x = -offset; x < offset; x += offset / (samples * 0.5)){
        for(float y = -offset; y < offset; y += offset / (samples * 0.5)){
            for(float z = -offset; z < offset; z += offset / (samples * 0.5))        {
                float closestDepth = texture(pointDepthMaps[i], fragToLight + vec3(x, y, z)).r; 
                closestDepth *= far_plane;   // undo mapping [0;1]
                if(currentDepth - bias > closestDepth)
                    shadow += 1.0;
                }
            }
        }
    shadow /= (samples * samples * samples);
    return shadow;
}

vec3 PointLightCalc(int i, vec3 norm, vec3 diffuseColor, vec3 specularColor){

    vec3 lightDir = normalize(pointLights[i].position - fs_in.FragPos);

    // AMBIENT
    vec3 ambient = pointLights[i].ambient * diffuseColor;

    // DIFFUSE

    float diff = max(dot(lightDir, norm), 0.0);
    vec3 diffuse = pointLights[i].diffuse * (diff * diffuseColor);

    // SPECULAR

    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), material.shininess);
    vec3 specular = pointLights[i].specular * (spec * specularColor);

    // ATTENUATION
    float dist = length(pointLights[i].position - fs_in.FragPos);
    float attenuation = 1.0 / ((pointLights[i].k0) + (pointLights[i].k1 * dist) + (pointLights[i].k2 * (dist * dist)));

    ambient *= attenuation;
    diffuse *= attenuation; 
    specular *= attenuation;

    float shadow = shadows? PointShadowCalculation(i, fs_in.FragPos) : 0.0;
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular));

    return lighting;
}

float SpotShadowCalculation(int i, vec4 fragPosLightSpace){
    // perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

    // transform to [0,1]
    projCoords = projCoords * 0.5 + 0.5;

    if(projCoords.z > 1.0)
        return 0.0;

    float currentDepth = projCoords.z;

    float bias = max(0.005 * (1.0 - dot(fs_in.Normal, -dirLight.direction)), 0.001);
    float shadow = 0;
    vec2 texelSize = 1.0 / textureSize(spotDepthMaps[i], 0);

    for(int x = -1; x <= 1; ++x){
        for(int y = -1; y <= 1; ++y){
            float closestDepth = texture(spotDepthMaps[i], projCoords.xy + vec2(x,y) * texelSize).r;
            if(currentDepth - bias > closestDepth){
                shadow += 1.0;
            }
        }
    }
    shadow /= 9.0;
    return shadow;
}
vec3 SpotLightCalculation(int i, vec3 norm, vec3 diffuseColor, vec3 specularColor){
    vec3 lightDir = normalize(spotLights[i].position - fs_in.FragPos);
    float theta = dot(lightDir, normalize(-spotLights[i].direction));

    // AMBIENT
    vec3 ambient = spotLights[i].ambient * diffuseColor;

    if(theta > spotLights[i].outerCutOff){
    // DIFFUSE

        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = spotLights[i].diffuse * (diff * diffuseColor);
        
    // SPECULAR
        
        vec3 viewDir = normalize(viewPos - fs_in.FragPos);
        vec3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(norm, halfwayDir), 0.0), material.shininess);
        vec3 specular = spotLights[i].specular * (spec * specularColor);

    //  INTENSITY

        float intensity = (theta - spotLights[i].outerCutOff) / (spotLights[i].cutOff - spotLights[i].outerCutOff);
        intensity = clamp(intensity, 0.0, 1.0);
        diffuse  *= intensity;
        specular *= intensity;

    // ATTENUATION

        float dist = length(spotLights[i].position - fs_in.FragPos);
        float attenuation = 1.0 / (spotLights[i].k0 + spotLights[i].k1 * dist + spotLights[i].k2 * (dist * dist));

        diffuse *= attenuation;
        specular *= attenuation;

        float shadow = shadows? SpotShadowCalculation(i, fs_in.FragPosLightSpace) : 0.0;
        vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular));

        return lighting;
    }
    else{
        return ambient;        
    }

}