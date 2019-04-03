#include "Entities.h"

Entities::Entities() :shader(nullptr), projectionMatrix(nullptr), viewMatrix(nullptr), position(glm::vec3(0, 0, 0)), scale(glm::vec3(1, 1, 1)), color(glm::vec4(1, 1, 1, 1)), datasize(0) {
    vertCount = 6;
    verts = new float[12]{
        0.5f, 0.5f, -0.5f, 0.5f, -0.5f, -0.5f,
        0.5f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f
    };
    calculateUVs(1, 1, 0);
    entityType = EntityType::STATIC;//default type
    clearData();
}

Entities::Entities(ShaderProgram* shader, glm::mat4* pM, glm::mat4* vM, glm::vec3 p, glm::vec3 s, glm::vec4 c) :shader(shader), projectionMatrix(pM), viewMatrix(vM), position(p), scale(s), color(c), datasize(0) {
    vertCount = 6;
    verts = new float[12]{
        0.5f, 0.5f, -0.5f, 0.5f, -0.5f, -0.5f,
        0.5f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f
    };
    calculateUVs(1, 1, 0);
    entityType = EntityType::STATIC;//default type
    clearData();
}

Entities::Entities(const Entities & rhs) :shader(rhs.shader), projectionMatrix(rhs.projectionMatrix), viewMatrix(rhs.viewMatrix), position(rhs.position), scale(rhs.scale), color(rhs.color), vertCount(rhs.vertCount), entityType(rhs.entityType) {
    datasize = rhs.datasize;
    verts = new float[vertCount * 2];
    uvs = new float[vertCount * 2];
    data = malloc(datasize);

    for (int i = 0; i < vertCount * 2; ++i) {
        verts[i] = rhs.verts[i];
        uvs[i] = rhs.uvs[i];
    }
    for (int i = 0; i < datasize; ++i) {
        ((char*)data)[i] = ((char*)rhs.data)[i];
    }
}

Entities::~Entities() {
    shader = nullptr;
    projectionMatrix = nullptr;
    viewMatrix = nullptr;
    clearData();
}

void Entities::clearData()
{
    if (datasize != 0) {
        free(data);
    }
    datasize = 0;
    data = nullptr;
}

Entities & Entities::operator=(const Entities & rhs)
{
    shader = rhs.shader;
    projectionMatrix = rhs.projectionMatrix;
    viewMatrix = rhs.viewMatrix;
    position = rhs.position;
    scale = rhs.scale;
    color = rhs.color;

    vertCount = rhs.vertCount;
    entityType = rhs.entityType;

    delete[] verts;
    delete[] uvs;
    clearData();

    datasize = rhs.datasize;
    
    verts = new float[vertCount * 2];
    uvs = new float[vertCount * 2];
    data = malloc(datasize);

    for (int i = 0; i < vertCount * 2; ++i) {
        verts[i] = rhs.verts[i];
        uvs[i] = rhs.uvs[i];
    }
    for (int i = 0; i < datasize; ++i) {
        ((char*)data)[i] = ((char*)rhs.data)[i];
    }

    return *this;
}

void Entities::Draw() {
    shader->SetProjectionMatrix(*projectionMatrix);
    //shader->SetModelMatrix(modelMatrix);
    shader->SetModelMatrix(RecalculateMM());
    shader->SetViewMatrix(*viewMatrix);
    shader->SetColor(color.r, color.g, color.b, color.a);

    glVertexAttribPointer(shader->positionAttribute, 2, GL_FLOAT, false, 0, verts);
    glEnableVertexAttribArray(shader->positionAttribute);

    glVertexAttribPointer(shader->texCoordAttribute, 2, GL_FLOAT, false, 0, uvs);
    glEnableVertexAttribArray(shader->texCoordAttribute);

    glDrawArrays(GL_TRIANGLES, 0, vertCount);
    glDisableVertexAttribArray(shader->positionAttribute);
    glDisableVertexAttribArray(shader->texCoordAttribute);

}

glm::mat4 Entities::RecalculateMM()
{
    //no rotations so calculate matrix directly. *Note to self* GLM is column major
    //modelMatrix =
    return glm::mat4(
        glm::vec4(scale.x, 0.0f, 0.0f, 0.0f),
        glm::vec4(0.0f, scale.y, 0.0f, 0.0f),
        glm::vec4(0.0f, 0.0f, scale.z, 0.0f),
        glm::vec4(position.x, position.y, position.z, 1));
}

void Entities::SetType(const EntityType & e)
{
    void* tmpData;
    //some types may need extra data
    switch (e)
    {
    case VOID:
        clearData();
        break;
    case PLAYER:
        clearData();
        datasize = sizeof(int);
        data = malloc(datasize);
        ((int*)data)[0] = 0;
        break;
    case ENEMY_FLY:
        //if it was a dropper, do nothing to data and restore to initial position
        if (entityType == ENEMY_DROP) {
            position = *((glm::vec3*)data);
        }
        else {
            clearData();
            datasize = sizeof(glm::vec3) + 3 * sizeof(int);
            ////init position, random counter, hp, and shift used when changed to dropper
            data = malloc(datasize);
            *((glm::vec3*)data) = position;
            ((int*)((char*)data + sizeof(glm::vec3)))[0] = 1;
            ((int*)((char*)data + sizeof(glm::vec3)))[1] = 3;
            ((int*)((char*)data + sizeof(glm::vec3)))[2] = 0;
        }
        break;
    case ENEMY_DROP:
        //if it was a flyer, do nothing 
        if (entityType == ENEMY_FLY) {
            ((int*)((char*)data + sizeof(glm::vec3)))[2] = 0;
        }
        else {//same initialization
            clearData();
            datasize = sizeof(glm::vec3) + 3 * sizeof(int);
            data = malloc(datasize);
            *((glm::vec3*)data) = position;
            ((int*)((char*)data + sizeof(glm::vec3)))[0] = 1;
            ((int*)((char*)data + sizeof(glm::vec3)))[1] = 3;
            ((int*)((char*)data + sizeof(glm::vec3)))[2] = 0;

        }
        break;
    case EXPLOSITON:
        clearData();
        datasize = sizeof(int);
        data = malloc(datasize);
        ((int*)data)[0] = 0;
        break;
    }
    entityType = e;
}

void Entities::calculateUVs(int x, int y, int index)
{
    float l, r, t, b;
    l = (index % x) / (float)x;
    r = (index % x + 1) / (float)x;
    t = (index / y) / (float)y;
    b = (index / y + 1) / (float)y;

    uvs = new float[12]{
        r, t, l, t, l, b,
        r, t, l, b, r, b
    };

}

bool BoxBoxC(const Entities & rhs, const Entities & lhs)
{
    float delta = rhs.position.y - lhs.position.y;
    delta = (delta > 0.0f) ? delta : -delta;
    delta *= 2;
    if (delta <= (rhs.scale.y + lhs.scale.y)) {
        float delta = rhs.position.x - lhs.position.x;
        delta = (delta > 0.0f) ? delta : -delta;
        delta *= 2;
        return delta <= (rhs.scale.x + lhs.scale.x);
    }
    return false;
}
