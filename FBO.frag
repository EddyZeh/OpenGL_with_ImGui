#version 330 core

in vec2 texCoords;
out vec4 FragColor;

uniform sampler2D screenTexture;

void main(){
	FragColor = vec4(vec3(texture(screenTexture, texCoords)), 1.0);
}