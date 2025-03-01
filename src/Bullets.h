#ifndef BULLET_H
#define BULLET_H

struct Bullet {
    int x, y;
    int direction;
    bool active;
    bool isPlayerBullet;
    void update_bullet_pos()
};

#endif // BULLET_H