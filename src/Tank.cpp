#include "Tank.h"
#include <iostream>

Tank::Tank() {
    struct Tank {
        int x, y;
        int speed;
        int direction; // 0: up, 1: right, 2: down, 3: left, 4: up-right, 5: down-right, 6: down-left, 7: up-left
        bool active;
        Uint32 lastShotTime; // Time when the tank last shot a bullet
    };
}

void Tank::move() {
    // Movement logic
}

void Tank::shoot() {
    // Shooting logic
}