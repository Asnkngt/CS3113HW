#include "Globals.h"
bool GameExit;
glm::mat4 projectionMatrix;
glm::mat4 viewMatrix;
glm::mat4 identityMatrix;

GameState state = GameState::TITLE;
void (*inits[GameState::GS_LAST])(void);
void (*draws[GameState::GS_LAST])(void);
void (*updates[GameState::GS_LAST])(void);
void (*fixedupdates[GameState::GS_LAST])(void);

int main(int argc, char *argv[]) {
    Init();

    inits[GameState::TITLE] = InitTitle;
    inits[GameState::GAME] = InitGame;
    inits[GameState::END] = InitEnd;

    draws[GameState::TITLE] = DrawTitle;
    draws[GameState::GAME] = DrawGame;
    draws[GameState::END] = DrawEnd;

    updates[GameState::TITLE] = UpdateTitle;
    updates[GameState::GAME] = UpdateGame;
    updates[GameState::END] = UpdateEnd;

    fixedupdates[GameState::TITLE] = FixedUpdateTitle;
    fixedupdates[GameState::GAME] = FixedUpdateGame;
    fixedupdates[GameState::END] = FixedUpdateEnd;

    float lastFrameTick = (float)SDL_GetTicks() / 1000.0f;
    float accumulator = 0.0f;

    GameState tmpState = GameState::GS_LAST;
    SDL_Event event;
    GameExit = false;
    while (!GameExit) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                GameExit = true;
                break;
            case SDL_WINDOWEVENT_CLOSE:
                GameExit = true;
                break;
            case SDL_WINDOWEVENT:
                switch (event.window.event) {
                case SDL_WINDOWEVENT_RESIZED:
                    WindowResize(event.window.data1, event.window.data2);
                    break;
                }
                break;
            }
        }
        float currentTick = (float)SDL_GetTicks() / 1000.0f;
        float elapsed = currentTick - lastFrameTick;
        accumulator += elapsed;
        if (tmpState != state) {
            tmpState = state;
            accumulator = 0.0f;
            inits[state]();
        }
        while (accumulator > FIXEDTICK) {
            accumulator -= FIXEDTICK;
            fixedupdates[state]();
        }
        if (tmpState != state) { continue; }
        updates[state]();
        if (tmpState != state) { continue; }
        draws[state]();
        lastFrameTick = currentTick;
    }

    MainExit();
    return 0;
}