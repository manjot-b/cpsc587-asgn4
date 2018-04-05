#version 410 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in mat4 model;


out vec4 vertexColor;
out vec3 surfaceNormal;
out vec3 toLight;

uniform mat4 projectionView;


void main()
{
	vec4 worldPos = model * vec4(aPos, 1.0);
	// gl_Position = projection * view * model * vec4(aPos, 1.0);
	gl_Position = projectionView * worldPos;
	vertexColor = vec4(aPos, 1.0);

	surfaceNormal = (model * vec4(aNormal, 1.0)).xyz;
	toLight = vec3(-10, -10, -1) - worldPos.xyz; 
}
