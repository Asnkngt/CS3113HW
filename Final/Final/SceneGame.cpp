#include "Globals.h"
#include "PointParticles.h"
const double screenratio = 1920.0 / 1080.0;

float playerRadius = 0.25f;//player is half the size of player
//float playerSpeed = 4.0f;
float playerSpeed = 16.0f;
glm::vec2 playerWeapon = glm::vec2(0.046875f, 0.28125);
glm::vec4 playerWeaponPoints[4] = {
    glm::vec4(playerWeapon.x,playerWeapon.y,0,1),
    glm::vec4(-playerWeapon.x,playerWeapon.y,0,1),
    glm::vec4(playerWeapon.x,-playerWeapon.y,0,1),
    glm::vec4(-playerWeapon.x,-playerWeapon.y,0,1)
};

float enemyRadius = 0.375f;

Map Cave;
int attackCooldown = 64;
int attackTime = attackCooldown + 16;
int attackTick = attackCooldown;

int mobTimer;
int mobIncrementTimer;
int mobcount;
int wanderTick = 128;

int shieldTick = 32;
int regenTick = 64;

//use extra data to store hp, status, status data
const int Wander = 0;
const int Chase = 1;
const int Stunned = 2;

float lightRadius = 16.0f;

int echoTicks = 128;
float echoSpeed = playerSpeed * 3.0f;

Entity* EffectEntity = nullptr;

//brute force map collision that only uses position and sphere radius<0.5f
void MapEntityCollision(Entity* e, float radius) {
    e->RecalculateMM();
    glm::vec2 pos = e->modelMatrix*glm::vec4(0, 0, 0, 1);//actually works
    glm::ivec2 coord = pos;
    glm::vec3 delta;
    //assume always in bound -> Cave.inBound(coord) always true
    //casting pos to int puts it in map coordinates
    coord = glm::ivec2((int)pos.x + 1, (int)pos.y);
    delta.x = pos.x + radius - coord.x;
    if (Cave.map[coord.y][coord.x] == 0 && delta.x > 0) {
        e->data.position.x -= delta.x;
        pos.x -= delta.x;
        e->calculated = false;
    }
    coord = glm::ivec2((int)pos.x - 1, (int)pos.y);
    delta.x = coord.x + 1 - pos.x + radius;
    if (Cave.map[coord.y][coord.x] == 0 && delta.x > 0) {
        e->data.position.x += delta.x;
        pos.x += delta.x;
        e->calculated = false;
    }
    coord = glm::ivec2((int)pos.x, (int)pos.y + 1);
    delta.x = pos.y + radius - coord.y;
    if (Cave.map[coord.y][coord.x] == 0 && delta.x > 0) {
        e->data.position.y -= delta.x;
        pos.y -= delta.x;
        e->calculated = false;
    }
    coord = glm::ivec2((int)pos.x, (int)pos.y - 1);
    delta.x = coord.y + 1 - pos.y + radius;
    if (Cave.map[coord.y][coord.x] == 0 && delta.x > 0) {
        e->data.position.y += delta.x;
        pos.y += delta.x;
        e->calculated = false;
    }
    
    coord = glm::ivec2(pos.x, pos.y);
    if ((Cave.map[coord.y + 1][coord.x + 1] == 0 || Cave.map[coord.y + 1][coord.x] == 0 || Cave.map[coord.y][coord.x + 1] == 0)) {
        delta = glm::vec3(pos.x - coord.x - 1, pos.y - coord.y - 1, 0.0f);
        delta.z = glm::sqrt(delta.x*delta.x + delta.y*delta.y);
        if (glm::dot(delta, glm::vec3(1, 1, 0)) < 0 && delta.z < radius) {
            delta.z = (radius - delta.z) / delta.z;
            delta.x *= delta.z; delta.y *= delta.z;
            e->data.position.x += delta.x; e->data.position.y += delta.y;
            pos.x += delta.x; pos.y += delta.y;
            e->calculated = false;
        }
    }
    
    if ((Cave.map[coord.y - 1][coord.x + 1] == 0 || Cave.map[coord.y - 1][coord.x] == 0 || Cave.map[coord.y][coord.x + 1] == 0)) {
        delta = glm::vec3(pos.x - coord.x - 1, pos.y - coord.y, 0.0f);
        delta.z = glm::sqrt(delta.x*delta.x + delta.y*delta.y);
        if (glm::dot(delta, glm::vec3(1, -1, 0)) < 0 && delta.z < radius) {
            delta.z = (radius - delta.z) / delta.z;
            delta.x *= delta.z; delta.y *= delta.z;
            e->data.position.x += delta.x; e->data.position.y += delta.y;
            pos.x += delta.x; pos.y += delta.y;
            e->calculated = false;
        }
    }
    
    if ((Cave.map[coord.y + 1][coord.x - 1] == 0 || Cave.map[coord.y + 1][coord.x] == 0 || Cave.map[coord.y][coord.x - 1] == 0)) {
        delta = glm::vec3(pos.x - coord.x, pos.y - coord.y - 1, 0.0f);
        delta.z = glm::sqrt(delta.x*delta.x + delta.y*delta.y);
        if (glm::dot(delta, glm::vec3(-1, 1, 0)) < 0 && delta.z < radius) {
            delta.z = (radius - delta.z) / delta.z;
            delta.x *= delta.z; delta.y *= delta.z;
            e->data.position.x += delta.x; e->data.position.y += delta.y;
            pos.x += delta.x; pos.y += delta.y;
            e->calculated = false;
        }
    }

    if ((Cave.map[coord.y - 1][coord.x - 1] == 0 || Cave.map[coord.y - 1][coord.x] == 0 || Cave.map[coord.y][coord.x - 1] == 0)) {
        delta = glm::vec3(pos.x - coord.x, pos.y - coord.y, 0.0f);
        delta.z = glm::sqrt(delta.x*delta.x + delta.y*delta.y);
        if (glm::dot(delta, glm::vec3(-1, -1, 0)) < 0 && delta.z < radius) {
            delta.z = (radius - delta.z) / delta.z;
            delta.x *= delta.z; delta.y *= delta.z;
            e->data.position.x += delta.x; e->data.position.y += delta.y;
            pos.x += delta.x; pos.y += delta.y;
            e->calculated = false;
        }
    }
}

void attackAnimation() {
    bool collided = false;

    if (attackTick >= attackCooldown) {
        float deg = 75.0f - ((attackTick - attackCooldown) * 150.0f / (attackTime - attackCooldown));
        float rad = glm::radians(deg);
        Player[1]->data.angle = deg;
        Player[1]->data.position.x = sin(rad)*-0.5f;
        Player[1]->data.position.y = cos(rad)*0.5f;
        Player[1]->updated = true;
        Player[1]->calculated = false;
        
        Player[1]->RecalculateMM();
        //check collision with map
        glm::vec3 points[4];
        for (int i = 0; i < 4; ++i) {
            points[i] = Player[1]->modelMatrix*playerWeaponPoints[i];
        }
        for (int i = 0; i < 4; ++i) {
            if (Cave.map[(int)points[i].y][(int)points[i].x] == 0) {
                collided = true;
            }
        }
        Entity* e;
        for (size_t j = 0; j < Enemies.size(); ++j) {
            if (collided) { break; }
            e = Enemies[j];
            e->RecalculateMM();
            glm::vec3 point = e->modelMatrix*glm::vec4(0, 0, 0, 1);
            for (int i = 0; i < 4; ++i) {
                glm::vec3 delta = points[i] - point;
                if (glm::sqrt(delta.x*delta.x + delta.y*delta.y) < enemyRadius) {
                    collided = true;
                    //delta = e->data.position - Player[0]->data.position;
                    //e->data.velocity = glm::normalize(delta);
                    e->extraData[1] = Stunned;
                    e->extraData[2] = 32;

                    if (--e->extraData[0] <= 0) {
                        delete e;
                        Enemies[j] = Enemies[Enemies.size() - 1];
                        Enemies.pop_back();
                    }
                    break;
                }
            }
        }
    }

    if (attackTick == -1 || collided) {
        if (collided) {
            attackTick = attackCooldown;
            VolumeSound(audioPlayerAttack, 0);
            PlaySound(audioPlayerHit, false, 5);
            Sparks(100, Player[1]->modelMatrix*glm::vec4(0, 0, 0, 1), 0.1f, 1.0f, 50);
        }
        Player[1]->data.position.x = 0.25f;
        Player[1]->data.position.y = 0.0f;
        Player[1]->data.angle = 0;
        Player[1]->updated = true;
        Player[1]->calculated = false;
    }

    --attackTick;
}

void loadMap(int x = 64, int y = 64) {
    Cave = Map(x, y, 45);
    Cave.Load();
    int threshhold = (int)(glm::sqrt(Cave.width * Cave.height)*glm::log(glm::sqrt(Cave.width * Cave.height)) / 2.0);
    while (Cave.automata() > threshhold) {}
    Cave.lazyConnect();

    size_t index = Entities.size();
    Entities.push_back(new Entity(ShaderIndex_Texture, &projectionMatrix, &viewMatrix));
    std::vector<glm::vec2> tmppos;
    std::vector<int> indicies;
    std::vector<int> rotations;
    int tmp1, tmp2, tmp3;
    for (int i = 0; i < y; ++i) {
        for (int j = 0; j < x; ++j) {
            tmppos.push_back(glm::vec2(j + 0.5f, i + 0.5f));
            tmp1 = 0;
            tmp2 = 0;
            tmp3 = 0;
            if (Cave.map[i][j] > 0) {
                if (Cave.inBound(j, i + 1) && Cave.map[i + 1][j] == 0) { tmp1 |= 0b1000; ++tmp2; }
                if (Cave.inBound(j + 1, i) && Cave.map[i][j + 1] == 0) { tmp1 |= 0b0100; ++tmp2; }
                if (Cave.inBound(j, i - 1) && Cave.map[i - 1][j] == 0) { tmp1 |= 0b0010; ++tmp2; }
                if (Cave.inBound(j - 1, i) && Cave.map[i][j - 1] == 0) { tmp1 |= 0b0001; ++tmp2; }
                if (tmp2 == 0) {
                    indicies.push_back(57);
                }
                else if (tmp2 == 1) {
                    indicies.push_back(58);
                    while (tmp1 != 0b1000) {
                        tmp1 = (tmp1 >> 1) | ((tmp1 & 0b1) << 3);
                        --tmp3;
                    }
                }
                else if (tmp2 == 2) {
                    if (tmp1 == 0b0101) {
                        indicies.push_back(61);
                    }
                    else if (tmp1 == 0b1010) {
                        indicies.push_back(61);
                        tmp3 = 1;
                    }
                    else {
                        indicies.push_back(59);
                        while (tmp1 != 0b1100) {
                            tmp1 = (tmp1 >> 1) | ((tmp1 & 0b1) << 3);
                            --tmp3;
                        }
                    }
                }
                else if (tmp2 == 3) {
                    indicies.push_back(60);
                    while (tmp1 != 0b1101) {
                        tmp1 = (tmp1 >> 1) | ((tmp1 & 0b1) << 3);
                        --tmp3;
                    }
                }
                else {
                    indicies.push_back(62);
                }
                rotations.push_back(tmp3);
            }
            else {
                //indicies.push_back(56);
                indicies.push_back(0);
                rotations.push_back(0);
            }
        }
    }
    Entities[index]->LoadQuad(glm::vec2(0.5f, 0.5f), tmppos, 8, 8, indicies,rotations);

    index = Entities.size();
    Entities.push_back(new Entity(ShaderIndex_Texture, &projectionMatrix, &viewMatrix));
    Entities[index]->data.position = glm::vec3(Cave.end.x + 0.5f, Cave.end.y + 0.5f, 0);
    Entities[index]->calculateUVs(8, 8, 63);
    Entities[index]->updated = true;
    Entities[index]->calculated = false;
    Entities[index]->LoadVBO();

    PPData data;
    data.pos = Entities[index]->data.position;
    data.vel = glm::vec3(0, 0, 0);
    data.lifeticks = -1;
    data.size = glm::vec3(lightRadius, lightRadius, 1);
    particleStatics.push_back(data);
    
    for (int i = 0; i < (int)glm::log2((float)x*y); ++i) {
        glm::ivec2 tmp = glm::ivec2(randRange(2, x - 2), randRange(2, y - 2));
        data.size = glm::vec3(lightRadius / 2.0f, lightRadius / 2.0f, 1);
        data.pos = glm::vec3(tmp.x + 0.5f, tmp.y + 0.5f, 0);
        if (Cave.map[tmp.y][tmp.x] != 0) {
            particleStatics.push_back(data);
            index = Entities.size();
            Entities.push_back(new Entity(ShaderIndex_Texture, &projectionMatrix, &viewMatrix));
            Entities[index]->data.position = data.pos;
            Entities[index]->calculateUVs(8, 8, 55);
            Entities[index]->updated = true;
            Entities[index]->calculated = false;
            Entities[index]->LoadVBO();
        }
    }
}

int Floor;
bool reinit = true;

void startFloor() {
    reinit = true;
    while (!Enemies.empty()) {
        delete Enemies[Enemies.size() - 1];
        Enemies.pop_back();
    }
    while (!Entities.empty()) {
        delete Entities[Entities.size() - 1];
        Entities.pop_back();
    }
    Player[2]->data.scale = glm::vec3(0, 0, 0);
    Player[2]->updated = true;
    Player[2]->calculated = false;
    Player[2]->LoadVBO();
    Player[2]->extraData.resize(1);
    Player[2]->extraData[0] = -1;

    PPCleanUp();
    PPData data;//reserve light for player
    data.vel = glm::vec3(0, 0, 0);
    data.lifeticks = -1;
    data.size = glm::vec3(3, 3, 1);
    particleStatics.push_back(data);

    size_t index = Text.size();
    Text.push_back(new Entity(ShaderIndex_Texture, &projectionMatrix, &identityMatrix));
    textLoader.calculateBoundedText(Text[index], "Floor " + std::to_string(Floor), 1, -0.75, 0.75);

    Text[index]->LoadVBO();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    enableAttributes(ShaderIndex_Texture);
    glUniform1i(ShaderPrograms[ShaderIndex_Texture].extra[ShaderIndex_Texture_Text], textures[textLoader.textID]);
    loadAndDraw(Text[index]);
    glUniform1i(ShaderPrograms[ShaderIndex_Texture].extra[ShaderIndex_Texture_Text], textures[textLoader.textID]);
    disableAttributes();
    SDL_GL_SwapWindow(DisplayWindow);

    loadMap(56 + 8 * Floor, 56 + 8 * Floor);
    mobcount = 8 + 4 * Floor;
    Player[0]->data.position = glm::vec3(Cave.start.x + 0.5f, Cave.start.y + 0.5f, 0.0f);
    Player[0]->calculated = false;
    attackTick = -1;
    attackAnimation();
    attackTick = attackCooldown - 1;

    for (int i = 1024; i > 0; --i) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        enableAttributes(ShaderIndex_Texture);
        setColor(glm::vec3(i / 1024.0f, i / 1024.0f, i / 1024.0f));
        glUniform1i(ShaderPrograms[ShaderIndex_Texture].extra[ShaderIndex_Texture_Text], textures[textLoader.textID]);
        loadAndDraw(Text[index]);
        disableAttributes();
        SDL_GL_SwapWindow(DisplayWindow);
    }
    delete Text[index];
    Text.pop_back();
    ++Floor;
}

void InitGame() {
    EntitiesCleanup();
    AudioHalt();
    PlaySound(backgroundMusicNether, true, 10);
    WindowRatio(true, screenratio);

    viewMatrix = glm::scale(identityMatrix, glm::vec3(0.25f, 0.25f, 1.0f));
    if (ShaderInUse != -1) { SetMatrix(ShaderPrograms[ShaderInUse].viewMat, viewMatrix); }

    Player.push_back(new Entity(ShaderIndex_Texture, &projectionMatrix, &viewMatrix));
    Player.push_back(new Entity(ShaderIndex_Texture, &projectionMatrix, &viewMatrix, Player[0]));
    Player.push_back(new Entity(ShaderIndex_Texture, &projectionMatrix, &viewMatrix));
    Player[0]->calculateUVs(8, 8, 0, 0);
    Player[0]->LoadVBO();

    Player[0]->extraData.resize(3);
    Player[0]->extraData[0] = 10;
    Player[0]->extraData[1] = 0;
    Player[0]->extraData[2] = 0;

    Player[1]->calculateUVs(8, 8, 3, 0);
    Player[1]->LoadVBO();
    
    Text.push_back(new Entity(ShaderIndex_Texture, &identityMatrix, &identityMatrix));
    textLoader.calculateBoundedText(Text[0], "HP " + std::to_string(Player[0]->extraData[0]), 0.125f, -0.125, 0.125);
    Text[0]->data.position.y = 0.75f;
    Text[0]->LoadVBO();

    Floor = 1;
    startFloor();

    EffectEntity = new Entity(ShaderIndex_Effect, &identityMatrix, &identityMatrix);
    EffectEntity->data.scale = glm::vec3(2, 2, 1);
    EffectEntity->updated = true; EffectEntity->calculated = false;
    EffectEntity->RecalculateMM();
    EffectEntity->LoadVBO();
}

void DrawGame() {
    if (state != GameState::GAME) { return; }
    glBindFramebuffer(GL_FRAMEBUFFER, fbos[fbo_diffuse].first);
    glViewport(0, 0, 1920, 1080);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    enableAttributes(ShaderIndex_Texture);
    glUniform1i(ShaderPrograms[ShaderIndex_Texture].extra[ShaderIndex_Texture_Text], textures[TextureIndexOutline]);
    //glUniform1i(ShaderPrograms[ShaderIndex_Texture].extra[ShaderIndex_Texture_Text], textures[TextureIndexLight]);
    //glUniform1i(ShaderPrograms[ShaderIndex_Texture].extra[ShaderIndex_Texture_Text], textures[textLoader.textID]);
    setColor(glm::vec3(1, 1, 1));
    //for (Entity* e : Player) { loadAndDraw(e); }
    loadAndDraw(Player[0]); loadAndDraw(Player[1]);
    setColor(glm::vec3(1, 0, 0));
    for (Entity* e : Enemies) { loadAndDraw(e); }
    setColor(glm::vec3(1, 1, 1));
    for (Entity* e : Entities) { loadAndDraw(e); }
    glUniform1i(ShaderPrograms[ShaderIndex_Texture].extra[ShaderIndex_Texture_Text], textures[textLoader.textID]);
    setColor(glm::vec3(0, 1, 0));
    for (Entity* e : Text) { loadAndDraw(e); }
    setColor(glm::vec3(1, 1, 1));
    glUniform1i(ShaderPrograms[ShaderIndex_Texture].extra[ShaderIndex_Texture_Text], textures[TextureIndexLight]);
    PPDraw();

    glBindFramebuffer(GL_FRAMEBUFFER, fbos[fbo_light].first);
    glViewport(0, 0, 1920, 1080);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUniform1i(ShaderPrograms[ShaderIndex_Texture].extra[ShaderIndex_Texture_Text], textures[textLoader.textID]);
    for (Entity* e : Text) { loadAndDraw(e); }
    glUniform1i(ShaderPrograms[ShaderIndex_Texture].extra[ShaderIndex_Texture_Text], textures[TextureIndexLight]);
    PPLight(16.0f);
    glUniform1i(ShaderPrograms[ShaderIndex_Texture].extra[ShaderIndex_Texture_Text], textures[TextureIndexEcho]);
    loadAndDraw(Player[2]);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    WindowReset();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    enableAttributes(ShaderIndex_Effect);
    glUniform1i(ShaderPrograms[ShaderIndex_Effect].extra[ShaderIndex_Effect_Text1], textures[fbos[fbo_diffuse].second]);
    glUniform1i(ShaderPrograms[ShaderIndex_Effect].extra[ShaderIndex_Effect_Text2], textures[fbos[fbo_light].second]);
    glDepthMask(GL_FALSE);
    loadAndDraw(EffectEntity);
    glDepthMask(GL_TRUE);

    disableAttributes();
    SDL_GL_SwapWindow(DisplayWindow);

    for (Entity* e : Player) {
        e->calculated = false;
        e->updated = false;
    }
    for (Entity* e : Enemies) {
        e->calculated = false;
        e->updated = false;
    }

    if (Player.size() > 0 && (int)Player[0]->data.position.x == Cave.end.x && (int)Player[0]->data.position.y == Cave.end.y) {
        startFloor();
    }
}

void UpdateGame() {
    reinit = false;
    if (Player[0]->extraData[0] <= 0) {
        state = GameState::END;
        delete EffectEntity;
        EffectEntity = nullptr;
        return;
    }
    int x, y;
    glm::vec2 pos;
    if ((SDL_GetMouseState(&x, &y)&SDL_BUTTON_LMASK) && attackTick < 0) {
        attackTick = attackTime;
        PlaySound(audioPlayerAttack, false, 10);
    }
    Player[0]->updated = true;
    pos = WindowPos(x, y) + glm::vec2(-0.5f, -0.5f);
    Player[0]->data.angle = -glm::degrees(glm::atan<float>(pos.y, pos.x)) - 90.0f;

    Player[0]->data.velocity.x = Player[0]->data.velocity.y = 0.0f;
    if (Keys[SDL_SCANCODE_W]) { Player[0]->data.velocity.y += 1.0f; }
    if (Keys[SDL_SCANCODE_S]) { Player[0]->data.velocity.y -= 1.0f; }
    if (Keys[SDL_SCANCODE_A]) { Player[0]->data.velocity.x -= 1.0f; }
    if (Keys[SDL_SCANCODE_D]) { Player[0]->data.velocity.x += 1.0f; }
    Player[0]->data.velocity *= (playerSpeed / Player[0]->data.velocity.length());

    if (Keys[SDL_SCANCODE_SPACE] && Player[2]->extraData[0] < 0) {
        Player[2]->extraData[0] = 0;
        Player[2]->data.position = Player[0]->data.position;
        Player[2]->updated = true; Player[2]->calculated = false;
    }
}

glm::ivec2 deltaCoords[8] = {
    glm::ivec2(1,0),glm::ivec2(1,1),glm::ivec2(0,1),glm::ivec2(-1,1),
    glm::ivec2(-1,0),glm::ivec2(-1,-1),glm::ivec2(0,-1),glm::ivec2(1,-1)
};

void randomWander(Entity* e, int dir = -1) {
    std::queue<int> tmp;
    e->extraData[1] = Wander;
    glm::ivec2 pos = glm::ivec2(e->data.position.x, e->data.position.y);
    for (int i = 0; i < 8; ++i) {
        if (Cave.map[(pos + deltaCoords[i]).y][(pos + deltaCoords[i]).x] != 0) {
            tmp.push(i);
            if (dir != -1 && (i == (dir + 1) % 8 || i == dir || (i + 1) % 8 == dir)) {
                tmp.push(i);
            }
        }
    }
    e->extraData[2] = rand() % tmp.size();
    for (int i = 0; !tmp.empty(); ++i) {
        if (i == e->extraData[2]) {
            e->extraData[3] = pos.x + deltaCoords[tmp.front()].x;
            e->extraData[4] = pos.y + deltaCoords[tmp.front()].y;
            e->data.velocity = glm::normalize(glm::vec3(e->extraData[3] + 0.5f - e->data.position.x, e->extraData[4] + 0.5f - e->data.position.y, 0))*2.0f;
        }
        tmp.pop();
    }
    e->extraData[5] = wanderTick;
}

//line tracing algorithm. lazy and could be optimized
bool existPath(Entity* e, const glm::ivec2& pos) {
    glm::vec2 dir = glm::vec2(pos.x - e->data.position.x, pos.y - e->data.position.y);
    //std::cout << dir.x << " " << dir.y << std::endl;
    glm::ivec2 curr;
    size_t count = 2 * (glm::abs<int>((int)dir.x) + glm::abs<int>((int)dir.y));
    if (count == 0) { return true; }
    dir /= count;
    //std::cout << dir.x << " " << dir.y << std::endl;
    for (int i = 0; i < count; ++i) {
        curr = glm::ivec2(dir.x*i + e->data.position.x, dir.y*i + e->data.position.y);
        //std::cout << curr.x << " " << curr.y << " " << Cave.map[curr.y][curr.x] << std::endl;
        if (Cave.map[curr.y][curr.x] == 0) { return false; }
    }
    return true;

    /*
    if (dir.x > 0) { axisdir.x = 1; }
    if (dir.x < 0) { axisdir.x = -1; }
    if (dir.y > 0) { axisdir.y = 1; }
    if (dir.y < 0) { axisdir.y = -1; }
    if (axisdir.x == 0) {
        for (; (int)curr.y != pos.y; curr.y += axisdir.y) {
            if (Cave.map[(int)curr.y][(int)curr.x] == 0) { return false; }
        }
        return true;
    }
    if (axisdir.y == 0) {
        for (; (int)curr.x != pos.x; curr.x += axisdir.x) {
            if (Cave.map[(int)curr.y][(int)curr.x] == 0) { return false; }
        }
        return true;
    }

    glm::vec2 cmp;
    while ((glm::ivec2)curr != pos) {
        if (Cave.map[(int)curr.y][(int)curr.x] == 0) { return false; }
        
        cmp.x = (axisdir.x)*((axisdir.x > 0) ? ((int)curr.x) + 1 - curr.x : curr.x - ((int)curr.x)) / dir.x;
        cmp.y = (axisdir.y)*((axisdir.y > 0) ? ((int)curr.y) + 1 - curr.y : curr.y - ((int)curr.y)) / dir.y;
        //std::cout << cmp.x << " " << cmp.y << std::endl;

        if (cmp.x < cmp.y) {
            curr.x += (cmp.x + 0.1f)*dir.x;
            curr.y += (cmp.x + 0.1f)*dir.y;
        }
        else {
            curr.x += (cmp.y + 0.1f)*dir.x;
            curr.y += (cmp.y + 0.1f)*dir.y;
        }
    }
    return true;*/
}

void FixedUpdateGame() {
    if (reinit) { return; }
    attackAnimation();

    for (Entity* e : Player) {
        e->data.position += e->data.velocity*FIXEDTICK;
        e->updated = true;
    }

    MapEntityCollision(Player[0], playerRadius);
    //viewMatrix = glm::translate(glm::scale(identityMatrix, glm::vec3(0.25f, 0.25f, 1.0f)), -Player[0]->data.position);
    //viewMatrix = glm::translate(glm::scale(identityMatrix, glm::vec3(0.03125f, 0.03125f, 1.0f)), -Player[0]->data.position);
    viewMatrix = glm::translate(glm::scale(identityMatrix, glm::vec3(0.0625f, 0.0625f, 1.0f)), -Player[0]->data.position);

    ++mobIncrementTimer;
    ++mobTimer;
    if (mobIncrementTimer > 1024) {
        mobIncrementTimer = 0;
        ++mobcount;
    }
    
    if (mobTimer >= 128 && mobcount > Enemies.size()) {//spawn mobs over time
        mobTimer = 0;
        glm::ivec2 pos = glm::ivec2(randRange(1, Cave.width - 2), randRange(1, Cave.height - 2));
        if (Cave.map[pos.y][pos.x] != 0 && !(glm::abs(Player[0]->data.position.x - pos.x) < 8.0f && glm::abs(Player[0]->data.position.y - pos.y) < 8.0f)) {
            size_t index = Enemies.size();
            Enemies.push_back(new Entity(ShaderIndex_Texture, &projectionMatrix, &viewMatrix));
            Enemies[index]->data.position = glm::vec3(pos.x + 0.5f, pos.y + 0.5f, 0);
            Enemies[index]->calculateUVs(8, 8, 8);
            Enemies[index]->LoadVBO();
            Enemies[index]->updated = true; Enemies[index]->calculated = false;
            
            Enemies[index]->extraData.resize(6);
            Enemies[index]->extraData[0] = glm::min<int>(4, Floor);
            randomWander(Enemies[index]);
        }
    }

    glm::ivec2 tmpPPos = glm::ivec2(Player[0]->data.position.x, Player[0]->data.position.y);
    for (Entity* e : Enemies) {
        if (e->data.velocity != glm::vec3(0.0f, 0.0f, 0.0f)) {
            glm::vec3 tmpV;
            float len;
            tmpV = Player[0]->data.position - e->data.position;
            len = glm::sqrt(tmpV.x*tmpV.x + tmpV.y*tmpV.y);
            if (len < (playerRadius + enemyRadius) && Player[0]->extraData[1] <= 0) {
                --Player[0]->extraData[0];
                Player[0]->extraData[1] = shieldTick;
                Player[0]->extraData[2] = regenTick;
            }
            switch (e->extraData[1]) {
            case Wander:
                if (len < 5.0f && existPath(e,tmpPPos)) {
                    e->extraData[1] = Chase;
                    e->extraData[2] = -1;
                    e->extraData[3] = tmpPPos.x;
                    e->extraData[4] = tmpPPos.y;
                    e->data.velocity = tmpV * 6.0f / len;
                }
                else {
                    if ((--e->extraData[5] <= 0) || ((int)e->data.position.x == e->extraData[3] && (int)e->data.position.y == e->extraData[4])) { randomWander(e, e->extraData[2]); }
                    else { e->data.position += e->data.velocity*FIXEDTICK; e->updated = true; }
                }
                break;
            case Chase:
                if (len < 5.0f && existPath(e, tmpPPos)) {
                    e->extraData[1] = Chase;
                    e->extraData[3] = tmpPPos.x;
                    e->extraData[4] = tmpPPos.y;
                    e->data.velocity = tmpV * 6.0f / len;
                }
                else {
                    //std::cout << "no chase" << std::endl;
                    e->extraData[1] = Wander;
                    e->extraData[5] = wanderTick;
                }
                e->data.position += e->data.velocity*FIXEDTICK; e->updated = true;
                break;
            case Stunned:
                if (--e->extraData[2] <= 0) {
                    randomWander(e);
                }
                break;
            }
            MapEntityCollision(e, enemyRadius);
        }
    }

    if (Player[0]->extraData[1] >= 0) {
        if (--Player[0]->extraData[1] > shieldTick * 3 / 4) {//screen shake
            glm::vec3 shake = glm::vec3(rand() % 1024 - 512, rand() % 1024 - 512, 0)*(1 / 512.0f);
            viewMatrix = glm::translate(glm::scale(identityMatrix, glm::vec3(0.25f, 0.25f, 1.0f)), -Player[0]->data.position + shake);
        }
    }
    if (--Player[0]->extraData[2] <= 0) {
        if (Player[0]->extraData[0] < 10) {
            ++Player[0]->extraData[0];
            Player[0]->extraData[2] = regenTick;
        }
    }

    textLoader.calculateBoundedText(Text[0], "HP " + std::to_string(Player[0]->extraData[0]), 0.125f, -0.125, 0.125);
    Text[0]->data.position.y = 0.75f;
    Text[0]->LoadVBO();

    particleStatics[0].pos = Player[0]->data.position;

    if (Player[2]->extraData[0] > echoTicks) {
        Player[2]->extraData[0] = -1;
        Player[2]->data.scale = glm::vec3(0, 0, 0);
        Player[2]->updated = true;
    }

    if (Player[2]->extraData[0] >= 0) {
        ++Player[2]->extraData[0];
        Player[2]->data.scale = glm::vec3(1, 1, 1)*(Player[2]->extraData[0] * echoSpeed*FIXEDTICK);
        Player[2]->updated = true;
    }

    PPUpdate();
}