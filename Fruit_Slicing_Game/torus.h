#pragma once

#ifndef TORUS_H
#define TORUS_H

#include <glad/glad.h>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shader.h"



class Torus {
public:
    // Material properties
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    // Texture properties
    float TXmin = 0.0f;
    float TXmax = 1.0f;
    float TYmin = 0.0f;
    float TYmax = 1.0f;
    unsigned int diffuseMap;
    unsigned int specularMap;

    // Common properties
    float shininess;

    // Constructor
    Torus()
    {
        setUpTorusVertexDataAndConfigureVertexAttribute();
    }

    Torus(glm::vec3 amb, glm::vec3 diff, glm::vec3 spec, float shiny)
    {
        this->ambient = amb;
        this->diffuse = diff;
        this->specular = spec;
        this->shininess = shiny;

        setUpTorusVertexDataAndConfigureVertexAttribute();
    }

    Torus(unsigned int dMap, unsigned int sMap, float shiny, float textureXmin, float textureYmin, float textureXmax, float textureYmax)
    {
        this->diffuseMap = dMap;
        this->specularMap = sMap;
        this->shininess = shiny;
        this->TXmin = textureXmin;
        this->TYmin = textureYmin;
        this->TXmax = textureXmax;
        this->TYmax = textureYmax;

        setUpTorusVertexDataAndConfigureVertexAttribute();
    }

    // Destructor
    ~Torus()
    {
        glDeleteVertexArrays(1, &torusVAO);
        glDeleteBuffers(1, &torusVBO);
        glDeleteBuffers(1, &torusEBO);
    }

    void drawTorusWithTexture(Shader& lightingShaderWithTexture, glm::mat4 model = glm::mat4(1.0f))
    {
        lightingShaderWithTexture.use();

        lightingShaderWithTexture.setInt("material.diffuse", 0);
        lightingShaderWithTexture.setInt("material.specular", 1);
        lightingShaderWithTexture.setFloat("material.shininess", this->shininess);

        // Bind diffuse map
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, this->diffuseMap);
        // Bind specular map
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, this->specularMap);

        lightingShaderWithTexture.setMat4("model", model);

        glBindVertexArray(torusVAO);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    }

    void drawTorusWithMaterialisticProperty(Shader& lightingShader, glm::mat4 model = glm::mat4(1.0f))
    {
        lightingShader.use();

        lightingShader.setVec3("material.ambient", this->ambient);
        lightingShader.setVec3("material.diffuse", this->diffuse);
        lightingShader.setVec3("material.specular", this->specular);
        lightingShader.setFloat("material.shininess", this->shininess);

        lightingShader.setMat4("model", model);

        glBindVertexArray(torusVAO);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    }

    void setMaterialisticProperty(glm::vec3 amb, glm::vec3 diff, glm::vec3 spec, float shiny)
    {
        this->ambient = amb;
        this->diffuse = diff;
        this->specular = spec;
        this->shininess = shiny;
    }

    void setTextureProperty(unsigned int dMap, unsigned int sMap, float shiny)
    {
        this->diffuseMap = dMap;
        this->specularMap = sMap;
        this->shininess = shiny;
    }

private:
    unsigned int torusVAO;
    unsigned int torusVBO;
    unsigned int torusEBO;
    int indexCount;

    void setUpTorusVertexDataAndConfigureVertexAttribute()
    {
        const int majorSegments = 24; // Number of segments along the major circle
        const int minorSegments = 12; // Number of segments along the minor circle
        const float majorRadius = 0.6f; // Radius of the major circle
        const float minorRadius = 0.25f; // Radius of the minor circle

        std::vector<float> vertices;
        std::vector<unsigned int> indices;

        // Generate vertices
        for (int i = 0; i <= majorSegments; ++i) {
            float majorAngle = 2.0f * 3.14 * i / majorSegments;
            float cosMajor = cos(majorAngle);
            float sinMajor = sin(majorAngle);

            for (int j = 0; j <= minorSegments; ++j) {
                float minorAngle = 2.0f * 3.14 * j / minorSegments;
                float cosMinor = cos(minorAngle);
                float sinMinor = sin(minorAngle);

                float x = (majorRadius + minorRadius * cosMinor) * cosMajor;
                float y = minorRadius * sinMinor;
                float z = (majorRadius + minorRadius * cosMinor) * sinMajor;

                float nx = cosMinor * cosMajor;
                float ny = sinMinor;
                float nz = cosMinor * sinMajor;

                float u = (float)i / majorSegments;
                float v = (float)j / minorSegments;

                vertices.insert(vertices.end(), { x, y, z, nx, ny, nz, u, v });
            }
        }

        // Generate indices
        for (int i = 0; i < majorSegments; ++i) {
            for (int j = 0; j < minorSegments; ++j) {
                unsigned int current = i * (minorSegments + 1) + j;
                unsigned int next = current + minorSegments + 1;

                indices.insert(indices.end(), { current, next, current + 1 });
                indices.insert(indices.end(), { current + 1, next, next + 1 });
            }
        }

        indexCount = indices.size();

        // Generate buffers
        glGenVertexArrays(1, &torusVAO);
        glGenBuffers(1, &torusVBO);
        glGenBuffers(1, &torusEBO);

        glBindVertexArray(torusVAO);

        glBindBuffer(GL_ARRAY_BUFFER, torusVBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, torusEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

        // Position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // Normal attribute
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        // Texture coordinate attribute
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);
    }
};

#endif /* TORUS_H */
