#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} fs_in;

struct Material{
    sampler2D diffuseTexture;
    sampler2D specularTexture;

    vec3 diffuseColor;
    vec3 specularColor;

    float shininess;
};
uniform Material material;

#define MAX_POINT_LIGHTS 16
struct PointLight{
    vec3 position;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform int noPointLights;

uniform samplerCube depthMaps[MAX_POINT_LIGHTS];

//uniform vec3 lightPos;
uniform vec3 viewPos;

uniform float far_plane;
uniform bool shadows;
uniform bool noTex;

float ShadowCalculation(int i, vec3 fragPos);
vec3 PointLightCalc(int i, vec3 norm);

void main(){  
    vec3 norm = normalize(fs_in.Normal);
   vec3 result = vec3(0.0);

   for(int i = 0; i < noPointLights; i++){
        result += PointLightCalc(i, norm);
   }
    
    FragColor =vec4(result, 1.0f);
}

float ShadowCalculation(int i, vec3 fragPos){
    vec3 fragToLight = fragPos - pointLights[i].position;
    float currentDepth = length(fragToLight);

    float shadow  = 0.0;
    float bias    = 0.05; 
    float samples = 4.0;
    float offset  = 0.1;
    for(float x = -offset; x < offset; x += offset / (samples * 0.5)){
        for(float y = -offset; y < offset; y += offset / (samples * 0.5)){
            for(float z = -offset; z < offset; z += offset / (samples * 0.5))        {
                float closestDepth = texture(depthMaps[i], fragToLight + vec3(x, y, z)).r; 
                closestDepth *= far_plane;   // undo mapping [0;1]
                if(currentDepth - bias > closestDepth)
                    shadow += 1.0;
                }
            }
        }
    shadow /= (samples * samples * samples);
    return shadow;
}

vec3 PointLightCalc(int i, vec3 norm){

    vec3 diffuseColor;
    vec3 specularColor;

    if(noTex){
        diffuseColor = material.diffuseColor;
        specularColor = material.specularColor;
    }
    else{
        diffuseColor = texture(material.diffuseTexture, fs_in.TexCoords).rgb;
        specularColor = texture(material.specularTexture, fs_in.TexCoords).rgb;
    }

    vec3 ambient = pointLights[i].ambient * diffuseColor;

    vec3 lightDir = normalize(pointLights[i].position - fs_in.FragPos);
    float diff = max(dot(lightDir, norm), 0.0);
    vec3 diffuse = pointLights[i].diffuse * (diff * diffuseColor);

    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), material.shininess);
    vec3 specular = pointLights[i].specular * (spec * specularColor);

    // attenuation
    float dist = length(pointLights[i].position - fs_in.FragPos);
    float attenuation = 1.0 / ((pointLights[i].constant) + (pointLights[i].linear * dist) + (pointLights[i].quadratic * (dist * dist)));

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    float shadow = shadows? ShadowCalculation(i, fs_in.FragPos) : 0.0;
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular));

    return lighting;
}
