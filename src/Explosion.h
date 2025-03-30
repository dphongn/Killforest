#ifndef EXPLOSION_H
#define EXPLOSION_H
#include <SDL.h>
#include <vector>

struct Explosion {
    int x, y;
    Uint32 startTime;
    bool active;
};
void addExplosion(int x, int y, int EXPLOSION_FRAME_WIDTH, int EXPLOSION_FRAME_HEIGHT, std::vector<Explosion>& explosions);
#endif // EXPLOSION_H