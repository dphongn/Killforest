#ifndef TANK_H
#define TANK_H

#include <SDL.h>
#include <vector>
#include "Bullet.h"

struct Tank {
    int x, y;
    int speed;
    int direction;
    bool active;
    Uint32 lastShotTime;

    void move(int newX, int newY);
    void shoot(std::vector<Bullet>& bullets, bool isPlayer);
};
//asdjnadjns
#endif // TANK_H