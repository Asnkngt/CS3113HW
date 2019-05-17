#pragma once
#include "Globals.h"

struct PPData {
    glm::vec3 pos;
    glm::vec3 vel;
    glm::vec3 size;
    //glm::vec3 color; leave it as white
    int lifeticks;
};

extern std::vector<PPData> particles;
extern std::vector<PPData> particleStatics;
extern Entity* particleEntity;

void PPInit();
void PPUpdate();
void PPCleanUp();

void Sparks(int n, const glm::vec3& position, float maxSize, float maxSpeed, int maxTime = 20);

void PPDraw(float scale=1.0f);
void PPLight(float scale=4.0f);