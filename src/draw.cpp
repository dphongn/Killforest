#include <SDL.h>
#include <SDL_image.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdlib> // For rand() and srand()
#include <ctime>   // For time()
#include "Game.h"
// #include "Bullets.h"
#include "Wall.h"
#include <fstream>
using namespace std;
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int TANK_SIZE = 40;
const int BULLET_SIZE = 10;
const int BULLET_SPEED = 5;
const int WALL_SIZE = 40;

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;

struct Tank {
    int x, y;
    int speed;
    int direction; // 0: up, 1: right, 2: down, 3: left, 4: up-right, 5: down-right, 6: down-left, 7: up-left
    bool active;
    Uint32 lastShotTime; // Time when the tank last shot a bullet
};

struct Bullet {
    int x, y;
    int direction;
    bool active;
    bool isPlayerBullet; // To distinguish between player and enemy bullets
};

// struct Wall {
//     int x, y;
//     bool active;
// };

Tank playerTank;
std::vector<Bullet> bullets;
std::vector<Wall> walls;
std::vector<Tank> enemyTanks;

bool keyStates[SDL_NUM_SCANCODES] = { false }; // Track the state of each key

bool checkCollision(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2) {
    return (x1 < x2 + w2 && x1 + w1 > x2 && y1 < y2 + h2 && y1 + h1 > y2);
}
void init() {
    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow("Battle City", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // Initialize walls
    // for (int i = 0; i < 10; ++i) {
    //     Wall wall;
    //     wall.x = rand() % (SCREEN_WIDTH / WALL_SIZE) * WALL_SIZE;
    //     wall.y = rand() % (SCREEN_HEIGHT / WALL_SIZE) * WALL_SIZE;
    //     wall.active = true;
    //     walls.push_back(wall);
    // }
    walls = Initialize_walls();

    // Initialize player tank
    playerTank.speed = 3;
    playerTank.direction = 0;
    playerTank.active = true;
    playerTank.lastShotTime = 0;

    // Ensure player tank does not spawn on a wall
    bool validPosition = false;
    while (!validPosition) {
        playerTank.x = rand() % (SCREEN_WIDTH - TANK_SIZE);
        playerTank.y = rand() % (SCREEN_HEIGHT - TANK_SIZE);
        validPosition = true;

        for (const auto& wall : walls) {
            if (checkCollision(playerTank.x, playerTank.y, TANK_SIZE, TANK_SIZE, wall.x, wall.y, WALL_SIZE, WALL_SIZE)) {
                validPosition = false;
                break;
            }
        }
    }

    // Initialize enemy tanks
    for (int i = 0; i < 3; ++i) {
        Tank enemy;
        enemy.speed = 2;
        enemy.direction = rand() % 4;
        enemy.active = true;
        enemy.lastShotTime = SDL_GetTicks(); // Initialize last shot time

        // Ensure enemy tank does not spawn on a wall or on the player tank
        validPosition = false;
        while (!validPosition) {
            enemy.x = rand() % (SCREEN_WIDTH - TANK_SIZE);
            enemy.y = rand() % (SCREEN_HEIGHT - TANK_SIZE);
            validPosition = true;

            // Check collision with walls
            for (const auto& wall : walls) {
                if (checkCollision(enemy.x, enemy.y, TANK_SIZE, TANK_SIZE, wall.x, wall.y, WALL_SIZE, WALL_SIZE)) {
                    validPosition = false;
                    break;
                }
            }

            // Check collision with player tank
            if (checkCollision(enemy.x, enemy.y, TANK_SIZE, TANK_SIZE, playerTank.x, playerTank.y, TANK_SIZE, TANK_SIZE)) {
                validPosition = false;
            }
        }

        enemyTanks.push_back(enemy);
    }

    srand(time(0)); // Seed random number generator
}

void close() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

// bool checkCollision(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2) {
//     return (x1 < x2 + w2 && x1 + w1 > x2 && y1 < y2 + h2 && y1 + h1 > y2);
// }

void handleInput(SDL_Event& event) {
    if (event.type == SDL_KEYDOWN) {
        keyStates[event.key.keysym.scancode] = true;
    } else if (event.type == SDL_KEYUP) {
        keyStates[event.key.keysym.scancode] = false;
    }

    // Determine the direction based on the combination of keys pressed
    if (keyStates[SDL_SCANCODE_UP] && keyStates[SDL_SCANCODE_RIGHT]) {
        playerTank.direction = 4; // Up-Right
    } else if (keyStates[SDL_SCANCODE_UP] && keyStates[SDL_SCANCODE_LEFT]) {
        playerTank.direction = 7; // Up-Left
    } else if (keyStates[SDL_SCANCODE_DOWN] && keyStates[SDL_SCANCODE_RIGHT]) {
        playerTank.direction = 5; // Down-Right
    } else if (keyStates[SDL_SCANCODE_DOWN] && keyStates[SDL_SCANCODE_LEFT]) {
        playerTank.direction = 6; // Down-Left
    } else if (keyStates[SDL_SCANCODE_UP]) {
        playerTank.direction = 0; // Up
    } else if (keyStates[SDL_SCANCODE_RIGHT]) {
        playerTank.direction = 1; // Right
    } else if (keyStates[SDL_SCANCODE_DOWN]) {
        playerTank.direction = 2; // Down
    } else if (keyStates[SDL_SCANCODE_LEFT]) {
        playerTank.direction = 3; // Left
    }

    // Move the tank based on the direction
    int prevX = playerTank.x;
    int prevY = playerTank.y;

    switch (playerTank.direction) {
        case 0: playerTank.y -= playerTank.speed; break;
        case 1: playerTank.x += playerTank.speed; break;
        case 2: playerTank.y += playerTank.speed; break;
        case 3: playerTank.x -= playerTank.speed; break;
        case 4: playerTank.x += playerTank.speed; playerTank.y -= playerTank.speed; break; // Up-Right
        case 5: playerTank.x += playerTank.speed; playerTank.y += playerTank.speed; break; // Down-Right
        case 6: playerTank.x -= playerTank.speed; playerTank.y += playerTank.speed; break; // Down-Left
        case 7: playerTank.x -= playerTank.speed; playerTank.y -= playerTank.speed; break; // Up-Left
    }

    // Prevent player from going out of the window
    if (playerTank.x < 0) playerTank.x = 0;
    if (playerTank.x > SCREEN_WIDTH - TANK_SIZE) playerTank.x = SCREEN_WIDTH - TANK_SIZE;
    if (playerTank.y < 0) playerTank.y = 0;
    if (playerTank.y > SCREEN_HEIGHT - TANK_SIZE) playerTank.y = SCREEN_HEIGHT - TANK_SIZE;

    // Check collision between player tank and walls
    for (const auto& wall : walls) {
        if (wall.active && checkCollision(playerTank.x, playerTank.y, TANK_SIZE, TANK_SIZE, wall.x, wall.y, WALL_SIZE, WALL_SIZE)) {
            // Move player tank back to its previous position
            playerTank.x = prevX;
            playerTank.y = prevY;
        }
    }

    // Handle shooting
    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE) {
        Bullet bullet;
        bullet.x = playerTank.x + TANK_SIZE / 2 - BULLET_SIZE / 2;
        bullet.y = playerTank.y + TANK_SIZE / 2 - BULLET_SIZE / 2;
        bullet.direction = playerTank.direction;
        bullet.active = true;
        bullet.isPlayerBullet = true;
        bullets.push_back(bullet);
    }
}

void update() {
    // Update player bullets
    for (auto& bullet : bullets) {
        if (bullet.active) {
            switch (bullet.direction) {
                case 0: bullet.y -= BULLET_SPEED; break;
                case 1: bullet.x += BULLET_SPEED; break;
                case 2: bullet.y += BULLET_SPEED; break;
                case 3: bullet.x -= BULLET_SPEED; break;
                case 4: bullet.x += BULLET_SPEED; bullet.y -= BULLET_SPEED; break; // Up-Right
                case 5: bullet.x += BULLET_SPEED; bullet.y += BULLET_SPEED; break; // Down-Right
                case 6: bullet.x -= BULLET_SPEED; bullet.y += BULLET_SPEED; break; // Down-Left
                case 7: bullet.x -= BULLET_SPEED; bullet.y -= BULLET_SPEED; break; // Up-Left
            }

            // Check if bullet is out of bounds
            if (bullet.x < 0 || bullet.x > SCREEN_WIDTH || bullet.y < 0 || bullet.y > SCREEN_HEIGHT) {
                bullet.active = false;
            }

            // Check if bullet hits a wall
            for (auto& wall : walls) {
                if (wall.active && checkCollision(bullet.x, bullet.y, BULLET_SIZE, BULLET_SIZE, wall.x, wall.y, WALL_SIZE, WALL_SIZE)) {
                    bullet.active = false;
                    wall.active = false; // Break the wall
                }
            }

            // Check if bullet hits an enemy (only if it's a player bullet)
            if (bullet.isPlayerBullet) {
                for (auto& enemy : enemyTanks) {
                    if (enemy.active && checkCollision(bullet.x, bullet.y, BULLET_SIZE, BULLET_SIZE, enemy.x, enemy.y, TANK_SIZE, TANK_SIZE)) {
                        bullet.active = false;
                        enemy.active = false; // Destroy the enemy
                    }
                }
            }

            // Check if bullet hits the player (only if it's an enemy bullet)
            if (!bullet.isPlayerBullet && playerTank.active && checkCollision(bullet.x, bullet.y, BULLET_SIZE, BULLET_SIZE, playerTank.x, playerTank.y, TANK_SIZE, TANK_SIZE)) {
                bullet.active = false;
                playerTank.active = false; // Player is killed
                std::cout << "Player has been killed!" << std::endl;
            }
        }
    }

    // Remove inactive bullets
    bullets.erase(std::remove_if(bullets.begin(), bullets.end(), [](const Bullet& b) { return !b.active; }), bullets.end());

    // Remove destroyed enemies
    enemyTanks.erase(std::remove_if(enemyTanks.begin(), enemyTanks.end(), [](const Tank& t) { return !t.active; }), enemyTanks.end());

    // Update enemy tanks
    for (auto& enemy : enemyTanks) {
        int prevX = enemy.x;
        int prevY = enemy.y;

        switch (enemy.direction) {
            case 0: enemy.y -= enemy.speed; break;
            case 1: enemy.x += enemy.speed; break;
            case 2: enemy.y += enemy.speed; break;
            case 3: enemy.x -= enemy.speed; break;
        }

        // Keep enemy within screen bounds
        if (enemy.x < 0) enemy.x = 0;
        if (enemy.x > SCREEN_WIDTH - TANK_SIZE) enemy.x = SCREEN_WIDTH - TANK_SIZE;
        if (enemy.y < 0) enemy.y = 0;
        if (enemy.y > SCREEN_HEIGHT - TANK_SIZE) enemy.y = SCREEN_HEIGHT - TANK_SIZE;

        // Check collision between enemy tanks and walls
        for (const auto& wall : walls) {
            if (wall.active && checkCollision(enemy.x, enemy.y, TANK_SIZE, TANK_SIZE, wall.x, wall.y, WALL_SIZE, WALL_SIZE)) {
                // Move enemy tank back to its previous position
                enemy.x = prevX;
                enemy.y = prevY;
                // Change direction randomly
                enemy.direction = rand() % 4;
            }
        }

        // Enemy shooting logic
        Uint32 currentTime = SDL_GetTicks();
        if (currentTime - enemy.lastShotTime > 2000) { // Shoot every 2 seconds
            Bullet bullet;
            bullet.x = enemy.x + TANK_SIZE / 2 - BULLET_SIZE / 2;
            bullet.y = enemy.y + TANK_SIZE / 2 - BULLET_SIZE / 2;
            bullet.direction = enemy.direction;
            bullet.active = true;
            bullet.isPlayerBullet = false;
            bullets.push_back(bullet);
            enemy.lastShotTime = currentTime;
        }
    }
}

void render() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Draw walls
    for (const auto& wall : walls) {
        if (wall.active) {
            SDL_SetRenderDrawColor(renderer, 139, 69, 19, 255); // Brown for walls
            SDL_Rect wallRect = { wall.x, wall.y, WALL_SIZE, WALL_SIZE };
            SDL_RenderFillRect(renderer, &wallRect);
        }
    }

    // Draw player tank
    if (playerTank.active) {
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_Rect tankRect = { playerTank.x, playerTank.y, TANK_SIZE, TANK_SIZE };
        SDL_RenderFillRect(renderer, &tankRect);
    }

    // Draw enemy tanks
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    for (const auto& enemy : enemyTanks) {
        if (enemy.active) {
            SDL_Rect enemyRect = { enemy.x, enemy.y, TANK_SIZE, TANK_SIZE };
            SDL_RenderFillRect(renderer, &enemyRect);
        }
    }

    // Draw bullets
    for (const auto& bullet : bullets) {
        if (bullet.active) {
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
            SDL_Rect bulletRect = { bullet.x, bullet.y, BULLET_SIZE, BULLET_SIZE };
            SDL_RenderFillRect(renderer, &bulletRect);
        }
    }

    SDL_RenderPresent(renderer);
}

int main(int argc, char* argv[]) {
    init();

    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            handleInput(event);
        }

        update();
        render();

        // End the game if the player is killed
        if (!playerTank.active) {
            std::cout << "Game Over! You were killed." << std::endl;
            running = false;
        }

        // End the game if all enemies are killed
        if (enemyTanks.empty()) {
            std::cout << "You Win! All enemies have been destroyed." << std::endl;
            running = false;
        }

        SDL_Delay(16); // ~60 FPS
    }

    close();
    return 0;
}