#include "Map.h"

void Map::paramCheck(int & startX, int & startY, int & endX, int & endY) {
    if (startX < 0) { startX = 0; }
    if (startX > width) { startX = width; }
    if (endX < 0) { endX = 0; }
    if (endX > width) { endX = width; }
    if (startY < 0) { startY = 0; }
    if (startY > height) { startY = height; }
    if (endY < 0) { endY = 0; }
    if (endY > height) { endY = height; }
}

Map::Map(int w, int h, int p) :width(w), height(h), pass(p), map(height, std::vector<int>(width, 0)) {}

Map::~Map() {}

bool Map::inBound(const int & x, const int & y) { return (x >= 0 && x < width) && (y >= 0 && y < height); }

bool Map::inBound(const glm::ivec2 & pos) { return inBound(pos.x, pos.y); }

void Map::Load() {
    Load(0, 0, width, height);
}

void Map::Load(int startX, int startY, int endX, int endY) {
    paramCheck(startX, startY, endX, endY);
    if (startX > endX || startY > endY) {
        Load();
        return;
    }

    srand((unsigned int)time(nullptr));
    for (int y = startY; y < endY; ++y) {
        for (int x = startX; x < endX; ++x) {
            map[y][x] = (x != 0 && x != width - 1 && y != 0 && y != height - 1 && rand() % 100 > pass) ? 1 : 0;//0 is wall
        }
    }
    
    //loadRandomWalk(startX, startY, endX, endY);
}

void Map::loadRandomWalk(int startX, int startY, int endX, int endY)
{
    int xi, yi;

    int len = 0;
    start.x = xi = randRange(startX + 2, endX - 2);
    start.y = yi = randRange(startY + 2, endY - 2);
    int area = (endX - startX) * (endY - startY);
    while (len < (int)(glm::sqrt(area)*(glm::log(area) / 2))) {
        end.x = randRange(startX + 2, endX - 2);
        end.y = randRange(startY + 2, endY - 2);
        len += glm::abs<int>(end.x - xi) + glm::abs<int>(end.y - yi);
        randomWalk(xi, yi, end.x, end.y);
        xi = end.x; yi = end.y;
    }
    map[start.y][start.x] = map[end.y][end.x] = 5;
}

void Map::randomWalk(int startX, int startY, int endX, int endY) {
    paramCheck(startX, startY, endX, endY);
    int dX, dY;

    map[startY - 1][startX - 1] = map[startY - 1][startX + 0] = map[startY - 1][startX + 1] = map[startY + 0][startX - 1] = map[startY + 0][startX + 0] = map[startY + 0][startX + 1] = map[startY + 1][startX - 1] = map[startY + 1][startX + 0] = map[startY + 1][startX + 1] = 1;

    while (startX != endX && startY != endY) {
        dX = glm::abs<int>(endX - startX);
        dY = glm::abs<int>(endY - startY);
        if (rand() % (dX + dY) < dX) { startX += (endX - startX) / dX; }
        else { startY += (endY - startY) / dX; }
        map[startY - 1][startX - 1] = map[startY - 1][startX + 0] = map[startY - 1][startX + 1] = map[startY + 0][startX - 1] = map[startY + 0][startX + 0] = map[startY + 0][startX + 1] = map[startY + 1][startX - 1] = map[startY + 1][startX + 0] = map[startY + 1][startX + 1] = 1;
    }
}

int Map::automata() {
    return automata(1, 1, width - 1, height - 1);
}

//make sure to bound in 1,width-1 and 1,height-1
int Map::automata(int startX, int startY, int endX, int endY) {

    int delta = 0;
    for (int y = startY; y < endY; ++y) {
        for (int x = startX; x < endX; ++x) {
            if (x == startX || x == endX - 1 || y == startY || y == endY - 1) {
                map[y][x] = 0;
            }
            else {
                int val =
                    map[y - 1][x - 1] + map[y - 1][x + 0] + map[y - 1][x + 1] +
                    map[y + 0][x - 1] + map[y + 0][x + 0] + map[y + 0][x + 1] +
                    map[y + 1][x - 1] + map[y + 1][x + 0] + map[y + 1][x + 1] -
                    ((map[y][x] > 0) ? 1 : 0);
                if (val > 4) {
                    delta += 1 - map[y][x];
                    map[y][x] = (map[y][x] > 1) ? map[y][x] : 1;
                }
                if (val < 4) {
                    delta += map[y][x];
                    map[y][x] = 0;
                }
            }
        }
    }
    return delta;
}

glm::ivec2 dirs[4] = {
    glm::ivec2(0,1),
    glm::ivec2(1,0),
    glm::ivec2(0,-1),
    glm::ivec2(-1,0)
};

void Map::lazyPath(glm::ivec2 pos, std::vector<std::vector<int>>& conmap) {
    std::queue<glm::ivec2> path,choices;
    glm::ivec2 currsum;
    path.push(pos);
    int val = conmap[pos.y][pos.x] - 1;
    do {
        for (int i = 0; i < 4; ++i) {
            currsum = pos + dirs[i];
            if (inBound(currsum) && conmap[currsum.y][currsum.x] == val) {
                choices.push(currsum);
            }
        }
        int pops = rand() % choices.size();
        for (int i = 0; i < 4 && !choices.empty(); ++i) {
            if (i == pops) {
                pos = choices.front();
            }
            choices.pop();
        }
        --val;
        path.push(pos);
    } while (map[pos.y][pos.x] != 1);
    while (!path.empty()) {
        pos = path.front();
        path.pop();
        currsum = pos;
        if (inBound(currsum)) { map[currsum.y][currsum.x] = 1; };
        for (int i = 0; i < 4; ++i) {
            currsum = pos + dirs[i];
            if (inBound(currsum)) { map[currsum.y][currsum.x] = 1; }
        }
    }
}

void Map::lazyRegion(std::queue<glm::ivec2>& next, std::queue<glm::ivec2>& region, std::vector<std::vector<int>>& conmap){
    glm::ivec2 curr, currsum;
    int val = conmap[region.front().y][region.front().x];
    while (!region.empty()) {
        curr = region.front();
        region.pop();
        for (int i = 0; i < 4; ++i) {
            currsum = curr + dirs[i];
            if (inBound(currsum) && conmap[currsum.y][currsum.x] == 0) {
                if (map[currsum.y][currsum.x] == 1) {
                        region.push(currsum);
                        conmap[currsum.y][currsum.x] = val;
                }
                else {
                    next.push(currsum);
                    conmap[currsum.y][currsum.x] = val + 1;
                }
            }
        }
    }
}

void Map::lazyConnect(){
    std::queue<glm::ivec2> next, region;
    std::vector<std::vector<int>> conmap(height, std::vector<int>(width, 0));

    int wroot = (int)glm::sqrt(width), hroot = (int)glm::sqrt(height);

    start.x = start.y = 0;
    while (map[start.y][start.x] == 0 || conmap[start.y][start.x] != 0) {
        start.x = randRange(2, width - 2);
        start.y = randRange(2, height - 2);
    }
    region.push(start);
    end = start;
    conmap[start.y][start.x] = 1;

    lazyRegion(next, region, conmap);

    glm::ivec2 curr, currsum;
    int val;
    while (!next.empty()) {
        curr = next.front();
        next.pop();
        for (int i = 0; i < 4; ++i) {
            currsum = curr + dirs[i];
            val = conmap[curr.y][curr.x];
            if (inBound(currsum) && conmap[currsum.y][currsum.x] == 0) {
                conmap[currsum.y][currsum.x] = val + 1;
                if (map[currsum.y][currsum.x] == 0) {
                    next.push(currsum);
                }
                else {
                    end = currsum;
                    region.push(currsum);
                    lazyPath(currsum, conmap);
                    lazyRegion(next, region, conmap);
                }
            }
        }
    }
    while (next.empty()) {
        for (int y = -hroot; y <= hroot; ++y) {
            for (int x = -wroot; x <= wroot; ++x) {
                currsum = end + glm::ivec2(x, y);
                if (inBound(currsum) && map[currsum.y][currsum.x] == 1) {
                    next.push(currsum);
                }
            }
        }
        hroot *= 2;
        wroot *= 2;
    }
    int tmp = rand() % next.size();
    while (!next.empty()) {
        if (--tmp == -1) {
            end = next.front();
        }
        next.pop();
    }
    //map[end.y][end.x] = 5;
}

std::ostream & operator<<(std::ostream & os, Map & m)
{
    for (int y = 0; y < m.height; ++y) {
        for (int x = 0; x < m.width; ++x) {
            if (m.map[y][x] == 5) { os << "X"; }
            if (m.map[y][x] == 0) { os << " "; }
            if (m.map[y][x] == 1) { os << "'"; }
        }
        os << '\n';
    }
    return os;
}
