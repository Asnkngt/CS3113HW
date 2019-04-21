#pragma once
#include "Headers.h"

class ShaderProgram {
    public:

		void Load(const char *vertexShaderFile, const char *fragmentShaderFile);
		void Cleanup();

		void SetModelMatrix(const glm::mat4 &matrix);
        void SetProjectionMatrix(const glm::mat4 &matrix);
        void SetViewMatrix(const glm::mat4 &matrix);
	
		void SetColor(float r, float g, float b, float a);
	
        GLuint LoadShaderFromString(const std::string &shaderContents, GLenum type);
        GLuint LoadShaderFromFile(const std::string &shaderFile, GLenum type);
    
        GLuint programID;
    
        GLuint projectionMatrixUniform;
        GLuint modelMatrixUniform;
        GLuint viewMatrixUniform;
		GLuint colorUniform;
	
        GLuint positionAttribute;
        GLuint texCoordAttribute;
    
        GLuint vertexShader;
        GLuint fragmentShader;
};
