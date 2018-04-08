#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <memory>
#include <vector>

#include "Spring.h"
#include "Shader.h"
#include "VertexArray.h"
#include "Camera.h"
#include "TriangleMesh.h"
#include "Boid.h"

struct Cage 
{
    glm::vec3 origin = glm::vec3(-0.2f, -0.1f, 0.1f);
    glm::vec3 dimension = glm::vec3(0.4f, 0.27f, 0.4f);
};

class Engine
{
    public:
        Engine(int argc, const char* argv[]);
        int run();

    private:
        struct DestroyglfwWin
        {
            void operator()(GLFWwindow* ptr)
            {
                glfwDestroyWindow(ptr);
            }
        };

        typedef std::unique_ptr<GLFWwindow, DestroyglfwWin> GLFWwindowPtr;
        GLFWwindowPtr window_;
        bool windowInitialized_;

        std::shared_ptr<Shader> shader;
        std::shared_ptr<Shader> simpleShader;
        std::shared_ptr<VertexArray> vertexArray;
        std::shared_ptr<VertexArray> boxVertexArray;
        Camera camera;
        std::shared_ptr<TriangleMesh> fishMesh;
        std::vector<glm::mat4> modelMatrices;
        GLuint instanceVBO;
        
        std::vector<Boid> boids;
        uint boidCount = 2000;
        float modelScale;
        struct Cage cage;
        float avoidance = 0.065;
        float cohesion = 0.12;
        float gather = 0.15;
        float maxSpeed = 0.15;

        const float EPSILON = 1E-5;
        float deltaT = 0.008;                               // in miliseconds
        uint updatesPerFrame = (1.0f / 60) / deltaT;
        glm::vec3 gravityForce = glm::vec3(0, -9.81f, 0);

        bool initWindow();
        void initScene();
        void processInput();
        void update();
        void render();       
        void checkCollisions(Boid& boid, uint boidIdx); 

};