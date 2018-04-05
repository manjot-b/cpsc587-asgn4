#pragma once

#include "Mesh.h"
#include <vector>
#include <glm/glm.hpp>

struct BoundingBox      
{
    float x = 0;
    float y = 0;
    float z = 0; 
    float width = 1.0f;
    float height = 1.0f;
    float depth = 1.0f;
};

class TriangleMesh : public Mesh
{
    public:
        TriangleMesh(const char* objFilePath);
        TriangleMesh(const float* data, unsigned int size);

        virtual void loadFromObjFile(const char* objFilePath);
        BoundingBox bbox;
    private:
        std::vector<float> normals_;
        void calcBoundingBox();
};