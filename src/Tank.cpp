#include "Tank.h"
#include <iostream>
#include <SDL.h>
#include <SDL_image.h>

void Initialize_playerTank(Tank &playerTank, int SCREEN_WIDTH, int SCREEN_HEIGHT, int TANK_SIZE){
    playerTank.speed = 1;
    playerTank.direction = 2;
    playerTank.active = true;
    playerTank.lastShotTime = 0;
    playerTank.range = -1;
    playerTank.x = 320;
    playerTank.y = 240;


    // Animation
    playerTank.currentFrame = 0;
    playerTank.lastFrameTime = SDL_GetTicks();
    playerTank.isMoving = false;
    playerTank.isShooting = false;
    playerTank.shootingStartTime = 0;


}
