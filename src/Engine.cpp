#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <memory>
#include <map>
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>

#include "Engine.h"
#include "Shader.h"
#include "VertexArray.h"
#include "Spring.h"

using namespace std;

Engine::Engine(int argc, const char *argv[])
{
	if (!initWindow())
	{
		cerr << "Failed to  initilize GLFW" << endl;
		windowInitialized_ = false;
	}
	windowInitialized_ = true;
	initScene();
}

bool Engine::initWindow()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window_ = GLFWwindowPtr(glfwCreateWindow(800, 800, "Boids", NULL, NULL));

	if (!window_)
	{
		cout << "Failed to create GLFW window. TERMINATING" << endl;
		glfwTerminate();
		return false;
	}
	glfwMakeContextCurrent(window_.get());

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		cout << "Failed to initialize GLAD" << endl;
		return false;
	}

	glViewport(0, 0, 800, 800);
	glfwSetWindowUserPointer(window_.get(), this);

	glfwSetFramebufferSizeCallback(window_.get(),
								   [](GLFWwindow *window, int width, int height) {
									   glViewport(0, 0, width, height);
								   });
	glEnable(GL_DEPTH_TEST);

	return true; // if we made it here then success
}

void Engine::initScene()
{
	shader = make_shared<Shader>("rsc/vertex.glsl", "rsc/fragment.glsl");
	shader->link();

	fishMesh = make_shared<TriangleMesh>("rsc/pyrm.obj");
	// float tempData[] = {
	// 	-0.5f, -0.5f, 0,
	// 	0.5f, -0.5f, 0,
	// 	0, 0.5f, -0.3
	// };

	vector<float> buffer;
	for (uint i = 0; i < fishMesh->verticesCount(); i++)
	{
		uint idx = i*3;
		// vertices
		buffer.push_back( *(fishMesh->getData() + idx) );
		buffer.push_back( *(fishMesh->getData() + idx+1) );
		buffer.push_back( *(fishMesh->getData() + idx+2) );

		// normals
		buffer.push_back( *(fishMesh->getNormalData() + idx) );
		buffer.push_back( *(fishMesh->getNormalData() + idx+1) );
		buffer.push_back( *(fishMesh->getNormalData() + idx+2) );
	}

	for (uint i = 0; i < buffer.size() / 6; i++)
	{
		cout << "VERT" << *(fishMesh->getData() + i) << " "
			<< *(fishMesh->getData() + i+1) << " "
			<< *(fishMesh->getData() + i+2) << " " << endl
			<< *(fishMesh->getNormalData() + i+3) << " "
			<< *(fishMesh->getNormalData() + i+4) << " "
			<< *(fishMesh->getNormalData() + i+5) << " "
			<< endl;
	}
	cout << buffer.size() << endl;

	int componentsPerAttrib[] = {3, 3};	
	vertexArray = make_shared<VertexArray>(
		componentsPerAttrib, 2, buffer.data(), buffer.size(), GL_STATIC_DRAW);

	glm::mat4 view = glm::lookAt(
			glm::vec3(0, 0.2f, 0.3f),	// camera position
			glm::vec3(0.0f, 0.f, 0),		// where camera is lookin
			glm::vec3(0, 1, 0)				// up vector
	);
	glm::mat4 perspective =  glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);
	camera = Camera(view, perspective);

	shader->use();
	shader->setUniformMatrix4fv("projectionView", camera.getProjectionViewMatrix());
	shader->unuse();

	float scale = 1 / max(fishMesh->bbox.width, max(fishMesh->bbox.height, fishMesh->bbox.depth)) * 0.1;
	// scale = 0.1;
	float xTrans = -fishMesh->bbox.x - (fishMesh->bbox.width / 2);
	float yTrans = -fishMesh->bbox.y - (fishMesh->bbox.height / 2);
	float zTrans = -fishMesh->bbox.z + (fishMesh->bbox.depth / 2);
	glm::mat4 model(1.0f);
	model = glm::scale(model, glm::vec3(scale, scale, scale));
	model = glm::translate(model, glm::vec3(xTrans, yTrans, zTrans));
	modelMatrices.push_back(model);
	model = glm::translate(model, glm::vec3(xTrans + 2, 0, -2));	
	modelMatrices.push_back(model);

	glGenBuffers(1, &instanceVBO);
	glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
	glBufferData(GL_ARRAY_BUFFER, modelMatrices.size() * sizeof(glm::mat4), &modelMatrices[0], GL_DYNAMIC_DRAW);
  
    // unsigned int VAO = rock.meshes[i].VAO;
    // glBindVertexArray(VAO);
	vertexArray->use();
    // vertex Attributes
    GLsizei vec4Size = sizeof(glm::vec4);
    glEnableVertexAttribArray(2); 
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)0);
    glEnableVertexAttribArray(3); 
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(vec4Size));
    glEnableVertexAttribArray(4); 
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(2 * vec4Size));
    glEnableVertexAttribArray(5); 
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(3 * vec4Size));

    glVertexAttribDivisor(2, 1);
    glVertexAttribDivisor(3, 1);
    glVertexAttribDivisor(4, 1);
    glVertexAttribDivisor(5, 1);

	vertexArray->unuse();

}



int Engine::run()
{
	if (!windowInitialized_)
		return -1;

	while (!glfwWindowShouldClose(window_.get()))
	{
		processInput();
		update();
		render();
	}

	glfwTerminate();
	return 0;
}

void Engine::processInput()
{
	if (glfwGetKey(window_.get(), GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window_.get(), true);
	}
}

void Engine::update()
{
	modelMatrices[0] = glm::rotate(modelMatrices[0], 0.01f, glm::vec3(0, 1, 0));
	modelMatrices[1] = glm::rotate(modelMatrices[1], -0.01f, glm::vec3(0, 1, 0));
	
	vertexArray->use();
	glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, modelMatrices.size() * sizeof(glm::mat4), &modelMatrices[0]);
	vertexArray->unuse();
}

void Engine::render()
{
	glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	shader->use();

	vertexArray->use();

	// glDrawArrays(GL_TRIANGLES, 0, 3);
	glDrawArraysInstanced(GL_TRIANGLES, 0, fishMesh->verticesCount(), 2); 
	vertexArray->unuse();

	shader->unuse();
	glfwSwapBuffers(window_.get());
	glfwPollEvents();
}

