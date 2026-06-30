#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 18) out;

in vec4 FragPos[];

uniform mat4 shadowMatrices[6];

out vec4 FragPos_out;

void main(){
	for (int face = 0; face < 6; ++face){
		gl_Layer = face;	// built-in variable that specifies which face to render
		for(int i = 0; i < 3; ++i){
			FragPos_out = FragPos[i];
			gl_Position = shadowMatrices[face] * FragPos[i];
			EmitVertex();
		}
		EndPrimitive();
	}
}