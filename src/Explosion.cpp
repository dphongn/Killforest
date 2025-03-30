#include "Explosion.h"
#include <SDL.h>
#include <vector>
void addExplosion(int x, int y, int EXPLOSION_FRAME_WIDTH, int EXPLOSION_FRAME_HEIGHT, std::vector<Explosion>& explosions) {
    Explosion exp;
    exp.x = x - EXPLOSION_FRAME_WIDTH/2;
    exp.y = y - EXPLOSION_FRAME_HEIGHT/2;
    exp.startTime = SDL_GetTicks();
    exp.active = true;
    explosions.push_back(exp);
}