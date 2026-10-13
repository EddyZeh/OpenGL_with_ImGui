#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 view;
uniform mat4 projection;
uniform float gridHeight;

out vec3 worldPos;

void main(){
	worldPos = vec3(aPos.x, gridHeight, aPos.z);
	gl_Position = projection * view * vec4(worldPos, 1.0);
}
