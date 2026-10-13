#version 330 core
out vec4 FragColor;

in vec3 worldPos;

uniform vec3  baseColor;
uniform vec3  lineColor;
uniform vec3  xAxisColor;
uniform vec3  zAxisColor;

uniform vec3  viewPos;
uniform float cellSize;
uniform float majorEvery;
uniform float fadeDistance;
uniform float holeHalfExtent;

float gridCoverage(vec2 p, float spacing, float widthInPixels){
	vec2 coord = p / spacing;
	vec2 distanceToLine = abs(fract(coord - 0.5) - 0.5) / fwidth(coord);
	float nearest = min(distanceToLine.x, distanceToLine.y);
	return 1.0 - min(nearest / widthInPixels, 1.0);
}

void main(){
	if (holeHalfExtent > 0.0 &&
		abs(worldPos.x) < holeHalfExtent && abs(worldPos.z) < holeHalfExtent)
		discard;

	vec2 p = worldPos.xz;

	float minor = gridCoverage(p, cellSize, 1.0);
	float major = gridCoverage(p, cellSize * majorEvery, 1.5);

	vec3 color = baseColor;
	color = mix(color, lineColor, minor * 0.55);
	color = mix(color, lineColor, major);

	float xAxis = 1.0 - min(abs(p.y) / (fwidth(p.y) * 1.5), 1.0);
	float zAxis = 1.0 - min(abs(p.x) / (fwidth(p.x) * 1.5), 1.0);
	color = mix(color, xAxisColor, xAxis);
	color = mix(color, zAxisColor, zAxis);

	float fade = 1.0 - clamp(length(worldPos - viewPos) / fadeDistance, 0.0, 1.0);

	FragColor = vec4(color, fade);
}
