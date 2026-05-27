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
} vs_out;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform bool reverse_normals;

void main(){
    mat4 instancedModel = model;
    instancedModel[0][0] *= aSize.x;
    instancedModel[1][1] *= aSize.y;
    instancedModel[2][2] *= aSize.z;

    instancedModel[3] += vec4(aOffset, 0.0);

    //vec3 pos = aPos * aSize;
    //vs_out.FragPos = vec3(model * vec4(pos + aOffset, 1.0));
    vs_out.FragPos = vec3(instancedModel * vec4(aPos, 1.0));
    if(reverse_normals)
        vs_out.Normal = transpose(inverse(mat3(model))) * (-1.0 * aNormal);
    else
        vs_out.Normal = transpose(inverse(mat3(model))) * aNormal;
    vs_out.TexCoords = aTexCoords;
    gl_Position = projection * view * vec4(vs_out.FragPos, 1.0);
}