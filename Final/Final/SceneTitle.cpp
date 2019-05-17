#include "Globals.h"

void InitTitle() {
    EntitiesCleanup();
    AudioHalt();
    WindowRatio(true, 480.0/640.0);
    PlaySound(backgroundMusicCalm, true, 50);
    viewMatrix = glm::scale(identityMatrix, glm::vec3(0.25f, 0.25f, 1.0f));
    if (ShaderInUse != -1) { SetMatrix(ShaderPrograms[ShaderInUse].viewMat, viewMatrix); }
    for (int i = 0; i < 6; ++i) {
        Text.push_back(new Entity(ShaderIndex_Texture, &projectionMatrix, &viewMatrix));
    }
    textLoader.calculateBoundedText(Text[0], "CS3113 FINAL-Charles Chan", 1, -2.0, 2.0);
    textLoader.calculateBoundedText(Text[1], "Keys:", 1, -0.5, 0.5);
    textLoader.calculateBoundedText(Text[2], "Move: WASD", 1, -1.0, 1.0);
    textLoader.calculateBoundedText(Text[3], "Attack: Left Click", 1, -1.75, 1.75);
    textLoader.calculateBoundedText(Text[4], "Scan: Space", 1, -1.0, 1.0);
    textLoader.calculateBoundedText(Text[5], "Left Click to start", 1, -1.75, 1.75);
    
    Text[0]->data.position.y = 2.5f; Text[0]->data.position.x = -0.0f;
    Text[1]->data.position.y = 1.5f; Text[1]->data.position.x = -1.5f;
    Text[2]->data.position.y = 0.5f; Text[2]->data.position.x = -1.0f;
    Text[3]->data.position.y = -0.5f; Text[3]->data.position.x = -0.25f;
    Text[4]->data.position.y = -1.5f; Text[4]->data.position.x = -1.0f;
    Text[5]->data.position.y = -2.5f; Text[5]->data.position.x = -0.25f;
}

void DrawTitle() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    enableAttributes(ShaderIndex_Texture);
    setColor(glm::vec3(1, 1, 1));
    glUniform1i(ShaderPrograms[ShaderIndex_Texture].extra[ShaderIndex_Texture_Text], textures[textLoader.textID]);
    for (Entity* e : Text) {
        loadAndDraw(e);
    }
    disableAttributes();
    SDL_GL_SwapWindow(DisplayWindow);
}

void UpdateTitle() {
    if ((SDL_GetMouseState(nullptr, nullptr)&SDL_BUTTON_LMASK)) {
        state = GameState::GAME;
    }
}

void FixedUpdateTitle() {}