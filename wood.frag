#version 330 core

in VS_OUT{
	vec2 TexCoords;
	vec3 Normal;
	vec3 FragPos;

} fs_in;

out vec4 FragColor;

uniform sampler2D woodTex;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform bool blinn;

void main(){
	vec3 color = vec3(texture(woodTex, fs_in.TexCoords));
	// ambient
	vec3 ambient = 0.05 * color;

	// diffuse
	vec3 lightDir = normalize(lightPos - fs_in.FragPos);
	vec3 norm = normalize(fs_in.Normal);
	float diff = max(dot(lightDir, norm), 0.0);
	vec3 diffuse = color * diff;

	// specular
	vec3 viewDir = normalize(viewPos - fs_in.FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = 0.0;
	if(blinn){
		vec3 halfwayDir = normalize(lightDir + viewDir);
		spec = pow(max(dot(norm, halfwayDir), 0.0), 32.0);
	}
	else{
		vec3 reflecDir = reflect(-lightDir, norm);
		spec = pow(max(dot(viewDir, reflectDir), 0.0), 8.0);
	}
	vec3 specular = color * spec;

	FragColor = vec4((ambient + diffuse + specular), 1.0);
}