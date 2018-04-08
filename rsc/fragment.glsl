#version 410 core

out vec4 fragColor;

in vec4 vertexColor;	// used for input from vertex shader
in vec3 surfaceNormal;
in vec3 toLight[2];

uniform vec4 uColor;			// used for input from program (CPU)

void main()
{
	vec3 lightColor[2];
	lightColor[0] = vec3(0, 0, 1);	
	lightColor[1] = vec3(1, 0, 0.4);	

	//AMBIENT
	vec3 ambient = vec3(0.35, 0.35, 0.35);

	//DIFFUSE
	vec3 diffuse = vec3(0, 0, 0);
	vec3 unitToLight;
	vec3 unitNormal = normalize(surfaceNormal);
	
	for (int i = 0; i < 2; i++)	// 2 light sources
	{
		unitToLight = normalize(toLight[i]);
		float diffBrightness = max ( dot(unitNormal, unitToLight), 0);
		diffuse = diffuse + (lightColor[i] * diffBrightness);
	}
	// fragColor = vertexColor;
	fragColor = vec4(ambient + diffuse, 1.0) * vec4(0.3, 0.1, 1, 1);
}