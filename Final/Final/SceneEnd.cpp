#include "Globals.h"

float col;
int ticks = 256;
int currTick;

void InitEnd() {
    EntitiesCleanup();
    WindowRatio(true, 1920.0 / 1080.0);
    viewMatrix = glm::scale(identityMatrix, glm::vec3(0.25f, 0.25f, 1.0f));

    Text.push_back(new Entity(ShaderIndex_Texture, &identityMatrix, &identityMatrix));
    textLoader.calculateBoundedText(Text[0], "Game Over", 1, -1.0, 1.0);
    Text[0]->LoadVBO();
    col = 1.0f;
    currTick = ticks;
    setColor(glm::vec3(col, col, col));
}

void DrawEnd() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    enableAttributes(ShaderIndex_Texture);
    setColor(glm::vec3(col, col, col));
    glUniform1i(ShaderPrograms[ShaderIndex_Texture].extra[ShaderIndex_Texture_Text], textures[textLoader.textID]);
    for (Entity* e : Text) {
        loadAndDraw(e);
    }
    disableAttributes();
    SDL_GL_SwapWindow(DisplayWindow);
}

void UpdateEnd() {
    if (currTick <= 0) {
        state = GameState::TITLE;
    }
}

void FixedUpdateEnd() {
    --currTick;
    col = (float)currTick / ticks;
}