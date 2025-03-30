#ifndef TANK_H
#define TANK_H
#include <SDL_image.h>
#include <SDL.h>
// #include <SDL.h>
#include <vector>
// #include "Bullet.h"

struct Tank {
    int x, y;
    int speed;
    int direction;
    bool active;
    int range;
    Uint32 lastShotTime;

    int currentFrame;
    Uint32 lastFrameTime;
    bool isMoving;
    bool isShooting;
    Uint32 shootingStartTime;
    
    // void move(int newX, int newY);
    // void shoot(std::vector<Bullet>& bullets, bool isPlayer);
};
void Initialize_playerTank(Tank &playerTank, int SCREEN_WIDTH, int SCREEN_HEIGHT, int TANK_SIZE);
//asdjnadjns
#endif // TANK_H