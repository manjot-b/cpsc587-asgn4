#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>
#include <algorithm>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cmath>
#include <cstdlib>
#include <ctime>

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
	parseInputFile();
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

	int componentsPerAttrib[] = {3, 3};	
	vertexArray = make_shared<VertexArray>(
		componentsPerAttrib, 2, buffer.data(), buffer.size(), GL_STATIC_DRAW);

	glm::mat4 view = glm::lookAt(
			glm::vec3(0, 0.2f, 0.6f),	// camera position
			glm::vec3(0.0f, 0.f, 0),		// where camera is lookin
			glm::vec3(0, 1, 0)				// up vector
	);
	glm::mat4 perspective =  glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);
	camera = Camera(view, perspective);

	shader->use();
	shader->setUniformMatrix4fv("projectionView", camera.getProjectionViewMatrix());
	shader->unuse();

	modelScale = 1 / max(fishMesh->bbox.width, max(fishMesh->bbox.height, fishMesh->bbox.depth)) * modelScale;
	// scale = 0.1;
	// float xTrans = -fishMesh->bbox.x - (fishMesh->bbox.width / 2);
	// float yTrans = -fishMesh->bbox.y - (fishMesh->bbox.height / 2);
	// float zTrans = -fishMesh->bbox.z + (fishMesh->bbox.depth / 2);
	// glm::mat4 model(1.0f);
	// model = glm::scale(model, glm::vec3(scale, scale, scale));
	// model = glm::translate(model, glm::vec3(xTrans, yTrans, zTrans));
	// modelMatrices.push_back(model);
	// model = glm::translate(model, glm::vec3(xTrans + 2, 0, -2));	
	// modelMatrices.push_back(model);

	struct Boid boid;
	boid.mass = 1;
	boid.weight = 1;
	boid.velocity = glm::vec3(1, 0, -0.1) * 0.05f;
	boid.netForce = glm::vec3(0, 0, 0);
	boid.position = glm::vec3(-0.1, 0, 0);
	boids.push_back(boid);
	boid.velocity = glm::vec3(0.1, 1, 0) * 0.05f;
	boids.push_back(boid);
	boid.velocity = glm::vec3(0.1, 0, 1) * 0.05f;
	boids.push_back(boid);

	srand(time(NULL));
	for (uint i = 0; i < bodyCount; i++)
	{
		int x = rand();
		int y = rand();
		int z = rand();

		x %= 100000;
		float randX = ((float)x) / 100000;
		y %= 100000;
		float randY = ((float)y) / 100000;
		z %= 100000;
		float randZ = ((float)x) / 100000;

		randX = cage.dimension.x * randX + cage.origin.x;
		randY = cage.dimension.y * randY + cage.origin.y;
		randZ = cage.origin.z - cage.dimension.z * randZ;

		boid.position = glm::vec3(randX, randY, randZ);
		boid.velocity = glm::vec3(rand(), rand(), rand());
		boid.velocity = glm::normalize(boid.velocity) * maxSpeed;
		boids.push_back(boid);
	}

	modelMatrices = vector<glm::mat4>(boids.size(), glm::mat4(1.0f));


	glGenBuffers(1, &instanceVBO);
	glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
	glBufferData(GL_ARRAY_BUFFER, modelMatrices.size() * sizeof(glm::mat4), &modelMatrices[0], GL_DYNAMIC_DRAW);
  
    // unsigned int VAO = rock.meshes[i].VAO;
    // glBindVertexArray(VAO);
	vertexArray->use();
    // vertex Attributes
    size_t vec4Size = sizeof(glm::vec4);
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

	// BOUNDING BOX
	simpleShader = make_shared<Shader>("rsc/simple_vertex.glsl", "rsc/simple_fragment.glsl");
	simpleShader->link();

	simpleShader->use();
	simpleShader->setUniformMatrix4fv("projectionView", camera.getProjectionViewMatrix());
	simpleShader->unuse();

	buffer.clear();
	buffer = {
		// front
		cage.origin.x, cage.origin.y + cage.dimension.y, cage.origin.z,
		cage.origin.x, cage.origin.y, cage.origin.z,
		cage.origin.x + cage.dimension.x, cage.origin.y, cage.origin.z,
		cage.origin.x + cage.dimension.x, cage.origin.y + cage.dimension.y, cage.origin.z,
		
		// back
		cage.origin.x, cage.origin.y + cage.dimension.y, cage.origin.z - cage.dimension.z,
		cage.origin.x, cage.origin.y, cage.origin.z - cage.dimension.z,
		cage.origin.x + cage.dimension.x, cage.origin.y, cage.origin.z - cage.dimension.z,
		cage.origin.x + cage.dimension.x, cage.origin.y + cage.dimension.y, cage.origin.z - cage.dimension.z,	
	};
	uint indices[] = {
		0, 1,
		1, 2,
		2, 3,
		3, 0,
		
		4, 5,
		5, 6,
		6, 7,
		7, 4,

		0, 4,
		1, 5,
		2, 6,
		3, 7
	};
	boxVertexArray = std::make_shared<VertexArray>(componentsPerAttrib, 1, buffer.data(), buffer.size());
	boxVertexArray->setElementBuffer(indices, 24);

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
	for (uint j = 0; j < updatesPerFrame; j++)
	{
		for (uint i = 0; i < boids.size(); i++)
		{
			checkCollisions(boids[i], i);
			
			boids[i].velocity += boids[i].netForce * boids[i].weight * deltaT;
			if ( glm::length(boids[i].velocity) > maxSpeed)
			{
				boids[i].velocity = glm::normalize(boids[i].velocity);
				boids[i].velocity *= maxSpeed;
			}

			boids[i].position += boids[i].velocity * deltaT;
			
			boids[i].netForce = glm::vec3(0, 0, 0);
		}
	}

	for (uint i = 0; i < boids.size(); i++)
	{

		glm::vec3 normal = boids[i].netForce - gravityForce;
		normal = normalize(normal);

		glm::vec3 tangent = boids[i].velocity;
		tangent = normalize(tangent);

		// take care of case where tangent and normal are parallel
		float dot = glm::dot(normal, tangent);
		uint k = 0;
		while (dot == 1 || dot == -1)
		{
			tangent[k%3] += EPSILON;
			tangent = normalize(tangent);
			dot = glm::dot(normal, tangent);
			k++;
		}

		glm::vec3 binormal = glm::cross(tangent, normal);

		binormal = glm::normalize(binormal);
		normal = glm::cross(binormal, tangent);
		normal = glm::normalize(normal);

		glm::mat4 model(1.0);
		model[0] = glm::vec4(binormal, 0) * modelScale;
		model[1] = glm::vec4(normal, 0) * modelScale;
		model[2] = glm::vec4(tangent, 0) * modelScale;
		model[3] = glm::vec4(boids[i].position, 1.0f);
		
		modelMatrices[i] = model;
	}
	
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
	glDrawArraysInstanced(GL_TRIANGLES, 0, fishMesh->verticesCount(), boids.size()); 
	vertexArray->unuse();
	shader->unuse();

	simpleShader->use();
	boxVertexArray->use();
	glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
	boxVertexArray->unuse();
	simpleShader->unuse();

	glfwSwapBuffers(window_.get());
	glfwPollEvents();
}

void Engine::checkCollisions(Boid& boid, uint boidIdx)
{
	// CAGE COLLISION
	// float constant = 0.01;
	
	if (boid.position.x > cage.origin.x + cage.dimension.x)
	{
		// float cageForce = constant / glm::distance(boid.position.x, cage.origin.x + cage.dimension.x);
		boid.position.x =  cage.origin.x + cage.dimension.x - EPSILON;
		// boid.netForce += glm::vec3(-cageForce, 0, 0);
		
		glm::vec3 incident = boid.velocity;
		incident = glm::normalize(incident);
		boid.velocity = glm::reflect(incident, glm::vec3(-1, 0, 0)) * glm::length(boid.velocity);
	}

	if (boid.position.x < cage.origin.x)
	{
		// float cageForce = constant / glm::distance(boid.position.x, cage.origin.x);
		boid.position.x = cage.origin.x + EPSILON;
		// boid.netForce += glm::vec3(cageForce, 0, 0);
		glm::vec3 incident = boid.velocity;
		incident = glm::normalize(incident);
		boid.velocity = glm::reflect(incident, glm::vec3(1, 0, 0)) * glm::length(boid.velocity);
	}

	if (boid.position.y > cage.origin.y + cage.dimension.y)
	{
		// float cageForce = constant / glm::distance(boid.position.y, cage.origin.y + cage.dimension.y);
		boid.position.y = cage.origin.y + cage.dimension.y - EPSILON;
		// boid.netForce += glm::vec3(0, -cageForce, 0);
		glm::vec3 incident = boid.velocity;
		incident = glm::normalize(incident);
		boid.velocity = glm::reflect(incident, glm::vec3(0, -1, 0)) * glm::length(boid.velocity);
	}

	if (boid.position.y < cage.origin.y)
	{
		// float cageForce = constant / glm::distance(boid.position.y, cage.origin.y);
		boid.position.y = cage.origin.y + EPSILON;
		// boid.netForce += glm::vec3(0, cageForce, 0);
		glm::vec3 incident = boid.velocity;
		incident = glm::normalize(incident);
		boid.velocity = glm::reflect(incident, glm::vec3(0, 1, 0)) * glm::length(boid.velocity);
	}

	if (boid.position.z > cage.origin.z)
	{
		// float cageForce = constant / glm::distance(boid.position.z, cage.origin.z);
		boid.position.z = cage.origin.z - EPSILON;
		// boid.netForce += glm::vec3(0, 0, -cageForce);
		glm::vec3 incident = boid.velocity;
		incident = glm::normalize(incident);
		boid.velocity = glm::reflect(incident, glm::vec3(0, 0, -1)) * glm::length(boid.velocity);
	}

	if (boid.position.z < cage.origin.z - cage.dimension.z)
	{
		// float cageForce = constant / glm::distance(boid.position.z, cage.origin.z - cage.dimension.z);
		boid.position.z = cage.origin.z - cage.dimension.z + EPSILON;
		// boid.netForce += glm::vec3(0, 0, cageForce);
		glm::vec3 incident = boid.velocity;
		incident = glm::normalize(incident);
		boid.velocity = glm::reflect(incident, glm::vec3(0, 0, 1)) * glm::length(boid.velocity);
	}

	// BOID COLLISION
	uint count = 0;
	for (uint i = boidIdx + 1; i < boids.size(); i++)
	{
		float dist = glm::distance(boid.position, boids[i].position);
		if (dist <= 0)
			return;

		if (dist < avoidance)
		{
			glm::vec3 direction = boid.position - boids[i].position;
			direction = normalize(direction);
			float forceMagnitude = 0.1 / (dist);
			glm::vec3 force = direction * forceMagnitude;
			boid.netForce += force;
			boids[i].netForce -= force;
			count++;
		}
		else if (dist < cohesion)
		{
			float forceMagnitude = 0.1 / dist;
			glm::vec3 force = ( (boid.velocity + boids[i].velocity) / 2.0f) * forceMagnitude;
			boid.netForce += force;
			boids[i].netForce += force;
			count++;
		}
		else if (dist < gather)
		{
			glm::vec3 direction = boid.position - boids[i].position;
			direction = normalize(direction);
			float forceMagnitude = 0.1 / (dist);
			glm::vec3 force = direction * forceMagnitude;
			boid.netForce -= force;
			boids[i].netForce += force;
			count++;
		}

		if (count == neighbours) break;	// boids only aware of a limited ammount of other boids
	}
}

void Engine::parseInputFile()
{
	char delim = ' ';
	ifstream inFile ("rsc/parameters.txt");
	string line;
	string tokens[2];
	string token;

	if (inFile.is_open())
	{
		while ( getline(inFile, line) )
		{
			int i = 0;
			stringstream ss(line);
			while ( getline(ss, token, delim))
			{
				tokens[i++] = token;
			}

			i = 0;

			if ( tokens[0] == "avoidance" )
				avoidance = stof( tokens[1] );
			else if ( tokens[0] == "cohesion" )
				cohesion = stof( tokens[1] );
			else if ( tokens[0] == "gather" )
				gather = stof( tokens[1] );
			else if ( tokens[0] == "max_speed" )
				maxSpeed = stof( tokens[1] );
			else if ( tokens[0] == "body_count" )
				bodyCount = stof( tokens[1] );
			else if ( tokens[0] == "neighbours" )
				neighbours = stof( tokens[1] );
			else if ( tokens[0] == "time_step" )
				deltaT = stof( tokens[1] );
			else if ( tokens[0] == "mesh_scale" )
				modelScale = stof( tokens[1] );
			else
				cerr << "Error. \'" << tokens[0]  << "\' is not a valid parameter in \'rsc\\parameters.txt\'" << endl;
		}
	}
	else 
	{
		cerr << "Error. Unable to open \'rsc\\parameters.txt\'" << endl;
	}
}
