#include "Game.h"
#include <iostream>
#include "Wall.h"
#include <fstream>
#include <vector>
#include "Tank.h"
using namespace std;


bool checkCollision(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2) {
    return (x1 < x2 + w2 && x1 + w1 > x2 && y1 < y2 + h2 && y1 + h1 > y2);
}

bool checkBulletCollision(int x1, int y1, int x2, int y2, int sizeTank){
    return (x1-x2)*(x1-x2) + (y1-y2)*(y1-y2) <= sizeTank*sizeTank/2;
}

vector<Wall> Initialize_walls(string filename){
    // ifstream wall_map("C:/Users/dphong/Downloads/mygame/src/wallmap.txt");
    ifstream wall_map(filename);
    vector<Wall> walls;
    Wall wall;
    if (!wall_map) { // Kiểm tra nếu file không mở được
        cerr << "Error: Could not open wallmap.txt" << endl;
        return {};
    }
    while(wall_map >> wall.x >> wall.y){

        wall.active = true;
        walls.push_back(wall);
    }
    wall_map.close();
    return walls;
}
vector<Tank> Initialize_enemies(){
    ifstream enemies_map("C:/Users/dphong/Downloads/mygame/src/enemiesmap.txt");
    vector<Tank> enemies;
    Tank enemy;
    if (!enemies_map) { // Kiểm tra nếu file không mở được
        cerr << "Error: Could not open enemiesmap.txt" << endl;
        return {};
    }
    while(enemies_map >> enemy.x >> enemy.y >> enemy.direction){
        enemy.speed = 1;
        // if(enemy.x == 0){
        //     enemy.direction = 1;
        // }
        // else if(enemy.x == 760){
        //     enemy.direction = 3;
        // }
        // else if(enemy.y == 0){
        //     enemy.direction = 2;
        // }
        // else if(enemy.y = 560){
        //     enemy.direction = 0;
        // }
        enemy.active = true;
        enemy.range = 200/40 *32;
        enemies.push_back(enemy);
    }
    enemies_map.close();
    return enemies;
}
// void Initialize_playerTank(Tank &playerTank, int SCREEN_WIDTH, int SCREEN_HEIGHT, int TANK_SIZE){
//     playerTank.active = true;
//     playerTank.lastShotTime = 0;
//     bool validPosition = false;
//     while (!validPosition) {
//         playerTank.x = rand() % (SCREEN_WIDTH - TANK_SIZE);
//         playerTank.y = rand() % (SCREEN_HEIGHT - TANK_SIZE);
//         validPosition = true;
//     }
// }
