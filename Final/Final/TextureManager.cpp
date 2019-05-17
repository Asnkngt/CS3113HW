#include "TextureManager.h"
#define STB_IMAGE_IMPLEMENTATION 
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"
#include "stb_truetype.h"

std::vector<std::pair<GLuint, size_t>> fbos;
std::vector<GLuint> textures;

TTFText::TTFText(const std::string& filepath) :filepath(filepath) {}
TTFText::~TTFText() {}

bool TTFText::LoadFont(int shift)
{
    std::ifstream bmp, data;
    bmp.open(filepath + ".bmp", std::ifstream::in);
    data.open(filepath + ".data", std::ifstream::in);
    if (!bmp.is_open()||!data.is_open()) {
        bmp.close(); data.close();
        std::cout << "Cannot find " << filepath << " information. Attempting to generate from ttf file" << std::endl;
        return GenFont(shift);
    }
    
    for (int i = 0; i < 256; ++i) {
        data >> chardata[i].x >> chardata[i].y >> chardata[i].z >> chardata[i].w;
    }
    textID = LoadTexture((filepath + ".bmp").c_str(), TextureFormat::G());
    std::cout << "Information loaded." << std::endl;
    bmp.close();
    data.close();
    return true;
}

bool TTFText::GenFont(int shift) {
    stbtt_fontinfo font;
    unsigned char * bitmap, *bitmapFinal;
    int w, h, s, tmp;
    char* ttf_buffer;

    //load in ttf file into ttf_buffer
    std::ifstream fontFile;
    fontFile.open((filepath + ".ttf").c_str(), std::ifstream::in);
    if (!fontFile) {
        std::cout << "Cannot find " << filepath << ".ttf" << std::endl;
        return false;
    }
    fontFile.seekg(0, fontFile.end);
    std::streamoff ffLen = fontFile.tellg();
    ttf_buffer = new char[(size_t)ffLen];
    fontFile.seekg(0, fontFile.beg);
    fontFile.read(ttf_buffer, ffLen);
    fontFile.close();

    //initialize values
    tmp = 256 << shift;
    w = h = tmp;
    s = 16 << shift;
    bitmapFinal = new unsigned char[w*h];
    for (int i = 0; i < w * h; ++i) {
        bitmapFinal[i] = 0;
    }
    //load each inidivdual char into bitmap and copy into the output bitmap
    //store the string widths and heights to parse when loading. Garuenteed to have 256 * 2 values so parsing is easier

    //Used to get individual uv heights and widths
    float invSide = 1.0f / tmp;
    //Used to get individual spacing and y shift
    float invShift = 1.0f / s;
    
    stbtt_InitFont(&font, (const unsigned char*)ttf_buffer, stbtt_GetFontOffsetForIndex((const unsigned char*)ttf_buffer, 0));
    std::ofstream bmpdata;
    bmpdata.open(filepath + ".data", std::ofstream::out);
    for (int k = 0; k < 256; ++k) {
        int xoff, yoff;
        bitmap = stbtt_GetCodepointBitmap(&font, 0, stbtt_ScaleForPixelHeight(&font, (float)s), k, &w, &h, &xoff, &yoff);
        int kshift = ((k / 16) * tmp + (k % 16)) * s;
        for (int j = 0; j < h; ++j) {
            for (int i = 0; i < w; ++i) {
                //default out of bounds to 0
                bitmapFinal[kshift + j * tmp + i] = (j < h || i < w) ? bitmap[j*w + i] : '0x0';
            }
        }
        chardata[k].x = invSide * w;
        chardata[k].y = invSide * h;
        chardata[k].z = invShift * xoff;
        chardata[k].w = invShift * yoff;
        //std::cout <<(char)k<<' '<< w << ' ' << h << ' ' << xoff << ' ' << yoff << std::endl;
        if (k == ' ') {
            chardata[k].x = 1.0f / 32.0f;
        }
        bmpdata << std::fixed << std::setprecision(10) << chardata[k].x << ' ' << chardata[k].y << ' ' << chardata[k].z << ' ' << chardata[k].w << std::endl;
    }
    bmpdata.close();
    //works suprisingly well

    w = h = tmp;
    stbi_write_bmp((filepath + ".bmp").c_str(), w, h, 1, bitmapFinal);
    
    textID = LoadTexture((filepath + ".bmp").c_str(), TextureFormat::G());
    std::cout << "Information loaded." << std::endl;

    delete[] bitmap;
    delete[] bitmapFinal;
    delete[] ttf_buffer;
    return true;
}

void TTFText::calculateBoundedText(Entity & e, const std::string& input, float height, float left, float right)
{
    //quad per char
    e.resize(input.length() * 2);
    //total space string will take up. used to calculate the scale in the x-direction
    float total = 0.0f;
    for (char c : input) {
        total += chardata[c].x;
    }
    float stretch = (right - left) / total;
    total = 0.0f;
    
    size_t tmp;
    for (size_t i = 0; i < input.length();++i) {
        tmp = i * 6;
        e.verts[tmp + 0].x = e.verts[tmp + 3].x = e.verts[tmp + 5].x = left + total + chardata[input[i]].x*stretch;//right x
        e.verts[tmp + 1].x = e.verts[tmp + 2].x = e.verts[tmp + 4].x = left + total;//left x
        e.verts[tmp + 0].y = e.verts[tmp + 1].y = e.verts[tmp + 3].y = height * (-chardata[input[i]].w);//top
        e.verts[tmp + 2].y = e.verts[tmp + 4].y = e.verts[tmp + 5].y = height * (-1.0f - chardata[input[i]].w);//bottom
        
        e.uvs[tmp + 0].x = e.uvs[tmp + 3].x = e.uvs[tmp + 5].x = ((input[i]) % 16) / 16.0f + chardata[input[i]].x;
        e.uvs[tmp + 1].x = e.uvs[tmp + 2].x = e.uvs[tmp + 4].x = ((input[i]) % 16) / 16.0f;
        e.uvs[tmp + 0].y = e.uvs[tmp + 1].y = e.uvs[tmp + 3].y = ((input[i]) / 16) / 16.0f;
        e.uvs[tmp + 2].y = e.uvs[tmp + 4].y = e.uvs[tmp + 5].y = ((input[i]) / 16 + 1) / 16.0f;
        
        total += chardata[input[i]].x*stretch;
    }
    e.LoadVBO();
}

void TTFText::calculateBoundedText(Entity * e, const std::string & input, float height, float left, float right){
    calculateBoundedText(*e, input, height, left, right);
}

int TextureFormat::G() { return STBI_grey; }
int TextureFormat::GA() { return STBI_grey_alpha; }
int TextureFormat::RGB() { return STBI_rgb; }
int TextureFormat::RGBA() { return STBI_rgb_alpha; }

size_t LoadTexture(const char * filepath, int textureformat, GLint paramMin, GLint paramMag) {
    int w, h, comp;
    unsigned char* image = stbi_load(filepath, &w, &h, &comp, 4);
    if (image == NULL) {
        std::cout << "Unable to load image. Make sure the path is correct\n";
        assert(false);
    }
    GLuint tex;
    glGenTextures(1, &tex);
    glActiveTexture(GL_TEXTURE0 + textures.size());
    glBindTexture(GL_TEXTURE_2D, tex);
    textures.push_back(0 + textures.size());//probably try and figure out some way to use the space better

    int counter = 0;
    switch (textureformat) {
    case STBI_grey:
        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                image[counter + 3] = image[counter + 0];
                counter += 4;
            }
        }
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
        break;
    case STBI_grey_alpha:
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
        break;
    case STBI_rgb:
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
        break;
    case STBI_rgb_alpha:
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
        break;
    }
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, paramMin);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, paramMag);

    stbi_image_free(image);

    return textures.size() - 1;
}

size_t GenerateFBO(int width, int height) {
    size_t index = fbos.size();
    fbos.push_back(std::pair<GLuint, size_t>(-1, textures.size()));
    glGenFramebuffers(1, &fbos[index].first);
    glBindFramebuffer(GL_FRAMEBUFFER, fbos[index].first);

    GLuint texture;
    glGenTextures(1, &texture);
    glActiveTexture(GL_TEXTURE0 + textures.size());
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    textures.push_back(0 + textures.size());

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return index;
}
