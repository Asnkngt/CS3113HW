#include "Entity.h"

EntityData::EntityData(const glm::vec3 & p, const float & ang, const glm::vec3 & s, const glm::vec3 & v, const glm::vec3 & a, const glm::vec4 & c, const EntityType & et) : position(p), angle(ang), scale(s), velocity(v), acceleration(a), color(c), entityType(et) {}

EntityData::EntityData(const EntityData & rhs) : position(rhs.position), angle(rhs.angle), scale(rhs.scale), velocity(rhs.velocity), acceleration(rhs.acceleration), color(rhs.color), entityType(rhs.entityType) {}

EntityData::~EntityData(){}

EntityData & EntityData::operator=(const EntityData & rhs) {
    position = rhs.position;
    angle = rhs.angle;
    scale = rhs.scale;
    velocity = rhs.velocity;
    acceleration = rhs.acceleration;
    color = rhs.color;
    entityType = rhs.entityType;
    return *this;
}

glm::mat4 EntityData::modelMatrix() {
    return glm::scale(glm::rotate(glm::translate(glm::mat4(1.0), position), glm::radians(angle), glm::vec3(0, 0, 1)), scale);
}
