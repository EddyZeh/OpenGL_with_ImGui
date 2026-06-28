#version 330 core

in vec2 texCoords;
out vec4 FragColor;

uniform sampler2D screenTexture;
uniform float screenHeight;
uniform float screenWidth;
uniform int effect;

float offsetX = 1.0 / screenWidth;
float offsetY = 1.0 / screenHeight;

vec2 offsets[9] = vec2[](
		vec2(-offsetX, offsetY),  // top -left
		vec2(0.0f,     offsetY),  // top
		vec2(offsetX,  offsetY),  // top-right
         	
		vec2(-offsetX, 0.0f), 	  // left
		vec2(0.0f,     0.0f),     // center
		vec2(offsetX,  0.0f),     // right
		
		vec2(-offsetX, -offsetY), // bottom-left
		vec2(0.0f,     -offsetY), // bottom
		vec2(offsetX,  -offsetY)  // bottom-right
	);

vec3 SampleKernel(float kernel[9]);

void main(){

	vec3 color = texture(screenTexture, texCoords).rgb;

	// Normal
	switch(effect){
		// NORMAL
		case 0:
		{
			FragColor = vec4(color, 1.0);
			break;
		}
		// INVERSION
		case 1:
		{
			FragColor = vec4(1.0 - color, 1.0);
			break;
		}
		// GREYSCALE
		case 2:
		{
			float average = 0.2126 * color.x + 0.7152 * color.y + 0.0722 * color.z;
			FragColor = vec4(average, average, average, 1.0);
			break;
		}
		// SHARPENING

		case 3:
		{
			float kernel[9] = float[](
				-1, -1, -1,
				-1,  9, -1,
 				-1, -1, -1
			);
			vec3 result = SampleKernel(kernel);
			FragColor = vec4(result, 1.0);
			break;
		}
		// Edge-detection
		case 4:
		{
			float kernel[9] = float[](
				-1, -1, -1,
				-1,  8, -1,
 				-1, -1, -1
			);
			vec3 result = SampleKernel(kernel);
			FragColor = vec4(result, 1.0);
			break;
		}

		// SOBEL EDGE DETECTION
		case 5:
		{
			float horizontal[9] = float[](
				-1, 0, 1,
				-2, 0, 2,
			-1, 0, 1
			);

			float vertical[9] = float[](
				-1, 2, -1,
				0, 0,  0,
				1, 2,  1 
			);

			vec3 samples[9];
			for(int i = 0; i < 9; i++){
				samples[i] = vec3(texture(screenTexture, texCoords.st + offsets[i]));
			}

			vec3 gx = vec3(0.0);
			vec3 gy = vec3(0.0);

			for(int i = 0; i < 9; i++){
				gx += samples[i] * horizontal[i];
				gy += samples[i] * vertical[i];
			}

			vec3 edge = sqrt(gx * gx + gy * gy);
			FragColor = vec4(edge, 1.0);
			break;
		}

		// Emboss
		case 6:
		{
			float kernel[9] = float[](
				-2, 1, 0,
				-1, 1, 1,
 				 0, 1, 2
			);

			vec3 result = SampleKernel(kernel);
			FragColor = vec4(result, 1.0);
			break;
		}

		default:{
			FragColor = vec4(color, 1.0);
			break;
		} 

	}		
}

vec3 SampleKernel(float kernel[9]){	
	vec3 sampleTex[9];

	for(int i = 0; i < 9; i++){
		sampleTex[i] = vec3(texture(screenTexture, texCoords.st + offsets[i]));
	}

	vec3 col = vec3(0.0);
	for(int i = 0; i < 9; i++){
		col += sampleTex[i] * kernel[i];
	}

	return col;
}