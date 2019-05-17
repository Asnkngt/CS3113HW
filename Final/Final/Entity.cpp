#include "Entity.h"

Entity::Entity() : data(), shader(-1), projMatrix(nullptr), viewMatrix(nullptr), modelMatrix(1.0), parent(nullptr) {
    LoadQuad(glm::vec2(0.5f, 0.5f), glm::vec2(0.0f, 0.0f));
}

Entity::Entity(GLuint s, glm::mat4 * pM, glm::mat4 * vM, Entity * p) : data(), shader(s), projMatrix(pM), viewMatrix(vM), modelMatrix(1.0), parent(p) {
    LoadQuad(glm::vec2(0.5f, 0.5f), glm::vec2(0.0f, 0.0f));
}
/*
Entity::Entity(const Entity & rhs) : data(rhs.data), updated(rhs.updated), calculated(rhs.calculated), shader(rhs.shader), projMatrix(rhs.projMatrix), viewMatrix(rhs.viewMatrix), modelMatrix(rhs.modelMatrix), parent(rhs.parent) {
    LoadVBO();
}*/

Entity::~Entity() {
	projMatrix = nullptr;
	viewMatrix = nullptr;
	delete[] verts;
    delete[] uvs;
    glDeleteBuffers(1, &vbo);
}

/*
Entity & Entity::operator=(const Entity & rhs) {
    data = rhs.data;
    shader = rhs.shader;
    updated = rhs.updated;
    calculated = rhs.calculated;
    shader = rhs.shader;
    projMatrix = rhs.projMatrix;
    viewMatrix = rhs.viewMatrix;
    modelMatrix = rhs.modelMatrix;
    parent = rhs.parent;

    resize(rhs.vertCount / 3);
    for (int i = 0; i < vertCount; ++i) {
        verts[i] = rhs.verts[i];
        uvs[i] = rhs.uvs[i];
    }
    LoadVBO();
    return *this;
}*/

//every update, default values to updated->false and calculated->false
glm::mat4 Entity::RecalculateMM() {
    if (updated&&calculated) { return modelMatrix; }
    
    Entity* tmp = this;
    std::vector<Entity*> stack;
    bool needUpdate = false;
    while (tmp != nullptr) {
        if (tmp->updated) {
            needUpdate = true;
        }
        stack.push_back(tmp);
        tmp = tmp->parent;
    }
    if (!needUpdate) { return modelMatrix; }

    size_t i = stack.size() - 1;
    if (stack[i]->updated && !stack[i]->calculated) {
        stack[i]->modelMatrix = stack[i]->data.modelMatrix();
        stack[i]->calculated = true;
    }
    if (stack.size() > 1) {
        for (i = stack.size() - 1; i > 0;) {
            --i;
            if ((stack[i]->parent->updated|| stack[i]->updated) && !stack[i]->calculated) {
                stack[i]->updated = true;
                stack[i]->calculated = true;
                stack[i]->modelMatrix = stack[i]->parent->modelMatrix * stack[i]->data.modelMatrix();
            }
        }
    }
    return modelMatrix;
}

void Entity::resize(int tris) {
    vertCount = tris * 3;
    delete[] verts;
    delete[] uvs;

    verts = new glm::vec2[vertCount];
    uvs = new glm::vec2[vertCount];
}

void Entity::calculateUVs(int x, int y, int index, int quadIndex, int rot) {
    int arrIndex = quadIndex * 6;
    if (arrIndex + 5 > vertCount) {
        return;
    }
    float l, r, t, b;
    glm::vec2 pos[4];
    l = (index % x) / (float)x; r = (index % x + 1) / (float)x; t = (index / x) / (float)y; b = (index / x + 1) / (float)y;
    pos[0].x = l; pos[0].y = t;//tl
    pos[1].x = r; pos[1].y = t;//tr
    pos[2].x = r; pos[2].y = b;//br
    pos[3].x = l; pos[3].y = b;//bl

    uvs[arrIndex + 0] = pos[(1 - rot) & 0b11];
    uvs[arrIndex + 1] = pos[(0 - rot) & 0b11];
    uvs[arrIndex + 2] = pos[(3 - rot) & 0b11];
    uvs[arrIndex + 3] = pos[(1 - rot) & 0b11];
    uvs[arrIndex + 4] = pos[(3 - rot) & 0b11];
    uvs[arrIndex + 5] = pos[(2 - rot) & 0b11];
}
/*
bool BoxBoxC(const Entity & rhs, const Entity & lhs)
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
}*/

void Entity::LoadVBO() {
    if (vbo == -1) { glGenBuffers(1, &vbo); }

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    base[0] = 0;
    base[1] = base[0] + sizeof(verts[0]) * vertCount;
    base[2] = base[1] + sizeof(uvs[0]) * vertCount;
    glBufferData(GL_ARRAY_BUFFER, base[2], NULL, GL_STATIC_DRAW);

    glBufferSubData(GL_ARRAY_BUFFER, base[0], base[1] - base[0], verts);
    glBufferSubData(GL_ARRAY_BUFFER, base[1], base[2] - base[1], uvs);
}

void Entity::LoadQuad(const glm::vec2 & dimensions, const glm::vec2 & position) {
    LoadQuad(dimensions, std::vector<glm::vec2>(1, position));
}

void Entity::LoadQuad(const glm::vec2& dim, const std::vector<glm::vec2>& pos, int uvW, int uvH, const std::vector<int>& indices, const std::vector<int>& rots) {
    resize(2 * pos.size());
    size_t tmp;
    for (size_t i = 0; i < pos.size(); ++i) {
        tmp = i * 6;
        verts[tmp + 0] = glm::vec2(dim.x, dim.y) + pos[i];
        verts[tmp + 1] = glm::vec2(-dim.x, dim.y) + pos[i];
        verts[tmp + 2] = glm::vec2(-dim.x, -dim.y) + pos[i];
        verts[tmp + 3] = glm::vec2(dim.x, dim.y) + pos[i];
        verts[tmp + 4] = glm::vec2(-dim.x, -dim.y) + pos[i];
        verts[tmp + 5] = glm::vec2(dim.x, -dim.y) + pos[i];
        calculateUVs(uvW, uvH, indices[i%indices.size()],i, rots[i%rots.size()]);
    }
    LoadVBO();
}

/*
//vec2(0,0)=no collision
glm::vec2 collision2D(Entity * lhs, Entity * rhs) {
    return collision2D(*lhs, *rhs);
}

glm::vec2 collision2D(Entity & lhs, Entity & rhs)
{
    glm::vec2 ret = glm::vec2(0, 0);
    int coltype = lhs.collider.type | rhs.collider.type;
    

    if (coltype == (ColliderCircle| ColliderCircle)) {//circle circle collision

    }
    else if (coltype == (ColliderBox | ColliderBox)) {}//box box collision (not used in this)
    else if (coltype == (ColliderCircle | ColliderBox)) {//boxsphere
        const Entity circle = (lhs.collider.type == ColliderCircle) ? lhs : rhs;
        const Entity box = (lhs.collider.type == ColliderCircle) ? lhs : rhs;

    }
    return ret;
}
*/