#ifndef BULLET_H
#define BULLET_H
#include <SDL.h>
struct Bullet {
    int x, y;
    int direction;
    bool active;
    bool active2;
    bool isPlayerBullet;

    // void update_bullet_pos()
};

#endif // BULLET_H