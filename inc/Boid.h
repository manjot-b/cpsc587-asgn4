#pragma once

#include <glm/glm.hpp>

struct Boid
{
    float mass;
    float weight;
    glm::vec3 velocity;
    glm::vec3 position;
    glm::vec3 netForce;
};