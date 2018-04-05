#version 410 core

out vec4 fragColor;

in vec4 vertexColor;	// used for input from vertex shader
in vec3 surfaceNormal;
in vec3 toLight;

uniform vec4 uColor;			// used for input from program (CPU)
vec3 lightColor = vec3(1, 1, 1);

void main()
{
	//AMBIENT
	vec3 ambient = 0.1*lightColor;

	//DIFFUSE
	vec3 unitNormal = normalize(surfaceNormal);
	vec3 unitToLight = normalize(toLight);
	float diffBrightness = max ( dot(unitNormal, unitToLight), 0);
	vec3 diffuse = diffBrightness * lightColor * 0.5;

	// fragColor = vertexColor;
	fragColor = vec4(ambient + diffuse, 1.0) * vertexColor;
}