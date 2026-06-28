#version 330 core

struct Material{
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shininess;
};

uniform Material material;

out vec4 FragColor;

void main(){

	vec3 result = (material.ambient + material.diffuse + material.specular);

	FragColor =vec4(vec3(1.0), 1.0f);
}