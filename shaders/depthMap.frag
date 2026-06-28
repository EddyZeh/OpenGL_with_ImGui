#version 330 core

in vec4 FragPos_out;

uniform vec3 lightPos;
uniform float far_plane;

void main(){
	// get distance between fragment and lightSource
	float lightDistance = length(FragPos_out.xyz - lightPos);
	
	// clamp between [0, 1] by dividing by far_plane
	lightDistance = lightDistance / far_plane;

	// modified depth
	gl_FragDepth = lightDistance;
}