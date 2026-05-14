#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} fs_in;

struct Material{
    sampler2D diffuseTexture;
    sampler2D spcularTexture;
    float shininess;
};
uniform Material material;

struct Light{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
uniform Light light;

uniform sampler2D diffuseTexture;
uniform samplerCube depthMap;

uniform vec3 lightPos;
uniform vec3 viewPos;

uniform float far_plane;
uniform bool shadows;

vec3 material_Lighting(Light light, vec3 norm, Material material);

float ShadowCalculation(vec3 fragPos){
    vec3 fragToLight = fragPos - lightPos;
    //float closestDepth = texture(depthMap, normalize(fragToLight)).r;
    //closestDepth *= far_plane;
    float currentDepth = length(fragToLight);

    //vec3 normal = normalize(fs_in.Normal);
    //vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    //float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005); 
    //float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

    float shadow  = 0.0;
    float bias    = 0.05; 
    float samples = 4.0;
    float offset  = 0.1;
    for(float x = -offset; x < offset; x += offset / (samples * 0.5)){
        for(float y = -offset; y < offset; y += offset / (samples * 0.5)){
            for(float z = -offset; z < offset; z += offset / (samples * 0.5))        {
                float closestDepth = texture(depthMap, fragToLight + vec3(x, y, z)).r; 
                closestDepth *= far_plane;   // undo mapping [0;1]
                if(currentDepth - bias > closestDepth)
                    shadow += 1.0;
                }
            }
        }
    shadow /= (samples * samples * samples);
    return shadow;
}

void main(){  
    vec3 norm = normalize(fs_in.Normal);
    vec3 l1 = material_Lighting(light, norm, material);
    
    FragColor =vec4(l1, 1.0f);
}

vec3 material_Lighting(Light light, vec3 norm, Material material){
    vec3 ambient = light.ambient * texture(material.diffuseTexture, fs_in.TexCoords).rgb;

    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    float diff = max(dot(lightDir, norm), 0.0);
    vec3 diffuse = light.diffuse * (diff * vec3(texture(material.diffuseTexture, fs_in.TexCoords)));

    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), material.shininess);
    vec3 specular = light.specular * (spec * vec3(texture(material.spcularTexture, fs_in.TexCoords)));

    float shadow = shadows? ShadowCalculation(fs_in.FragPos) : 0.0;
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular));

    return lighting;
}