#pragma once
#include "Headers.h"
#include "Map.h"
#include "Entity.h"
#include "TextureManager.h"
#include "Shader.h"
#include "Audio.h"
#include "PointParticles.h"

#define FIXEDTICK 0.015625f

enum GameState {
    TITLE = 0,
    GAME,
    END,
    GS_LAST
};

//----------------------------------------------------------------InitHelper.cpp
extern SDL_Window* DisplayWindow;
extern SDL_GLContext context;
//extern const char* WindowName;
extern const Uint8 *Keys;

extern glm::vec2 WindowPos(int x, int y);
extern void WindowReset();
extern void WindowResize(int width, int height);
extern void WindowRatio(bool useRatio = false, const double& ratio = -1.0);

extern size_t ShaderIndex_Texture;
extern size_t ShaderIndex_Texture_Text;

extern GLuint ShaderIndex_Effect;
extern size_t ShaderIndex_Effect_Text1;
extern size_t ShaderIndex_Effect_Text2;

extern size_t fbo_diffuse;
extern size_t fbo_light;

//split things up to make logic and rendering easier
extern std::vector<Entity*> Player;
extern std::vector<Entity*> Enemies;
extern std::vector<Entity*> Entities;//map and decal
extern std::vector<Entity*> Text;
extern TTFText textLoader;
extern size_t TextureIndexOutline;
extern size_t TextureIndexLight;
extern size_t TextureIndexEcho;

extern int backgroundMusicCalm;
extern int backgroundMusicNether;
extern int audioPlayerAttack;
extern int audioPlayerHit;
extern int audioEnemyAttack;
extern int audioEnemyHit;

extern void EntitiesCleanup();

extern void Init();
extern void SDLInit();
extern void TextureInit();

//call at end to clean up all values needed
extern void MainExit();

//----------------------------------------------------------------Main.cpp
extern bool GameExit;
extern GameState state;

//Scenes.cpp
extern glm::mat4 projectionMatrix;
extern glm::mat4 viewMatrix;
extern glm::mat4 identityMatrix;

extern void InitTitle();
extern void DrawTitle();
extern void UpdateTitle();
extern void FixedUpdateTitle();

extern void InitGame();
extern void DrawGame();
extern void UpdateGame();
extern void FixedUpdateGame();

extern void InitEnd();
extern void DrawEnd();
extern void UpdateEnd();
extern void FixedUpdateEnd();