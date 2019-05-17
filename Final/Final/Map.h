#pragma once
#include "Headers.h"
class Map
{
private:
    void paramCheck(int& startX, int& startY, int& endX, int& endY);

public:
    friend std::ostream& operator<<(std::ostream& os, Map& m);
    Map(int width = 64, int height = 64, int pass = 50);
    ~Map();

    bool inBound(const int& x, const int& y);
    bool inBound(const glm::ivec2& pos);

    void Load();
    void Load(int startX, int startY, int endX, int endY);
    void loadRandomWalk(int startX, int startY, int endX, int endY);
    void randomWalk(int startX, int startY, int endX, int endY);
    
    int automata();
    int automata(int startX, int startY, int endX, int endY);
    
    //Suprisingly works
    void lazyPath(glm::ivec2 pos, std::vector<std::vector<int>>& conmap);
    void lazyRegion(std::queue<glm::ivec2>& next, std::queue<glm::ivec2>& region, std::vector<std::vector<int>>& conmap);
    void lazyConnect();

    int width;
    int height;
    int pass;
    glm::ivec2 start;
    glm::ivec2 end;

    std::vector<std::vector<int>> map;

};

