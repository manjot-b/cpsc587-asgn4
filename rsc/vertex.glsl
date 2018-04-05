#version 410 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in mat4 model;


out vec4 vertexColor;
out vec4 surfaceNormal;

uniform mat4 projectionView;


void main()
{
	// gl_Position = projection * view * model * vec4(aPos, 1.0);
	gl_Position = projectionView * model * vec4(aPos, 1.0);
	vertexColor = vec4(aPos, 1.0);

	surfaceNormal = model * vec4(aNormal, 1.0);
}
