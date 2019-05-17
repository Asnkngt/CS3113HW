#include "PointParticles.h"

std::vector<PPData> particles;
std::vector<PPData> particleStatics;
Entity* particleEntity = nullptr;

void PPInit() {
    delete particleEntity;
    particleEntity = new Entity(ShaderIndex_Texture, &projectionMatrix, &viewMatrix);
}

void PPUpdate(){
    for (size_t i = 0; i < particles.size(); ++i) {
        particles[i].pos += particles[i].vel*FIXEDTICK;
        --particles[i].lifeticks;
        if (particles[i].lifeticks <= 0) {
            particles[i] = particles[particles.size() - 1];
            particles.pop_back();
            --i;
        }
    }
}

void PPCleanUp() {
    particles.clear();
    particleStatics.clear();
    //particleEntity = nullptr;
}

void Sparks(int n, const glm::vec3 & position, float maxSize, float maxSpeed, int maxTime) {
    PPData data;
    data.pos = position;
    for (int i = 0; i < n; ++i) {
        float size = maxSize * (rand() % 1024) / 1024.0f;
        data.size = glm::vec3(size, size, 1.0f);
        float rad = glm::radians(360.0f * (rand() % 1024 / 1024.0f));
        float speed = maxSpeed * (rand() % 1024 / 1024.0f);
        data.vel = glm::vec3(speed*glm::cos(rad), speed*glm::sin(rad), 0);
        data.lifeticks = rand() % maxTime;
        particles.push_back(data);
    }
}

void PPDraw(float scale) {
    glDepthMask(GL_FALSE);
    for (const PPData& p : particles) {
        particleEntity->data.position = p.pos;
        particleEntity->data.scale = p.size*scale;
        particleEntity->calculated = false;
        loadAndDraw(particleEntity);
    }
    glDepthMask(GL_TRUE);
}
void PPLight(float scale) {
    glDepthMask(GL_FALSE);
    setColor(glm::vec3(.85, .65, .35));
    for (const PPData& p : particles) {
        particleEntity->data.position = p.pos;
        particleEntity->data.scale = p.size*scale;
        particleEntity->calculated = false;
        loadAndDraw(particleEntity);
    }
    for (int i = 0; i < particleStatics.size(); ++i) {
        setColor(glm::vec3(0.85, 0.65, 0.35));
        if (i == 0) { setColor(glm::vec3(0.5, 0.5, 0.5)); }
        PPData& p = particleStatics[i];
        particleEntity->data.position = p.pos;
        particleEntity->data.scale = p.size;
        particleEntity->calculated = false;
        loadAndDraw(particleEntity);
    }

    setColor(glm::vec3(1, 1, 1));
    glDepthMask(GL_TRUE);
}
/*
void PPDrawLight(float scale) {
    glDepthMask(GL_FALSE);
    for (const PPData& p : particles) {
        particleEntity->data.position = p.pos;
        particleEntity->data.scale = p.size*scale;
        particleEntity->calculated = false;
        loadAndDraw(particleEntity);
    }
    glDepthMask(GL_TRUE);
}*/