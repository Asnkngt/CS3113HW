#pragma once
#include "Headers.h"
#include "ShaderProgram.h"

enum EntityType {
    VOID,
    STATIC,
    PLAYER,
    TERRAIN,
    ITEM,
    TEXT,
    OTHER
};

class Entity
{
public:
    //ints to bitwise and to indicate collision on corners
    //const static int BL = 0b00000010, BR = 0b00000001, TL = 0b00001000, TR = 0b00000100;
    //masks to check collision
    //const static int BM = BL + BR, TM = TL + TR, LM = BL + TL, RM = BR + TR;

    const static int B = 0b00000001, T = 0b00000010, L = 0b00000100, R = 0b00001000;

    Entity();
    Entity(
        ShaderProgram* shader,
        GLuint textureID,
        glm::mat4* projectionMatrix,
        glm::mat4* viewMatrix,
        glm::vec3 position = glm::vec3(0, 0, 0),
        glm::vec3 scale = glm::vec3(1, 1, 1),
        glm::vec3 velocity = glm::vec3(0, 0, 0),
        glm::vec3 acceleration = glm::vec3(0, 0, 0),
        glm::vec4 color = glm::vec4(1, 1, 1, 1)
    );
    Entity(const Entity& rhs);
    ~Entity();
    
    Entity& operator=(const Entity& rhs);

    void Draw();

    glm::mat4 RecalculateMM();

    void resize(int tris);
    void calculateUVs(int x, int y, int index, int quadIndex = 0);

    EntityType entityType;

    glm::vec3 position;
    glm::vec3 scale;
    glm::vec3 velocity;
    glm::vec3 acceleration;
    glm::vec4 color;

    ShaderProgram* shader;
    GLuint textureID;
    glm::mat4* projectionMatrix;
    glm::mat4* viewMatrix;

    int vertCount;
    float* verts;
    float* uvs;
    /*
    int datasize;
    void* data;
    */
    int textureIndex;

    //use bitwise operations to fill out collisions
    int collision;
};

//Box Box collision
bool BoxBoxC(const Entity& rhs, const Entity& lhs);