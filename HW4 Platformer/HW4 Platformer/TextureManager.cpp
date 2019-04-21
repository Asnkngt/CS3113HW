/*
In progress script
Currently supposts the bare minimum needed for rendering text but will require a bunch of improvements over this semester

*/
#include "TextureManager.h"
#define STB_IMAGE_IMPLEMENTATION 
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"
#include "stb_truetype.h"


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
    ttf_buffer = new char[ffLen];
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

    //compact but ugly
    //stbtt_bakedchar cdata[256];
    //stbtt_BakeFontBitmap((const unsigned char*)ttf_buffer, 0, s, bitmapFinal, w, h, 0, 255, cdata);

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
    e.vertCount = input.length() * 6;
    delete[] e.verts;
    delete[] e.uvs;
    e.verts = new float[input.length() * 12];
    e.uvs = new float[input.length() * 12];
    //std::cout << e.vertCount << std::endl;
    //total space string will take up. used to calculate the scale in the x-direction
    float total = 0.0f;
    for (char c : input) {
        //std::cout << (int)c << ' ' << chardata[c].x << ' ' << chardata[c].y << ' ' << chardata[c].z << ' ' << chardata[c].w << std::endl;
        total += chardata[c].x;
    }
    float stretch = (right - left) / total;
    total = 0.0f;
    
    for (size_t i = 0; i < input.length();++i) {
        //std::cout << total << std::endl;
        e.verts[i * 12] = e.verts[i * 12 + 6] = e.verts[i * 12 + 10] = left + total + chardata[input[i]].x*stretch;//right x
        e.verts[i * 12 + 2] = e.verts[i * 12 + 4] = e.verts[i * 12 + 8] = left + total;//left x
        e.verts[i * 12 + 1] = e.verts[i * 12 + 3] = e.verts[i * 12 + 7] = height * (-chardata[input[i]].w);//top
        e.verts[i * 12 + 5] = e.verts[i * 12 + 9] = e.verts[i * 12 + 11] = height * (-1.0f - chardata[input[i]].w);//bottom
        //std::cout << chardata[input[i]].w << ' ' << 1.0f + chardata[input[i]].w << ' ' << 1.0f - chardata[input[i]].w << std::endl;
        
        e.uvs[i * 12] = e.uvs[i * 12 + 6] = e.uvs[i * 12 + 10] = ((input[i]) % 16) / 16.0f + chardata[input[i]].x;
        e.uvs[i * 12 + 2] = e.uvs[i * 12 + 4] = e.uvs[i * 12 + 8] = ((input[i]) % 16) / 16.0f;
        e.uvs[i * 12 + 1] = e.uvs[i * 12 + 3] = e.uvs[i * 12 + 7] = ((input[i]) / 16) / 16.0f;
        e.uvs[i * 12 + 5] = e.uvs[i * 12 + 9] = e.uvs[i * 12 + 11] = ((input[i]) / 16 + 1) / 16.0f;
        
        total += chardata[input[i]].x*stretch;
    }
}

int TextureFormat::G() { return STBI_grey; }
int TextureFormat::GA() { return STBI_grey_alpha; }
int TextureFormat::RGB() { return STBI_rgb; }
int TextureFormat::RGBA() { return STBI_rgb_alpha; }

GLuint LoadTexture(const char * filepath, int textureformat, GLint paramMin, GLint paramMag)
{
    int w, h, comp;
    unsigned char* image = stbi_load(filepath, &w, &h, &comp, textureformat);
    if (image == NULL) {
        std::cout << "Unable to load image. Make sure the path is correct\n";
        assert(false);
    }
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    switch (textureformat)
    {
    case STBI_grey:
        //will need to write my own shader for using this
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, image);
        break;
    case STBI_grey_alpha:
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RG, w, h, 0, GL_RG, GL_UNSIGNED_BYTE, image);
        break;
    case STBI_rgb:
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
        break;
    case STBI_rgb_alpha:
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
        break;
    }
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, paramMin);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, paramMag);

    stbi_image_free(image);

    return tex;
}
