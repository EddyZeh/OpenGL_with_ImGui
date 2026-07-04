#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aOffset;
layout (location = 4) in vec3 aSize;

out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
} vs_out;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

uniform mat4 lightSpaceMatrix;

uniform bool reverse_normals;
uniform bool instanced;


void main(){
    mat4 instancedModel = model;
    instancedModel[0][0] *= aSize.x;
    instancedModel[1][1] *= aSize.y;
    instancedModel[2][2] *= aSize.z;

    instancedModel[3] += vec4(aOffset, 0.0);

    if(reverse_normals){
        vs_out.Normal = transpose(inverse(mat3(model))) * (-1.0 * aNormal);
    }
    else{
        vs_out.Normal = transpose(inverse(mat3(model))) * aNormal;
    }
    vs_out.TexCoords = aTexCoords;
    
    vec3 worldPos;

    if(instanced){
        worldPos = vec3(instancedModel * vec4(aPos, 1.0));
        gl_Position = projection * view * vec4(worldPos, 1.0);
    }
    else{
        worldPos = vec3(model * vec4(aPos, 1.0));
        gl_Position = projection * view * model * vec4(aPos, 1.0);
    }

    vs_out.FragPos = worldPos;
    vs_out.FragPosLightSpace = lightSpaceMatrix * vec4(worldPos, 1.0);
}