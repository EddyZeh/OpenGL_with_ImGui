#version 330 core
out vec4 FragColor;

in vec3 worldPos;

uniform vec3 viewPos;

// the spotlight: same quantities the surface shader gets
uniform vec3  lightPos;
uniform vec3  lightDir;			
uniform float cutOff;			
uniform float outerCutOff;		
uniform float k0;
uniform float k1;
uniform float k2;
uniform vec3  lightColor;

// the torch's shadow map
uniform sampler2D shadowMap;
uniform mat4  lightSpaceMatrix;
uniform float shadowBias;
uniform bool  useShadows;

// look of the beam
uniform float maxChord;			
uniform float density;			
uniform float anisotropy;		
uniform float intensity;
uniform int   steps;

const float PI = 3.14159265359;

float interleavedGradientNoise(vec2 p){
	return fract(52.9829189 * fract(dot(p, vec2(0.06711056, 0.00583715))));
}

float phase(float cosTheta, float g){
	float g2 = g * g;
	float denom = 1.0 + g2 - 2.0 * g * cosTheta;
	return (1.0 - g2) / pow(max(denom, 1e-4), 1.5);
}

float shadowVisibility(vec3 p){
	if (!useShadows)
		return 1.0;

	vec4 lightClip = lightSpaceMatrix * vec4(p, 1.0);
	if (lightClip.w <= 0.0)
		return 1.0;

	vec3 proj = lightClip.xyz / lightClip.w * 0.5 + 0.5;

	if (proj.z < 0.0 || proj.z > 1.0 ||
		proj.x < 0.0 || proj.x > 1.0 ||
		proj.y < 0.0 || proj.y > 1.0)
		return 1.0;

	float closest = texture(shadowMap, proj.xy).r;
	return (proj.z - shadowBias > closest) ? 0.0 : 1.0;
}

// light scattered towards the eye from point p, per unit length of the ray
float inScatter(vec3 p, vec3 rayDir){
	vec3 fromLight = p - lightPos;
	float dist = length(fromLight);
	if (dist < 1e-4)
		return 0.0;

	// direction the light is travelling when it reaches p
	vec3 travel = fromLight / dist;

	float theta = dot(travel, lightDir);
	float spot = clamp((theta - outerCutOff) / (cutOff - outerCutOff), 0.0, 1.0);
	if (spot <= 0.0)
		return 0.0;

	float attenuation = 1.0 / (k0 + k1 * dist + k2 * dist * dist);

	// angle between the light's travel direction and the direction towards the eye
	float scatter = phase(dot(travel, -rayDir), anisotropy);

	return spot * attenuation * scatter * shadowVisibility(p) * density;
}

void main(){
	vec3 toFrag = worldPos - viewPos;
	float rayEnd = length(toFrag);
	vec3 rayDir = toFrag / rayEnd;

	// no line inside the cone is longer than maxChord, so never start earlier than that
	float rayStart = max(0.0, rayEnd - maxChord);

	float stepLen = (rayEnd - rayStart) / float(steps);
	float t = rayStart + stepLen * interleavedGradientNoise(gl_FragCoord.xy);

	float light = 0.0;
	for (int i = 0; i < steps; i++){
		light += inScatter(viewPos + rayDir * t, rayDir) * stepLen;
		t += stepLen;
	}

	// additive blending, so black adds nothing
	FragColor = vec4(lightColor * light * intensity, 1.0);
}
