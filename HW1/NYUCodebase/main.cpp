#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#define GL_GLEXT_PROTOTYPES 1
#include <SDL_opengl.h>
#include <SDL_image.h>

#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"

#define STB_IMAGE_IMPLEMENTATION 
#include "stb_image.h"

#ifdef _WINDOWS
#define RESOURCE_FOLDER "Resources/"
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

SDL_Window* displayWindow;

int objects;
float lastFrameTick;
float** verts;
float** uvs;
int* vertCount;
glm::mat4* modelMatrix;
glm::mat4 projectionMatrix;
glm::mat4 viewMatrix;
ShaderProgram* shaders;
glm::vec4* colors;
GLuint* textures;

GLuint LoadTexture(const char * filepath, GLint paramMin = GL_NEAREST,GLint paramMag = GL_NEAREST)
{
    int w, h, comp;
    unsigned char* image = stbi_load(filepath, &w, &h, &comp, STBI_rgb_alpha);
    if (image == NULL) {
        std::cout << "Unable to load image. Make sure the path is correct\n";
        assert(false);
    }
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, paramMin);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, paramMag);

    stbi_image_free(image);

    return tex;
}

void draw(int si, int i) {
    shaders[si].SetProjectionMatrix(projectionMatrix);
    shaders[si].SetModelMatrix(modelMatrix[i]);
    shaders[si].SetViewMatrix(viewMatrix);
    shaders[si].SetColor(colors[i].r, colors[i].g, colors[i].b, colors[i].a);
    if (si == 0) {
        glVertexAttribPointer(shaders[si].positionAttribute, 2, GL_FLOAT, false, 0, verts[i]);
        glEnableVertexAttribArray(shaders[si].positionAttribute);

        glDrawArrays(GL_TRIANGLES, 0, vertCount[i]);
        
        glDisableVertexAttribArray(shaders[si].positionAttribute);
    }
    if (si == 1) {
        glVertexAttribPointer(shaders[si].positionAttribute, 2, GL_FLOAT, false, 0, verts[i]);
        glEnableVertexAttribArray(shaders[si].positionAttribute);

        glVertexAttribPointer(shaders[si].texCoordAttribute, 2, GL_FLOAT, false, 0, uvs[i]);
        glEnableVertexAttribArray(shaders[si].texCoordAttribute);
        glBindTexture(GL_TEXTURE_2D, textures[i]);

        glDrawArrays(GL_TRIANGLES, 0, vertCount[i]);
        
        glDisableVertexAttribArray(shaders[si].positionAttribute);
        glDisableVertexAttribArray(shaders[si].texCoordAttribute);

    }
}

int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);

#ifdef _WINDOWS
    glewInit();
#endif

    objects = 7;
    verts = new float*[objects];
    uvs = new float*[objects];
    vertCount = new int[objects];
    modelMatrix = new glm::mat4[objects];
    colors = new glm::vec4[objects];
    textures = new GLuint[objects];
    for (int i = 0; i < objects; ++i) {
        //default all objects to unit squares centered at 0,0,0 with uv coords mapping to a square
        vertCount[i] = 6;
        verts[i] = new float[vertCount[i] * 2]{
            0.5f, 0.5f, -0.5f, 0.5f, -0.5f, -0.5f,
            0.5f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f
        };
        uvs[i] = new float[vertCount[i] * 2]{
            1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
            1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f
        };
        //default all colors to white
        colors[i].r = 1.0f; colors[i].g = 1.0f; colors[i].b = 1.0f; colors[i].a = 1.0f;
    }

    shaders = new ShaderProgram[2];
    shaders[0].Load(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");
    shaders[1].Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
    
    glm::mat4 mat = glm::mat4(1.0f);
    projectionMatrix = glm::ortho(-1.777f, 1.777f, -1.0f, 1.0f, -1.0f, 1.0f);
    viewMatrix = glm::mat4(1.0f);
    
    //object 0 is plain green ground
    modelMatrix[0] = glm::scale(glm::scale(glm::translate(mat, glm::vec3(0.0f, -1.0f, 0.0f)), glm::vec3(1.777f, 0.2f, 1.0f)), glm::vec3(2.0f, 2.0f, 1.0f));
    colors[0].r = 0.1f; colors[0].g = 1.0f; colors[0].b = 0.3f;

    //object 1 is background
    modelMatrix[1] = glm::scale(glm::scale(glm::translate(mat, glm::vec3(0.0f, 0.0f, -0.5f)), glm::vec3(1.777f, 1.0f, 1.0f)), glm::vec3(2.0f, 2.0f, 1.0f));
    textures[1] = LoadTexture(RESOURCE_FOLDER"backgrounds.png",GL_LINEAR);
    
    
    //object 2-5 are trees
    textures[2] = textures[3] = textures[4] = textures[5] = LoadTexture(RESOURCE_FOLDER"coniferTall.png", GL_LINEAR, GL_LINEAR);
    modelMatrix[2] = modelMatrix[3] = modelMatrix[4] = modelMatrix[5];
    for (int i = 2; i < 6; ++i) {
        modelMatrix[i] = glm::translate(mat, glm::vec3((i - 3.5f) / 1.5f, -0.5f, -0.25f));
        modelMatrix[i] = glm::scale(modelMatrix[i], glm::vec3(0.25f, 0.75f, 1.0f));
    }
    //object 6 is an airplane
    textures[6] = LoadTexture(RESOURCE_FOLDER"planes.png");
    float x1 = 88.0f / 256.0f;
    float x2 = 176.0f / 256.0f;
    float yFactor = 73.0f / 512.0f;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    lastFrameTick = (float)SDL_GetTicks() / 1000.0f;
    SDL_Event event;
    bool done = false;
    while (!done) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
            }
        }
        
        float currentTick = (float)SDL_GetTicks() / 1000.0f;
        float elapsed = currentTick - lastFrameTick;
        //my gpu was at 99% usage and very loud wiithout this
        if (elapsed < 0.02f) {
            continue;
        }
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        lastFrameTick = currentTick;

        glUseProgram(shaders[0].programID);
        draw(0, 0);

        glUseProgram(shaders[1].programID);
        draw(1, 1);
        draw(1, 2);
        draw(1, 3);
        draw(1, 4);
        draw(1, 5);
        //object 6 is an airplane going across the screen
        //uses time to calculate position and uv being used
        //modify the time by a division to drag out the time it takes to move across the screen
        float modifiedTime1 = currentTick / 5.0f;
        int modifiedTime2 = modifiedTime1;

        modelMatrix[6] = glm::translate(mat, glm::vec3((modifiedTime1 - modifiedTime2)*4.0f - 2.0f, 0.5, 0.0f));
        modelMatrix[6] = glm::scale(modelMatrix[6], glm::vec3(0.5f, 0.25f, 1.0f));
        
        int uvShift = (int)(modifiedTime1 * 16) % 4 + 1;
        uvShift = (uvShift == 4) ? 2 : uvShift;
        
        delete[] uvs[6];
        uvs[6] = new float[vertCount[6] * 2]{
            x2, uvShift*yFactor, x1, uvShift*yFactor, x1, (uvShift + 1)*yFactor,
            x2, uvShift*yFactor, x1, (uvShift + 1)*yFactor, x2, (uvShift + 1)*yFactor
        };

        draw(1, 6);

        SDL_GL_SwapWindow(displayWindow);
    }
    
    shaders[0].Cleanup();
    shaders[1].Cleanup();
    
    //clean up memory leaks
    for (int i = 0; i < objects; ++i) {
        delete[] verts[i];
        delete[] uvs[i];
    }

    SDL_Quit();
    return 0;
}
