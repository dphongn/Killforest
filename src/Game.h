#ifndef GAME_H

#define GAME_H
#include<iostream>
#include <vector>
#include "Wall.h"
#include "Tank.h"
using namespace std;

vector<Wall> Initialize_walls(string filename);
bool checkCollision(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2);
vector<Tank> Initialize_enemies();
bool checkBulletCollision(int x1, int y1, int x2, int y2, int sizeTank);
#endif