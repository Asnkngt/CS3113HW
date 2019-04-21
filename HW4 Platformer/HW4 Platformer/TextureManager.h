#pragma once
#include "Headers.h"
#include "Entity.h"

class TTFText
{
public:
    GLuint textID;

    //filepath not including .ttf ending
    TTFText(const std::string& filepath = "");
    ~TTFText();

    //shift is used if trying to load font from ttf file if it does not exist
    bool LoadFont(int shift = 1);
    //can manually generate if needed. Handles generation and loading in one call
    bool GenFont(int shift = 1);
    
    /*
    Note to self:
    verticies generated will not be centered on center of the object. Its not needed for collision anyways so it doesn't matter too much
    Bounded is centered on bottom left
    LeftBounded is centered on bottom left
    RightBounded is centered on bottom right
    Also, the bottom of characters is on the line y=0, not necessarily the quad itself.
    */
    //insert left as a negative value with respect to the object's center
    void calculateBoundedText(Entity& e, const std::string& input, float height, float left, float right);

private:
    std::string filepath;
    //stores the char's length, height, spacing and y shift
    glm::vec4 chardata[256];
    
};

class TextureFormat
{
public:
    static int G();
    static int GA();
    static int RGB();
    static int RGBA();
};
//Use Textureformat class to choose int to pass in
GLuint LoadTexture(const char * filepath, int textureformat, GLint paramMin = GL_NEAREST, GLint paramMag = GL_NEAREST);
