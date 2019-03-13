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

const float SPEED = 5.0f;

int objects;
bool running;
float lastFrameTick;
float** verts;
int* vertCount;
glm::vec3* positions;
glm::vec3* velocities;

glm::mat4 modelMatrix;
glm::mat4 projectionMatrix;
glm::mat4 viewMatrix;
glm::mat4 mat;
ShaderProgram shader;
glm::vec4* colors;

float rectY = 2.0f;//distance from center
float rectX = 0.5f;//distance from center
float screenX = 16.0f;
float screenY = 8.0f;

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

void draw(const glm::mat4& modelM, int i) {
    shader.SetProjectionMatrix(projectionMatrix);
    shader.SetModelMatrix(modelM);
    shader.SetViewMatrix(viewMatrix);
    shader.SetColor(colors[i].r, colors[i].g, colors[i].b, colors[i].a);

    glVertexAttribPointer(shader.positionAttribute, 2, GL_FLOAT, false, 0, verts[i]);
    glEnableVertexAttribArray(shader.positionAttribute);

    glDrawArrays(GL_TRIANGLES, 0, vertCount[i]);
    glDisableVertexAttribArray(shader.positionAttribute);
}

void init() {
    running = false;
    velocities[0] = velocities[1] = velocities[2] = velocities[3] = velocities[4] = glm::vec3(0.0f, 0.0f, 0.0f);

    positions[0] = glm::vec3(-screenX, 0.0f, 0.0f);
    positions[1] = glm::vec3(screenX, 0.0f, 0.0f);
    positions[2] = glm::vec3(0.0f, 0.0f, 0.0f);
    positions[3] = positions[4] = glm::vec3(0.0f, 0.0f, -0.5f);
    colors[3].r = colors[4].r = 0.1f;
    colors[3].g = colors[4].g = 0.1f;
    colors[3].b = colors[4].b = 0.1f;
    colors[3].a = colors[4].a = 1.0f;

}

void GameLogic(const float& elapsed, const float& currentTick) {
    const Uint8 *keys = SDL_GetKeyboardState(NULL);

    //don't process inputs until its running
    //HIT ENTER/RETURN TO START PLAYING
    if (!running) {
        running = keys[SDL_SCANCODE_RETURN];
        //Use time to create a "random" start
        if (running) {
            //reset everything to prep for game to run
            //works as both start and restart control
            init();
            running = true;
            float velY = currentTick;
            int temp = velY;
            //now a value between 0,1
            velY -= temp;
            //value between -0.75,0.75
            velY = (velY - 0.5f) * 1.5f;
            velocities[2].y = velY;
            //temp % 2->(0,1)*2->(0,2)-1->(-1,1)
            //basically direction in the x
            velocities[2].x = glm::sqrt(1.0f - velY * velY)*(temp % 2 * 2 - 1);
        }
        return;
    }
    else {
        velocities[0].y = 0.0f;
        velocities[1].y = 0.0f;

        if (keys[SDL_SCANCODE_W]) {
            velocities[0].y += 1.0f;
        }
        if (keys[SDL_SCANCODE_S]) {
            velocities[0].y -= 1.0f;
        }

        if (keys[SDL_SCANCODE_UP]) {
            velocities[1].y += 1.0f;
        }
        if (keys[SDL_SCANCODE_DOWN]) {
            velocities[1].y -= 1.0f;
        }

        for (int i = 0; i < objects; ++i) {
            positions[i] += elapsed * SPEED * velocities[i];
        }
        positions[2] += elapsed * SPEED * 0.25f * velocities[2];
        //limit paddle y positions
        positions[0].y = (positions[0].y + rectY > screenY) ? screenY - rectY : positions[0].y;
        positions[0].y = (positions[0].y - rectY < -screenY) ? -screenY + rectY : positions[0].y;
        positions[1].y = (positions[1].y + rectY > screenY) ? screenY - rectY : positions[1].y;
        positions[1].y = (positions[1].y - rectY < -screenY) ? -screenY + rectY : positions[1].y;

        //limit ball y rectX is used for x and y of ball
        //and do the bouncing so it doesn't collide the same wall multiple times
        if (positions[2].y + rectX > screenY) {
            positions[2].y = 2.0f * screenY - positions[2].y - 2.0f * rectX;
            velocities[2].y *= -1;
        }
        if (positions[2].y - rectX < -screenY) {
            positions[2].y = -2.0f * screenY - positions[2].y + 2.0f * rectX;
            velocities[2].y *= -1;
        }

        //collision check
        if (velocities[2].x < 0) {//left
            glm::vec3 tmp = positions[2] - positions[0];
            if (glm::abs(tmp.x) - rectX - rectX < 0.0f &&
                glm::abs(tmp.y) - rectY - rectX < 0.0f) {
                float mag = glm::sqrt(1 + tmp.y * tmp.y / 4.0f);
                velocities[2].x = 1.0f / mag;
                velocities[2].y = tmp.y / 2.0f / mag;
            }
        }
        else {//right
            glm::vec3 tmp = positions[2] - positions[1];
            if (glm::abs(tmp.x) - rectX - rectX < 0.0f &&
                glm::abs(tmp.y) - rectY - rectX < 0.0f) {
                float mag = glm::sqrt(1 + tmp.y * tmp.y / 4.0f);
                velocities[2].x = -1.0f / mag;
                velocities[2].y = tmp.y / 2.0f / mag;
            }
        }
        if (positions[2].x < -screenX) {//right wins
            std::cout << "right wins" << std::endl;
            colors[4].g = 1.0f;
            colors[3].r = 1.0f;
            std::cout << "press enter to play again" << std::endl;
            running = false;
        }
        else if (positions[2].x > screenX) {//left wins
            std::cout << "left wins" << std::endl;
            colors[3].g = 1.0f;
            colors[4].r = 1.0f;
            std::cout << "press enter to play again" << std::endl;
            running = false;
        }
    }
}

void DrawGame() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shader.programID);
    for (int i = 0; i < objects; ++i) {
        draw(glm::translate(mat, positions[i]), i);
    }

    SDL_GL_SwapWindow(displayWindow);
}

int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("Pong", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 640, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);

#ifdef _WINDOWS
    glewInit();
#endif
    objects = 5;
    verts = new float*[objects];
    vertCount = new int[objects];
    positions = new glm::vec3[objects];
    velocities = new glm::vec3[objects];
    colors = new glm::vec4[objects];
    
    shader.Load(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");
    
    mat = glm::mat4(1.0f);
    projectionMatrix = glm::ortho(-screenX, screenX, -screenY, screenY, -1.0f, 1.0f);
    viewMatrix = glm::mat4(1.0f);
    
    //object 0 is left paddle
    //object 1 is right paddle
    //object 2 is ball
    //object is left side
    //object is right side
    //green is winner red is loser
    for (int i = 0; i < objects; ++i) {
        //default all objects to unit squares centered at 0,0,0 with uv coords mapping to a square
        vertCount[i] = 6;
        colors[i].r = 1.0f; colors[i].g = 1.0f; colors[i].b = 1.0f; colors[i].a = 1.0f;
    }
    init();

    verts[0] = new float[vertCount[0] * 2]{
            rectX, rectY, -rectX, rectY, -rectX, -rectY,
            rectX, rectY, -rectX, -rectY, rectX, -rectY
    };
    verts[1] = new float[vertCount[1] * 2]{
            rectX, rectY, -rectX, rectY, -rectX, -rectY,
            rectX, rectY, -rectX, -rectY, rectX, -rectY
    };
    verts[2] = new float[vertCount[2] * 2]{
            rectX, rectX, -rectX, rectX, -rectX, -rectX,
            rectX, rectX, -rectX, -rectX, rectX, -rectX
    };
    verts[3] = new float[vertCount[3] * 2]{
            0.0f, screenY, -screenX, screenY, -screenX, -screenY,
            0.0f, screenY, -screenX, -screenY, 0.0f, -screenY
    };
    verts[4] = new float[vertCount[4] * 2]{
            screenX, screenY, 0.0f, screenY, -0.0f, -screenY,
            screenX, screenY, -0.0f, -screenY, screenX, -screenY
    };
    modelMatrix = glm::translate(mat, glm::vec3(0.0f, -1.0f, 0.0f));

    glEnable(GL_DEPTH_TEST);
    
    std::cout << "press enter to start playing" << std::endl;

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
        
        if (elapsed < 0.005f) {
            continue;
        }
        GameLogic(elapsed, currentTick);
        lastFrameTick = currentTick;
        DrawGame();
    }
    
    shader.Cleanup();

    //clean up memory leaks
    for (int i = 0; i < objects; ++i) {
        delete[] verts[i];
    }

    delete[] positions;
    delete[] velocities;
    SDL_Quit();
    return 0;
}
