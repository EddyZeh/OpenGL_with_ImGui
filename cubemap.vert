#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 3) in vec3 aOffset;
layout (location = 4) in vec3 aSize;

uniform mat4 model;
uniform bool instanced;

out vec4 FragPos;

void main(){
	
	mat4 instancedModel = model;
	instancedModel[0][0] *= aSize.x;
	instancedModel[1][1] *= aSize.y;
	instancedModel[2][2] *= aSize.z;

	instancedModel[3] += vec4(aOffset, 0.0);

	if(instanced)
		FragPos = instancedModel * vec4(aPos, 1.0);
	else
		FragPos = model * vec4(aPos, 1.0);
}