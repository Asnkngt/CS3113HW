#pragma once
#include "Headers.h"
enum EntityType;
class EntityData;
class Entity;

enum EntityType {
    STATIC = 0,
    VOID,//unused or going to be deleted
    PLAYER,
    ENEMY,
    TEXT,
    ET_LAST//used for array of functions
};

class EntityData {
public:
    EntityData(const glm::vec3& position = glm::vec3(0, 0, 0), const float& angle = 0.0f, const glm::vec3& scale = glm::vec3(1, 1, 1), const glm::vec3& velocity = glm::vec3(0, 0, 0), const glm::vec3& acceleration = glm::vec3(0, 0, 0), const glm::vec4& color = glm::vec4(1, 1, 1, 1), const EntityType& entitytype = EntityType::STATIC);
    EntityData(const EntityData& rhs);
    ~EntityData();

    EntityData& operator=(const EntityData& rhs);

    glm::mat4 modelMatrix();

    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 acceleration;
    float angle;
    glm::vec4 color;
    glm::vec3 scale;
    EntityType entityType = EntityType::VOID;
    float rotation;//in the xy plane only. In radians
};

const int ColliderCircle = 1;
const int ColliderBox = 2;

/*
struct Collider2D {
    int type = ColliderCircle;
    glm::vec2 data = glm::vec3(0, 0, 0);
};*/

class Entity
{
public:
    Entity();
    ~Entity();
    Entity(GLuint shader, glm::mat4 * projMat, glm::mat4 * viewMat, Entity * parent = nullptr);
    void LoadQuad(const glm::vec2& dimensions, const glm::vec2& position);
    void LoadQuad(const glm::vec2& dimensions, const std::vector<glm::vec2>& positions, int uvW = 1, int uvH = 1, const std::vector<int>& indices = { 0 }, const std::vector<int>& rotations = { 0 });

    bool updated = true;
    bool calculated = false;
    glm::mat4 RecalculateMM();
    void LoadVBO();
    int vertCount;
    glm::vec2* verts;
    glm::vec2* uvs;
    void resize(int tris);
    void calculateUVs(int x, int y, int index, int quadIndex = 0, int rotation = 0);

    EntityData data;
    std::vector<int> extraData = {};
    GLuint shader;
    Entity* parent;
    glm::mat4* projMatrix;
    glm::mat4* viewMatrix;
    glm::mat4 modelMatrix;

    //Collider2D collider;
    GLuint vbo = -1;
    int base[3];
};

//glm::vec2 collision2D(const Entity* lhs, const Entity* rhs);
//glm::vec2 collision2D(const Entity& lhs, const Entity& rhs);