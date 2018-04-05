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
        std::shared_ptr<VertexArray> vertexArray;
        Camera camera;
        std::shared_ptr<TriangleMesh> fishMesh;
        std::vector<glm::mat4> modelMatrices;

        const float EPSILON = 1E-5;
        float deltaT = 0.0001;                               // in miliseconds
        uint updatesPerFrame = (1.0f / 60) / deltaT;
        glm::vec3 gravityForce = glm::vec3(0, -9.81f, 0);

        bool initWindow();
        void initScene();
        void processInput();
        void update();
        void render();        

};