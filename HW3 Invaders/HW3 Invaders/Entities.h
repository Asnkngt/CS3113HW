#pragma once
#include "Headers.h"
#include "ShaderProgram.h"

enum EntityType {
    STATIC,//default type I don't plan on using a background
    VOID,//unused or going to be deleted
    PLAYER,
    BULLET_P,
    ENEMY_FLY,//enemies hovering
    ENEMY_DROP,//enemies dropping down to player not enough time to implement
    BULLET_E,
    EXPLOSITON,
    TEXT//probably not needed even though I should put down player info
};

class Entities
{
public:
    Entities();
    Entities(
        ShaderProgram* shader,
        glm::mat4* projectionMatrix,
        glm::mat4* viewMatrix,
        //float rotation = 0.0f,
        glm::vec3 position = glm::vec3(0, 0, 0),
        glm::vec3 scale = glm::vec3(1, 1, 1),
        //glm::vec3 velocity = glm::vec3(0, 0, 0),
        //glm::vec3 acceleration = glm::vec3(0, 0, 0),
        glm::vec4 color = glm::vec4(1, 1, 1, 1)
    );
    Entities(const Entities& rhs);
    ~Entities();
    
    void clearData();

    Entities& operator=(const Entities& rhs);

    void Draw();

    //calculate model matrix
    //no point in storing matricies since it will be updated every frame
    glm::mat4 RecalculateMM();

    void SetType(const EntityType& entityType);
    
    //calculate uv for uniform spritesheet
    //remember that index starts at 0
    void calculateUVs(int x, int y, int index);

    EntityType entityType;

    //for this retro style I wont need rotation, velocity or accleration
    //float rotation;
    glm::vec3 position;
    glm::vec3 scale;
    //glm::vec3 velocity;
    //glm::vec3 acceleration;
    glm::vec4 color;

    ShaderProgram* shader;
    //glm::mat4 modelMatrix;
    //these 2 will be consistent globals
    glm::mat4* projectionMatrix;
    glm::mat4* viewMatrix;

    int vertCount;
    //all objects will be 1x1 boxes centered at origin by default
    float* verts;
    float* uvs;
    //used for holding additional data and a counter for how much data there is
    //player and enemies will use this
    int datasize;
    void* data;
};
//Box Box collision
bool BoxBoxC(const Entities& rhs, const Entities& lhs);