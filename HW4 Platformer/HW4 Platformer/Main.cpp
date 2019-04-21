#include "Headers.h"
#include "ShaderProgram.h"
#include "TextureManager.h"
#include "Entity.h"

SDL_Window* displayWindow;

//static stage stored as single entity
Entity stage;
int mapX, mapY;
//lighting effect
Entity effect;
float brightness;
int torchCount;

std::vector<std::vector<int>> map;
std::vector<Entity> entities;//players and torches
std::vector<Entity> text;

bool running;
float lastFrameTick;

glm::mat4 projectionMatrix;
glm::mat4 viewMatrix;
glm::mat4 mat;

ShaderProgram shaderDefault;
ShaderProgram shaderText;
ShaderProgram shaderEffect;
TTFText textLoader;

GLuint* charTextures;
GLuint stageTexture;

const Uint8 *keys;

const char* WindowName = "Game";
float accumulator = 0.0f;
float startT;
const float stepT = 0.04f;

float speed = 25.0f;

void LoadMap();
void DrawGame();
void GameLogic(float deltaT, float currT);
void FixedUpdate();

void initGl() {
    SDL_Init(SDL_INIT_VIDEO);
    
    displayWindow = SDL_CreateWindow(WindowName, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 640, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);

#ifdef _WINDOWS
    glewInit();
#endif
    shaderDefault.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
    shaderText.Load(RESOURCE_FOLDER"vertex_text.glsl", RESOURCE_FOLDER"fragment_text.glsl");
    shaderEffect.Load(RESOURCE_FOLDER"vertex_effect.glsl", RESOURCE_FOLDER"fragment_effect.glsl");

    keys = SDL_GetKeyboardState(NULL);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

}

//loads a small map drawn on a text file
void LoadMap() {
    stage = Entity(&shaderDefault, stageTexture, &projectionMatrix, &viewMatrix);
    stage.entityType = EntityType::TERRAIN;
    int tmp;

    std::ifstream file;
    file.open(RESOURCE_FOLDER"map.txt");
    if (!file.is_open()) {
        std::cout << "ERROR: cannot open map file" << std::endl;
    }
    file >> mapX >> mapY;
    for (tmp = 0; tmp < mapY; ++tmp) {
        map.push_back(std::vector<int>(mapX));
    }
    
    for (int i = 0; i < mapY; ++i) {
        for (int j = 0; j < mapX; ++j) {
            file >> map[mapY - i - 1][j];
            //std::cout << map[mapY - i - 1][j];
        }
        //std::cout << std::endl;
    }
    file.close();

    Entity torch(&shaderDefault, stageTexture, &projectionMatrix, &viewMatrix);//use to duplicate torches quickly
    torch.entityType = EntityType::ITEM;
    torch.calculateUVs(3, 1, 2);

    stage.scale = glm::vec3(1.25f, 1.25f, 1.0f);
    torch.scale = stage.scale;
    stage.position = glm::vec3(-stage.scale.x * 0.5f * mapX, -stage.scale.y * 0.5f * mapY, -0.75f);

    torchCount = 1;
    stage.resize(mapX * mapY * 2);
    //setup map with corner at (0,0)
    for (int i = 0; i < mapY; ++i) {
        for (int j = 0; j < mapX; ++j) {
            if (map[i][j] == 2) {//torch found
                torch.position = glm::vec3(torch.scale.x * (j - (mapX * 0.5f) + 0.5f), torch.scale.y * (i - (mapY * 0.5f) + 0.5f), -0.5f);
                ++torchCount;
                entities.push_back(torch);
                map[i][j] = 0;
            }
            tmp = i * mapX + j;
            stage.calculateUVs(3, 1, map[i][j], tmp);
            tmp *= 12;
            //std::cout << ((float)x * -0.5f) << " " << j << " " << y * -0.5f << std::endl;
            stage.verts[tmp + 0] = (float)j + 1; stage.verts[tmp + 1] = (float)i;
            stage.verts[tmp + 2] = (float)j; stage.verts[tmp + 3] = (float)i;
            stage.verts[tmp + 4] = (float)j; stage.verts[tmp + 5] = (float)i + 1;
            stage.verts[tmp + 6] = (float)j + 1; stage.verts[tmp + 7] = (float)i;
            stage.verts[tmp + 8] = (float)j; stage.verts[tmp + 9] = (float)i + 1;
            stage.verts[tmp + 10] = (float)j + 1; stage.verts[tmp + 11] = (float)i + 1;
        }
    }
    brightness = 1.0f / torchCount;
    torchCount = 1;
    //std::cout << torchcount << " " << brightness << std::endl;
}

void BoxMapC(Entity& e) {
    e.collision = 0;
    e.position.y += e.velocity.y * stepT;
    glm::vec3 check;
    //position + half height up or down depending on velocity
    check = e.position + glm::vec3(0.0f, ((e.velocity.y > 0) ? e.scale.y : -e.scale.y)*0.5f, 0.0f);
    check.x = (check.x - stage.position.x) / stage.scale.x;
    check.y = (check.y - stage.position.y) / stage.scale.y;
    if (check.y >= 0 && check.y < mapY&&check.x >= 0 && check.x < mapX) {
        //for now only 0 will be empty air
        if (map[(int)check.y][(int)check.x]) {
            //collision detected
            if (e.velocity.y > 0) {
                e.position.y -= (check.y - (int)check.y)*stage.scale.y;
                e.collision |= Entity::T;
            }
            else {
                e.position.y += (1 + (int)check.y - check.y)*stage.scale.y;
                e.collision |= Entity::B;
            }
            e.velocity.y = 0.0f;
        }
    }

    e.position.x += e.velocity.x * stepT;
    check = e.position + glm::vec3(((e.velocity.x > 0) ? e.scale.x : -e.scale.x)*0.5f, 0.0f, 0.0f);
    check.x = (check.x - stage.position.x) / stage.scale.x;
    check.y = (check.y - stage.position.y) / stage.scale.y;
    if (check.y >= 0 && check.y < mapY&&check.x >= 0 && check.x < mapX) {
        //for now only 0 will be empty air
        if (map[(int)check.y][(int)check.x]) {
            if (e.velocity.x > 0) {
                e.position.x -= (check.x - (int)check.x)*stage.scale.x;
                e.collision |= Entity::R;
            }
            else {
                e.position.x += (1 + (int)check.x - check.x)*stage.scale.x;
                e.collision |= Entity::L;
            }
            e.velocity.x = 0.0f;
        }
    }

}

void initGame() {
    mat = glm::mat4(1.0f);
    charTextures = new GLuint[2];

    charTextures[0] = LoadTexture(RESOURCE_FOLDER"char1.png", TextureFormat::RGBA());//10x16
    charTextures[1] = LoadTexture(RESOURCE_FOLDER"char2.png", TextureFormat::RGBA());//16x10
    stageTexture = LoadTexture(RESOURCE_FOLDER"blocks.png", TextureFormat::RGBA());

    projectionMatrix = glm::ortho(-1.777f, 1.777f, -1.0f, 1.0f, -1.0f, 1.0f);
    mat = glm::mat4(1.0f);
    viewMatrix = glm::scale(mat, glm::vec3(0.25f, 0.25f, 1.0f));

    entities.clear();
    text.clear();
    //entity 0 should be player;
    entities.push_back(Entity(&shaderDefault, charTextures[0], &projectionMatrix, &viewMatrix));
    entities[0].entityType = EntityType::PLAYER;
    entities[0].textureIndex = 3;
    entities[0].calculateUVs(3, 2, entities[0].textureIndex);
    entities[0].acceleration.y = -10.0f;
    entities[0].scale.x = 0.625f;
    entities[0].position.z = -0.25f;
    LoadMap();

    effect = Entity(&shaderEffect, charTextures[0], &projectionMatrix, &viewMatrix);
    effect.color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    //make effect fill screen
    for (int i = 0; i < 12; ++i) {
        effect.verts[i] *= 2;
    }

    startT = (float)SDL_GetTicks() / 1000.0f;
    accumulator = 0.0f;

    text.push_back(Entity(&shaderText, textLoader.textID, &projectionMatrix, &viewMatrix));
    text.push_back(Entity(&shaderText, textLoader.textID, &projectionMatrix, &viewMatrix));
    text.push_back(Entity(&shaderText, textLoader.textID, &projectionMatrix, &viewMatrix));
    textLoader.calculateBoundedText(text[0], "Current Brightness: " + std::to_string((int)(torchCount*brightness*100.0)) + "%", 0.5f, -1.0f, 1.0f);
    textLoader.calculateBoundedText(text[1], "Find torches to light up map", 0.25f, -1.0f, 1.0f);
    textLoader.calculateBoundedText(text[2], "Try going left down the hole", 0.25f, -1.0f, 1.0f);
    text[0].position = glm::vec3(entities[0].position.x, entities[0].position.y + 1.0f, 0.0f);
    text[2].position.y = -0.5f;
}

void GameLogic(float deltaT, float currT) {
    float gameT = currT - startT;
    while (accumulator+stepT<gameT)
    {
        accumulator += stepT;
        FixedUpdate();
    }
    for (Entity& e : entities) {
        if (e.entityType == EntityType::PLAYER) {
            viewMatrix = glm::translate(glm::scale(mat, glm::vec3(0.25f, 0.25f, 1.0f)), -e.position);
        }
    }
}

void DrawGame() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaderDefault.programID);
    stage.Draw();
    for (Entity& e : entities) {
        e.Draw();
    }
    glUseProgram(shaderEffect.programID);
    glUniform1f(glGetUniformLocation(shaderEffect.programID, "brightness"), torchCount * brightness);
    glUniform1f(glGetUniformLocation(shaderEffect.programID, "offset"), glm::sin(glm::pi<float>() * accumulator) * 0.075f);
    
    effect.Draw();

    glUseProgram(shaderText.programID);
    glBindTexture(GL_TEXTURE_2D, textLoader.textID);
    for (Entity& t : text) {
        t.Draw();
    }
    SDL_GL_SwapWindow(displayWindow);
}

void FixedUpdate() {
    int x = (keys[SDL_SCANCODE_LEFT] ? -1 : 0) + (keys[SDL_SCANCODE_RIGHT] ? 1 : 0);
    bool updateText = ((int)(accumulator / stepT) % 5) == 0;
    for (size_t i = 0; i < entities.size(); ++i) {
        switch (entities[i].entityType)
        {
        case EntityType::PLAYER:
            entities[i].acceleration.x = x * speed;
            entities[i].velocity.x /= 1.5f;
            entities[i].velocity += entities[i].acceleration * stepT;
            if (entities[i].collision&Entity::B && keys[SDL_SCANCODE_SPACE]) {
                entities[i].velocity.y = 7.5f;
            }
            
            //calculate final tile position
            BoxMapC(entities[i]);
            //update text position
            text[0].position = glm::vec3(entities[0].position.x, entities[0].position.y + 3.0f, 0.0f);

            if (updateText) {
                if (x == 0) {
                    entities[i].textureIndex = (entities[i].textureIndex / 3) * 3;//make it a value from 0 to 3
                }
                else {
                    entities[i].textureIndex = (entities[i].textureIndex + 1) % 3;
                    if (x > 0) {
                        entities[i].textureIndex += 3;
                    }
                }
                entities[i].calculateUVs(3, 2, entities[i].textureIndex);
            }
            break;
        case EntityType::ITEM:
            break;
        }
    }

    //collision checking
    for (size_t i = 0; i < entities.size(); ++i) {
        if (entities[i].entityType == EntityType::PLAYER) {
            for (size_t j = 0; j < entities.size(); ++j) {
                if (entities[j].entityType == EntityType::ITEM) {//try collecting
                    if (BoxBoxC(entities[i], entities[j])) {
                        ++torchCount;
                        //update text
                        textLoader.calculateBoundedText(text[0], "Current Brightness: " + std::to_string((int)(torchCount*brightness*100.0)) + "%", 0.5f, -1.0f, 1.0f);
                        entities[j] = entities[entities.size() - 1];
                        entities.pop_back();
                        --j;
                    }
                }
            }
            //should only be one player
            break;
        }
    }
}

int main(int argc, char *argv[])
{
    initGl();
    textLoader = TTFText(RESOURCE_FOLDER"expressway");
    if (!textLoader.LoadFont(4)) {
        return 0;
    }
    initGame();

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

        DrawGame();
        GameLogic(elapsed, currentTick);
        lastFrameTick = currentTick;
    }
    shaderDefault.Cleanup();
    shaderText.Cleanup();
    
    SDL_Quit();
    return 0;
}