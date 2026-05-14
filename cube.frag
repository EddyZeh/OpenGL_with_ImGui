#version 330 core

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;
out vec4 FragColor;

struct Material{
	sampler2D diffuse;
	sampler2D specular;
	float shininess;
};
uniform Material material;

struct Light{
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

uniform Light light;


uniform vec3 lightPos;
uniform vec3 viewPos;

vec3 materialLighting(Light light, vec3 norm, Material material); 

void main(){
	vec3 norm = normalize(Normal);
	vec3 l1 = materialLighting(light, norm, material);
	FragColor =vec4(l1, 1.0f);
}

vec3 materialLighting(Light light, vec3 norm, Material material){
	// ambient 
	vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords)); 

	// diffuse
	vec3 lightDir = normalize(lightPos - FragPos);
	float diff = max(dot(norm,lightDir), 0.0);
	vec3 diffuse = light.diffuse * (diff * vec3(texture(material.diffuse, TexCoords)));

	// specular
	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	vec3 specular = light.specular * (spec * vec3(texture(material.specular, TexCoords)));

	vec3 result = ambient + diffuse + specular;
	return result;
}
