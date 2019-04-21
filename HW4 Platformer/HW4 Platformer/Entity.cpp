#include "Entity.h"

Entity::Entity() :shader(nullptr), textureID(-1), projectionMatrix(nullptr), viewMatrix(nullptr), position(glm::vec3(0, 0, 0)), velocity(glm::vec3(0, 0, 0)), acceleration(glm::vec3(0, 0, 0)), scale(glm::vec3(1, 1, 1)), color(glm::vec4(1, 1, 1, 1)) {
	vertCount = 6;
	verts = new float[12]{
		0.5f, 0.5f, -0.5f, 0.5f, -0.5f, -0.5f,
		0.5f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f
	};
    uvs = new float[12];
	calculateUVs(1, 1, 0);
	entityType = EntityType::STATIC;
}

Entity::Entity(ShaderProgram* shader, GLuint tID, glm::mat4* pM, glm::mat4* vM, glm::vec3 p, glm::vec3 s, glm::vec3 v, glm::vec3 a, glm::vec4 c) :shader(shader), textureID(tID), projectionMatrix(pM), viewMatrix(vM), position(p), velocity(v), acceleration(a), scale(s), color(c) {
  vertCount = 6;
  verts = new float[12]{
	  0.5f, 0.5f, -0.5f, 0.5f, -0.5f, -0.5f,
	  0.5f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f
  };
  uvs = new float[12];
  calculateUVs(1, 1, 0);
  entityType = EntityType::STATIC;
}

Entity::Entity(const Entity & rhs) :shader(rhs.shader), textureID(rhs.textureID), textureIndex(rhs.textureIndex), projectionMatrix(rhs.projectionMatrix), viewMatrix(rhs.viewMatrix), position(rhs.position), velocity(rhs.velocity), acceleration(rhs.acceleration), scale(rhs.scale), color(rhs.color), vertCount(rhs.vertCount), entityType(rhs.entityType) {

    verts = new float[vertCount * 2];
    uvs = new float[vertCount * 2];

    for (int i = 0; i < vertCount * 2; ++i) {
        verts[i] = rhs.verts[i];
        uvs[i] = rhs.uvs[i];
    }
}

Entity::~Entity() {
	shader = nullptr;
	projectionMatrix = nullptr;
	viewMatrix = nullptr;
	delete[] verts;
    delete[] uvs;
}

Entity & Entity::operator=(const Entity & rhs)
{
	shader = rhs.shader;
    textureID = rhs.textureID;
	projectionMatrix = rhs.projectionMatrix;
    textureIndex = rhs.textureIndex;
	viewMatrix = rhs.viewMatrix;
	position = rhs.position;
    velocity = rhs.velocity;
    acceleration = rhs.acceleration;
	scale = rhs.scale;
	color = rhs.color;
    entityType = rhs.entityType;

    resize(rhs.vertCount / 3);

	for (int i = 0; i < vertCount * 2; ++i) {
		verts[i] = rhs.verts[i];
		uvs[i] = rhs.uvs[i];
	}
	
	return *this;
}

void Entity::Draw() {
    glBindTexture(GL_TEXTURE_2D, textureID);
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

glm::mat4 Entity::RecalculateMM()
{
	return glm::mat4(
		glm::vec4(scale.x, 0.0f, 0.0f, 0.0f),
		glm::vec4(0.0f, scale.y, 0.0f, 0.0f),
		glm::vec4(0.0f, 0.0f, scale.z, 0.0f),
		glm::vec4(position.x, position.y, position.z, 1));
}

void Entity::resize(int tris)
{
    vertCount = tris * 3;
    delete[] verts;
    delete[] uvs;

    verts = new float[vertCount * 2];
    uvs = new float[vertCount * 2];
}

void Entity::calculateUVs(int x, int y, int index, int quadIndex)
{
    int arrIndex = quadIndex * 12;
    if (arrIndex + 11 > vertCount * 2) {//out of bounds
        return;
    }
    float l, r, t, b;
    l = (index % x) / (float)x; r = (index % x + 1) / (float)x; t = (index / x) / (float)y; b = (index / x + 1) / (float)y;

    uvs[arrIndex + 0] = r; uvs[arrIndex + 1] = t; uvs[arrIndex + 2] = l; uvs[arrIndex + 3] = t;
    uvs[arrIndex + 4] = l; uvs[arrIndex + 5] = b; uvs[arrIndex + 6] = r; uvs[arrIndex + 7] = t;
    uvs[arrIndex + 8] = l; uvs[arrIndex + 9] = b; uvs[arrIndex + 10] = r; uvs[arrIndex + 11] = b;
}

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
}
