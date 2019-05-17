#include "Globals.h"
SDL_Window* DisplayWindow;
SDL_GLContext context;
const char* WindowName = "Final Project";
const Uint8 *Keys;

int WindowWidth = 480;
int WindowHeight = 640;

int WindowWidthUsed = 480;
int WindowHeightUsed = 640;

bool useWindowRatio = false;
double windowRatio = 1920.0 / 1080.0;

GLuint ShaderIndex_Texture = -1;
size_t ShaderIndex_Texture_Text = -1;
GLuint ShaderIndex_Effect = -1;
size_t ShaderIndex_Effect_Text1 = -1;
size_t ShaderIndex_Effect_Text2 = -1;

size_t fbo_diffuse = -1;
size_t fbo_light = -1;

std::vector<Entity*> Player;
std::vector<Entity*> Enemies;
std::vector<Entity*> Entities;
std::vector<Entity*> Text;

TTFText textLoader;
size_t TextureIndexOutline = -1;
size_t TextureIndexLight = -1;
size_t TextureIndexEcho = -1;

int backgroundMusicCalm;
int backgroundMusicNether;
int audioPlayerAttack;
int audioPlayerHit;
int audioEnemyAttack;
int audioEnemyHit;

glm::vec2 WindowPos(int x, int y) { return glm::vec2((float)x / WindowWidth, (float)y / WindowHeight); }

void WindowReset() {
    glViewport((WindowWidth - WindowWidthUsed) / 2, (WindowHeight - WindowHeightUsed) / 2, WindowWidthUsed, WindowHeightUsed);
}

void WindowResize(int w, int h) {
    WindowHeight = h;
    WindowWidth = w;
    if (useWindowRatio) {
        int delta;
        if (w < h*windowRatio) {
            WindowWidthUsed = w;
            WindowHeightUsed = (int)(w / windowRatio);
            delta = (h - WindowHeightUsed) / 2;
            glViewport(0, delta, WindowWidthUsed, WindowHeightUsed);
        }
        else {
            WindowWidthUsed = (int)(h * windowRatio);
            WindowHeightUsed = h;
            delta = (w - WindowWidthUsed) / 2;
            glViewport(delta, 0, WindowWidthUsed, WindowHeightUsed);
        }
        projectionMatrix = glm::ortho((float)-windowRatio, (float)windowRatio, -1.0f, 1.0f, -1.0f, 1.0f);
        if (ShaderInUse != -1) { SetMatrix(ShaderPrograms[ShaderInUse].projMat, projectionMatrix); }
    }
    else {
        WindowWidth = w;
        WindowHeight = h;
        WindowWidthUsed = w;
        WindowHeightUsed = h;
        glViewport(0, 0, WindowWidth, WindowHeight);
        projectionMatrix = glm::ortho(-(float)WindowWidth / WindowHeight, (float)WindowWidth / WindowHeight, -1.0f, 1.0f, -1.0f, 1.0f);
        if (ShaderInUse != -1) { SetMatrix(ShaderPrograms[ShaderInUse].projMat, projectionMatrix); }
    }
}

void WindowRatio(bool useRatio, const double & ratio){
    useWindowRatio = useRatio;
    if (useWindowRatio && ratio > 0.0) {
        windowRatio = ratio;
    }
    WindowResize(WindowWidth, WindowHeight);
}

void EntitiesCleanup() {
    while (!Player.empty()) {
        delete Player[Player.size() - 1];
        Player.pop_back();
    }
    while (!Enemies.empty()) {
        delete Enemies[Enemies.size() - 1];
        Enemies.pop_back();
    }
    while (!Entities.empty()) {
        delete Entities[Entities.size() - 1];
        Entities.pop_back();
    }
    while (!Text.empty()) {
        delete Text[Text.size() - 1];
        Text.pop_back();
    }
}

void Init() {
    SDLInit();
    TextureInit();
    ShaderIndex_Texture = loadProgramData(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
    ShaderIndex_Texture_Text = loadExtraData(ShaderIndex_Texture, "diffuse");
    
    ShaderIndex_Effect = loadProgramData(RESOURCE_FOLDER"vertex_effect.glsl", RESOURCE_FOLDER"fragment_effect.glsl");
    ShaderIndex_Effect_Text1 = loadExtraData(ShaderIndex_Effect, "diffuse");
    ShaderIndex_Effect_Text2 = loadExtraData(ShaderIndex_Effect, "light");

    fbo_diffuse = GenerateFBO();
    fbo_light = GenerateFBO();

    PPInit();
    AudioInit();
    backgroundMusicCalm = LoadSound(RESOURCE_FOLDER"Calm.wav");
    backgroundMusicNether = LoadSound(RESOURCE_FOLDER"Nether.wav");
    audioPlayerAttack = LoadSound(RESOURCE_FOLDER"attack.wav");
    audioPlayerHit = LoadSound(RESOURCE_FOLDER"block.wav");
    audioEnemyAttack = LoadSound(RESOURCE_FOLDER"block.wav");
    audioEnemyHit = LoadSound(RESOURCE_FOLDER"block.wav");
    identityMatrix = glm::mat4(1.0);
    viewMatrix = glm::mat4(1.0);
}

void SDLInit() {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

    DisplayWindow = SDL_CreateWindow(WindowName, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WindowWidth, WindowHeight, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
    context = SDL_GL_CreateContext(DisplayWindow);
    SDL_GL_MakeCurrent(DisplayWindow, context);
    WindowResize(WindowWidth, WindowHeight);
#ifdef _WINDOWS
    glewInit();
#endif
    Keys = SDL_GetKeyboardState(NULL);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void TextureInit() {
    textLoader = TTFText(RESOURCE_FOLDER"expressway");
    if (!textLoader.LoadFont(4)) {
        exit(0);
    }
    TextureIndexOutline = LoadTexture(RESOURCE_FOLDER"FinalOutline.png", TextureFormat::RGBA());
    TextureIndexLight = LoadTexture(RESOURCE_FOLDER"Light.png", TextureFormat::RGBA());
    TextureIndexEcho = LoadTexture(RESOURCE_FOLDER"Echo.png", TextureFormat::RGBA());

}

void MainExit() {
    shaderCleanup();
    AudioHalt();
    EntitiesCleanup();
    SDL_Quit();
    for (std::pair<GLuint, size_t> fbo : fbos) {
        glDeleteFramebuffers(1, &fbo.first);
    }
}