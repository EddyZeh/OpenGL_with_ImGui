#version 330 core

in vec2 texCoords;
out vec4 FragColor;

uniform sampler2D screenTexture;

void main(){
	vec3 color = texture(screenTexture, texCoords).rgb;
	float average = 0.2126 * color.x + 0.7152 * color.y + 0.0722 * color.z;
	FragColor = vec4(average, average, average, 1.0);
	//FragColor = vec4(color, 1.0);
}