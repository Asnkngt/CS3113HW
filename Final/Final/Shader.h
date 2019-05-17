#pragma once
#include "Globals.h"
struct Shader;
extern std::vector<Shader> ShaderPrograms;
//extern std::vector<GLuint> ShaderTextures;
extern size_t ShaderInUse;

struct Shader {
    GLuint programID = -1;
    GLuint vertexShader = -1;
    GLuint fragmentShader = -1;

    GLuint pos = -1;
    GLuint uv = -1;

    GLuint modelMat = -1;
    GLuint viewMat = -1;
    GLuint projMat = -1;
    GLuint col = -1;

    std::vector<GLuint> extra;
};

GLuint loadShaderFromFile(const std::string &shaderFile, GLenum type);
size_t loadProgramData(const std::string& vertShader, const std::string& fragShader, const std::string& p = "position", const std::string& uv = "texCoord");
size_t loadExtraData(const size_t& shaderIndex, const std::string& dataName);

void enableAttributes(size_t shaderIndex);
void disableAttributes();
void setColor(const glm::vec3& col = glm::vec3(0, 0, 0));
void loadBufferData(const Entity& e);
void loadAndDraw(Entity& e);
void loadAndDraw(Entity* e);
void shaderCleanup();

void SetMatrix(GLuint i, const glm::mat4 &matrix);