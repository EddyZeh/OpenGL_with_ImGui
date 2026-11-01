#version 330 core
/*
    Escape Room surface shader.
    Same lighting model and uniform names as the engine's object.frag (so DirLight / PointLight /
    SpotLight::render() and LightUnits::configureSamplers() work unchanged), plus:
      - world-space box-projected UVs so textures tile on walls instead of stretching
      - emissive colour (light fixtures, keypad screens, clue lights)
      - highlight (pulsing tint on the object the player is aiming at)
      - unlit mode (debug collider wireframes)
      - per point light shadow switch
      - shadow samplers indexed with constants only (portable to strict GLSL 3.30 drivers)
*/
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} fs_in;

struct Material{
    sampler2D diffuseTexture;
    sampler2D specularTexture;

    vec3 diffuseColor;      // colour, or tint when textured
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
uniform mat4 dirLightSpaceMatrix;

#define MAX_POINT_LIGHTS 8
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
uniform bool pointCastsShadow[MAX_POINT_LIGHTS];

#define MAX_SPOT_LIGHTS 4
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

// Escape Room additions
uniform bool worldUV;
uniform vec2 uvScale;
uniform vec3 emissive;
uniform float highlight;
uniform bool unlit;
uniform vec3 unlitColor;

// ---------------------------------------------------------------- constant-index sampler access
float samplePointDepth(int i, vec3 dir){
    if(i == 0) return texture(pointDepthMaps[0], dir).r;
    if(i == 1) return texture(pointDepthMaps[1], dir).r;
    if(i == 2) return texture(pointDepthMaps[2], dir).r;
    if(i == 3) return texture(pointDepthMaps[3], dir).r;
    if(i == 4) return texture(pointDepthMaps[4], dir).r;
    if(i == 5) return texture(pointDepthMaps[5], dir).r;
    if(i == 6) return texture(pointDepthMaps[6], dir).r;
    return texture(pointDepthMaps[7], dir).r;
}

float sampleSpotDepth(int i, vec2 uv){
    if(i == 0) return texture(spotDepthMaps[0], uv).r;
    if(i == 1) return texture(spotDepthMaps[1], uv).r;
    if(i == 2) return texture(spotDepthMaps[2], uv).r;
    return texture(spotDepthMaps[3], uv).r;
}

vec2 spotTexelSize(int i){
    if(i == 0) return 1.0 / vec2(textureSize(spotDepthMaps[0], 0));
    if(i == 1) return 1.0 / vec2(textureSize(spotDepthMaps[1], 0));
    if(i == 2) return 1.0 / vec2(textureSize(spotDepthMaps[2], 0));
    return 1.0 / vec2(textureSize(spotDepthMaps[3], 0));
}

// ---------------------------------------------------------------- shadows
float DirShadowCalculation(vec3 fragPos, vec3 norm){
    vec4 lp = dirLightSpaceMatrix * vec4(fragPos, 1.0);
    vec3 proj = lp.xyz / lp.w * 0.5 + 0.5;
    if(proj.z > 1.0)
        return 0.0;

    float bias = max(0.005 * (1.0 - dot(norm, normalize(-dirLight.direction))), 0.001);
    float shadow = 0.0;
    vec2 texel = 1.0 / vec2(textureSize(dirDepthMap, 0));
    for(int x = -1; x <= 1; ++x)
        for(int y = -1; y <= 1; ++y)
            if(proj.z - bias > texture(dirDepthMap, proj.xy + vec2(x, y) * texel).r)
                shadow += 1.0;
    return shadow / 9.0;
}

float PointShadowCalculation(int i, vec3 fragPos){
    vec3 fragToLight = fragPos - pointLights[i].position;
    float currentDepth = length(fragToLight);

    float shadow = 0.0;
    float bias = 0.05;
    float diskRadius = 0.02 + currentDepth * 0.004;
    vec3 offsets[8] = vec3[](
        vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1),
        vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1)
    );
    for(int s = 0; s < 8; s++){
        float closest = samplePointDepth(i, fragToLight + offsets[s] * diskRadius) * far_plane;
        if(currentDepth - bias > closest)
            shadow += 1.0;
    }
    return shadow / 8.0;
}

float SpotShadowCalculation(int i, vec3 fragPos, vec3 norm){
    vec4 lp = spotLightSpaceMatrix[i] * vec4(fragPos, 1.0);
    vec3 proj = lp.xyz / lp.w * 0.5 + 0.5;
    if(proj.z > 1.0)
        return 0.0;

    float bias = max(0.0008 * (1.0 - dot(norm, normalize(-spotLights[i].direction))), 0.0002);
    float shadow = 0.0;
    vec2 texel = spotTexelSize(i);
    for(int x = -1; x <= 1; ++x)
        for(int y = -1; y <= 1; ++y)
            if(proj.z - bias > sampleSpotDepth(i, proj.xy + vec2(x, y) * texel))
                shadow += 1.0;
    return shadow / 9.0;
}

// ---------------------------------------------------------------- lights
vec3 DirLightCalculation(vec3 norm, vec3 viewDir, vec3 diffuseColor, vec3 specularColor){
    vec3 lightDir = normalize(-dirLight.direction);
    vec3 ambient = dirLight.ambient * diffuseColor;
    vec3 diffuse = dirLight.diffuse * max(dot(norm, lightDir), 0.0) * diffuseColor;
    vec3 halfwayDir = normalize(lightDir + viewDir);
    vec3 specular = dirLight.specular * pow(max(dot(norm, halfwayDir), 0.0), material.shininess) * specularColor;
    float shadow = shadows ? DirShadowCalculation(fs_in.FragPos, norm) : 0.0;
    return ambient + (1.0 - shadow) * (diffuse + specular);
}

vec3 PointLightCalc(int i, vec3 norm, vec3 viewDir, vec3 diffuseColor, vec3 specularColor){
    vec3 toLight = pointLights[i].position - fs_in.FragPos;
    float dist = length(toLight);
    vec3 lightDir = toLight / dist;

    vec3 ambient = pointLights[i].ambient * diffuseColor;
    vec3 diffuse = pointLights[i].diffuse * max(dot(lightDir, norm), 0.0) * diffuseColor;
    vec3 halfwayDir = normalize(lightDir + viewDir);
    vec3 specular = pointLights[i].specular * pow(max(dot(norm, halfwayDir), 0.0), material.shininess) * specularColor;

    float attenuation = 1.0 / (pointLights[i].k0 + pointLights[i].k1 * dist + pointLights[i].k2 * dist * dist);

    float shadow = 0.0;
    if(shadows && pointCastsShadow[i])
        shadow = PointShadowCalculation(i, fs_in.FragPos);

    return attenuation * (ambient + (1.0 - shadow) * (diffuse + specular));
}

vec3 SpotLightCalculation(int i, vec3 norm, vec3 viewDir, vec3 diffuseColor, vec3 specularColor){
    vec3 toLight = spotLights[i].position - fs_in.FragPos;
    float dist = length(toLight);
    vec3 lightDir = toLight / dist;
    float theta = dot(lightDir, normalize(-spotLights[i].direction));

    vec3 ambient = spotLights[i].ambient * diffuseColor;
    if(theta <= spotLights[i].outerCutOff)
        return ambient;

    vec3 diffuse = spotLights[i].diffuse * max(dot(norm, lightDir), 0.0) * diffuseColor;
    vec3 halfwayDir = normalize(lightDir + viewDir);
    vec3 specular = spotLights[i].specular * pow(max(dot(norm, halfwayDir), 0.0), material.shininess) * specularColor;

    float intensity = clamp((theta - spotLights[i].outerCutOff) / (spotLights[i].cutOff - spotLights[i].outerCutOff), 0.0, 1.0);
    float attenuation = 1.0 / (spotLights[i].k0 + spotLights[i].k1 * dist + spotLights[i].k2 * dist * dist);

    float shadow = shadows ? SpotShadowCalculation(i, fs_in.FragPos, norm) : 0.0;
    return ambient + (1.0 - shadow) * (diffuse + specular) * intensity * attenuation;
}

// ---------------------------------------------------------------- main
void main(){
    if(unlit){
        FragColor = vec4(unlitColor, 1.0);
        return;
    }

    vec3 norm = normalize(fs_in.Normal);

    vec3 diffuseColor;
    vec3 specularColor;
    if(noTex){
        diffuseColor = material.diffuseColor;
        specularColor = material.specularColor;
    }
    else{
        vec2 uv = fs_in.TexCoords;
        if(worldUV){
            // box projection: pick the plane the surface faces
            vec3 an = abs(norm);
            if(an.x >= an.y && an.x >= an.z)      uv = fs_in.FragPos.zy;
            else if(an.y >= an.z)                 uv = fs_in.FragPos.xz;
            else                                  uv = fs_in.FragPos.xy;
        }
        uv *= uvScale;
        diffuseColor = texture(material.diffuseTexture, uv).rgb * material.diffuseColor;
        specularColor = texture(material.specularTexture, uv).rgb * material.specularColor;
    }

    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 result = vec3(0.0);

    if(hasDirLight)
        result += DirLightCalculation(norm, viewDir, diffuseColor, specularColor);

    for(int i = 0; i < noPointLights; i++)
        result += PointLightCalc(i, norm, viewDir, diffuseColor, specularColor);

    for(int i = 0; i < noSpotLights; i++)
        result += SpotLightCalculation(i, norm, viewDir, diffuseColor, specularColor);

    result += emissive;

    // aim highlight: warm rim + lift so the target reads clearly
    if(highlight > 0.0){
        float rim = pow(1.0 - max(dot(norm, viewDir), 0.0), 2.0);
        result += highlight * (vec3(0.30, 0.24, 0.10) * rim + diffuseColor * 0.25);
    }

    FragColor = vec4(result, 1.0);
}
