#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdlib> 
#include <ctime>  
#include "Game.h"
#include "Bullets.h"
#include "Wall.h"
#include "Tank.h"
#include "Explosion.h"
#include <fstream>
#include <sstream>
#include <string>
#include <SDL_mixer.h> 

using namespace std;

Mix_Chunk* shootSound = nullptr;
Mix_Chunk* enemyshootSound = nullptr;
Mix_Chunk* enemyDeath = nullptr;
Mix_Chunk* gameover = nullptr;
Mix_Chunk* victory = nullptr;
Mix_Chunk* treefall = nullptr;
Mix_Chunk* bulletsExplosion = nullptr;
Mix_Music* menuBackgroundMusic = nullptr;
Mix_Music* gameBackgroundMusic = nullptr;
Mix_Chunk* menu1 = nullptr;
Mix_Chunk* menu2 = nullptr;
bool isMusicPlaying = false;
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int TANK_SIZE = 28;
const int BULLET_SIZE = 16;
const int BULLET_SPEED = 3;
const int WALL_SIZE = 30;
const int TILE_SIZE = 32;
const int MAP_WIDTH = 20;
const int MAP_HEIGHT = 15;
const int FRAME_WIDTH = 16;
const int FRAME_HEIGHT = 16;
const int FRAME_COUNT = 4;
const int FRAMES_PER_ROW = 4;
const int ANIMATION_ROWS = 5;
const int SHOOTING_ROW = 4; 
const int SHOOTING_DIAGONAL_ROW = 5;
bool continuehandle = false;
SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
SDL_Texture* tilesetTexture = nullptr;
SDL_Texture* wallTexture = nullptr;
SDL_Texture* tankTexture = nullptr;
SDL_Texture* enemyTexture = nullptr;
vector<SDL_Texture*> bulletFrames, bulletFrames2; 
int currentBulletFrame = 0; 
Uint32 lastBulletFrameTime = 0; 
const int BULLET_FRAME_DELAY = 50;
vector<vector<int>> mapData;
enum Direction {
    DIR_UP = 0,
    DIR_RIGHT = 1,
    DIR_DOWN = 2,
    DIR_LEFT = 3,
    DIR_UP_RIGHT = 4,
    DIR_DOWN_RIGHT = 5,
    DIR_DOWN_LEFT = 6,
    DIR_UP_LEFT = 7
};
Tank playerTank;
vector<Bullet> bullets;
vector<Wall> walls;
vector<Tank> enemyTanks;
const int EXPLOSION_FRAME_WIDTH = 32;
const int EXPLOSION_FRAME_HEIGHT = 32;
const int EXPLOSION_FRAME_COUNT = 6;
const int EXPLOSION_DURATION = 300;
const int EXPLOSION_FRAME_DELAY = EXPLOSION_DURATION / EXPLOSION_FRAME_COUNT;

Uint32 gameStartTime = 0;
bool gameCompleted = false;
Uint32 completionTime = 0;
TTF_Font* font = nullptr;
SDL_Color textColor = {88, 57, 39, 255};
vector<Explosion> explosions;
SDL_Texture* explosionTexture = nullptr;
bool keyStates[SDL_NUM_SCANCODES] = { false }; 

enum GameState {
    MENU,
    LEVEL_SELECT,
    GUIDE,
    PLAYING
};

GameState currentGameState = MENU;
SDL_Texture* menuBackground = nullptr;
SDL_Texture* levelSelectBackground = nullptr;
SDL_Texture* guideBackground = nullptr;
SDL_Texture* map1Texture = nullptr;
SDL_Texture* map2Texture = nullptr;

SDL_Rect startButtonRect = {SCREEN_WIDTH/2 - 100, SCREEN_HEIGHT/2 - 50, 200, 50};
SDL_Rect guideButtonRect = {SCREEN_WIDTH/2 - 100, SCREEN_HEIGHT/2 + 20, 200, 50};
SDL_Rect map1ButtonRect = {SCREEN_WIDTH/2 - 100, SCREEN_HEIGHT/2 - 50, 200, 50};
SDL_Rect map2ButtonRect = {SCREEN_WIDTH/2 - 100, SCREEN_HEIGHT/2 + 20, 200, 50};
bool startHovered = false;
bool guideHovered = false;
bool map1Hovered = false;
bool map2Hovered = false;
bool menu1sound = false;
bool menu2sound = false;
bool isplayingsound = false;
void renderText(const string& text, int x, int y, SDL_Color color) {
    if (!font) return;
    
    SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), color);
    if (!surface) return;
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        SDL_FreeSurface(surface);
        return;
    }
    
    SDL_Rect dstRect = {x, y, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, NULL, &dstRect);
    
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}
void renderMenu() {
    SDL_RenderCopy(renderer, menuBackground, NULL, NULL);
    
    SDL_Color startColor = startHovered ? SDL_Color{255, 255, 255, 255} : SDL_Color{255, 0, 0, 255};
    renderText("START", startButtonRect.x + 70, startButtonRect.y + 10, startColor);
    
    SDL_Color guideColor = guideHovered ? SDL_Color{255, 255, 255, 255} : SDL_Color{255, 0, 0, 255};
    renderText("GUIDE", guideButtonRect.x + 70, guideButtonRect.y + 10, guideColor);
}
void renderLevelSelect() {
    SDL_RenderCopy(renderer, levelSelectBackground, NULL, NULL);
    
    SDL_Color map1Color = map1Hovered ? SDL_Color{255, 255, 255, 255} : SDL_Color{255, 0, 0, 255};
    renderText("MAP 1", map1ButtonRect.x + 70, map1ButtonRect.y + 10, map1Color);
    
    SDL_Color map2Color = map2Hovered ? SDL_Color{255, 255, 255, 255} : SDL_Color{255, 0, 0, 255};
    renderText("MAP 2", map2ButtonRect.x + 70, map2ButtonRect.y + 10, map2Color);
}
void renderGuide() {
    SDL_RenderCopy(renderer, guideBackground, NULL, NULL);
    
    renderText("Press ESC to return", SCREEN_WIDTH/2 - 100, SCREEN_HEIGHT - 50, SDL_Color{255, 255, 255, 255});
}
bool initAudio() {
    Mix_AllocateChannels(16);
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048)) {
        cerr << "SDL_mixer could not initialize! Mix_Error: " << Mix_GetError() << endl;
        return false;
    }
    shootSound = Mix_LoadWAV("map/sound/playershoot.wav");
    enemyshootSound = Mix_LoadWAV("map/sound/enemyshoot.wav");
    enemyDeath = Mix_LoadWAV("map/sound/enemydeath.wav");
    gameover = Mix_LoadWAV("map/sound/over.wav");
    victory = Mix_LoadWAV("map/sound/victory.wav");
    treefall = Mix_LoadWAV("map/sound/treefall.wav");
    bulletsExplosion = Mix_LoadWAV("map/sound/bulletsexplosion.wav");
    menuBackgroundMusic = Mix_LoadMUS("map/sound/menu_background.ogg"); 
    gameBackgroundMusic = Mix_LoadMUS("map/sound/game_background.ogg");
    menu1 = Mix_LoadWAV("map/sound/menu_1.wav");
    menu2 = Mix_LoadWAV("map/sound/menu_2.wav");
    return true;
}
void handleInput(SDL_Event& event) {
    int prevX = playerTank.x;
    int prevY = playerTank.y;
    if (event.type == SDL_KEYDOWN) {
        keyStates[event.key.keysym.scancode] = true;
    } else if (event.type == SDL_KEYUP) {
        if(event.key.keysym.scancode == SDL_SCANCODE_SPACE){
            playerTank.isShooting = false;
            return;
        }
        keyStates[event.key.keysym.scancode] = false;
        if(!keyStates[SDL_SCANCODE_UP] && !keyStates[SDL_SCANCODE_DOWN] && !keyStates[SDL_SCANCODE_LEFT] && !keyStates[SDL_SCANCODE_RIGHT])playerTank.isMoving = false;
        return;
    }
    else if(event.type == SDL_MOUSEMOTION){
        return;
    }
    if (keyStates[SDL_SCANCODE_UP] && keyStates[SDL_SCANCODE_RIGHT]) {
        if(playerTank.direction != 4)playerTank.direction = 4; // Up-Right
        playerTank.x += playerTank.speed;
        playerTank.y -= playerTank.speed;
    } else if (keyStates[SDL_SCANCODE_UP] && keyStates[SDL_SCANCODE_LEFT]) {
        if(playerTank.direction != 7)playerTank.direction = 7; // Up-Left
        playerTank.x -= playerTank.speed;
        playerTank.y -= playerTank.speed;
    } else if (keyStates[SDL_SCANCODE_DOWN] && keyStates[SDL_SCANCODE_RIGHT]) {
        if(playerTank.direction != 5)playerTank.direction = 5; // Down-Right
        playerTank.x += playerTank.speed;
        playerTank.y += playerTank.speed;   
    } else if (keyStates[SDL_SCANCODE_DOWN] && keyStates[SDL_SCANCODE_LEFT]) {
        if(playerTank.direction != 6)playerTank.direction = 6; // Down-Left
        playerTank.x -= playerTank.speed;
        playerTank.y += playerTank.speed;
    } else if (keyStates[SDL_SCANCODE_UP]) {
        if(playerTank.direction != 0)playerTank.direction = 0; // Up
        playerTank.y -= playerTank.speed;
    } else if (keyStates[SDL_SCANCODE_RIGHT]) {
        if(playerTank.direction != 1)playerTank.direction = 1; // Right
       playerTank.x += playerTank.speed;
    } else if (keyStates[SDL_SCANCODE_DOWN]) {
        if(playerTank.direction != 2)playerTank.direction = 2; // Down
        playerTank.y += playerTank.speed;
    } else if (keyStates[SDL_SCANCODE_LEFT]) {
        if(playerTank.direction != 3)playerTank.direction = 3; // Left
       playerTank.x -= playerTank.speed;
    }
    Uint32 currentTime = SDL_GetTicks();
    const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);
    if (((event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE) || currentKeyStates[SDLK_SPACE]||currentKeyStates[SDL_SCANCODE_SPACE]) && currentTime - playerTank.lastShotTime > 300) {
        Mix_PlayChannel(2, shootSound, 0);
        Mix_Volume(2, MIX_MAX_VOLUME / 3);
        Bullet bullet;
        keyStates[SDLK_SPACE] = false;
        bullet.x = playerTank.x + TANK_SIZE / 2 - BULLET_SIZE / 2;
        bullet.y = playerTank.y + TANK_SIZE / 2 - BULLET_SIZE / 2;
        bullet.direction = playerTank.direction;
        bullet.active = true;
        bullet.active2 = true;
        bullet.isPlayerBullet = true;
        bullets.push_back(bullet);
        playerTank.isShooting = true;
        playerTank.shootingStartTime = SDL_GetTicks();
        playerTank.lastShotTime = SDL_GetTicks();
        return;
    }
    int prevDirection = playerTank.direction;
    if (playerTank.x < 0) playerTank.x = 0;
    if (playerTank.x > SCREEN_WIDTH - TANK_SIZE) playerTank.x = SCREEN_WIDTH - TANK_SIZE;
    if (playerTank.y < 0) playerTank.y = 0;
    if (playerTank.y > SCREEN_HEIGHT - TANK_SIZE) playerTank.y = SCREEN_HEIGHT - TANK_SIZE;

    for (const auto& wall : walls) {
        if (wall.active && checkCollision(playerTank.x, playerTank.y, TANK_SIZE, TANK_SIZE, wall.x, wall.y, WALL_SIZE, WALL_SIZE)) {
            playerTank.x = prevX;
            playerTank.y = prevY;
        }
    }

        playerTank.isMoving = (keyStates[SDL_SCANCODE_UP] || 
            keyStates[SDL_SCANCODE_RIGHT] ||
            keyStates[SDL_SCANCODE_DOWN] ||
            keyStates[SDL_SCANCODE_LEFT]);
    if(continuehandle)continuehandle = false;
}
bool initFont() {
    if (TTF_Init() == -1)return false;
    
    font = TTF_OpenFont("map/font.ttf", 24);
    if (!font)return false;
    return true;
}
void update() {
    for (auto& bullet : bullets) {
        if (bullet.active) {
            switch (bullet.direction) {
                case 0: bullet.y -= BULLET_SPEED; break;
                case 1: bullet.x += BULLET_SPEED; break;
                case 2: bullet.y += BULLET_SPEED; break;
                case 3: bullet.x -= BULLET_SPEED; break;
                case 4: bullet.x += BULLET_SPEED; bullet.y -= BULLET_SPEED; break; 
                case 5: bullet.x += BULLET_SPEED; bullet.y += BULLET_SPEED; break; 
                case 6: bullet.x -= BULLET_SPEED; bullet.y += BULLET_SPEED; break;
                case 7: bullet.x -= BULLET_SPEED; bullet.y -= BULLET_SPEED; break;
            }

            if (bullet.x < 0 || bullet.x > SCREEN_WIDTH || bullet.y < 0 || bullet.y > SCREEN_HEIGHT) {
                bullet.active = false;
            }

            for (auto& wall : walls) {
                if (wall.active && checkCollision(bullet.x, bullet.y, BULLET_SIZE, BULLET_SIZE, wall.x, wall.y, WALL_SIZE, WALL_SIZE)) {
                    bullet.active = false;
                    if(bullet.isPlayerBullet){
                        wall.active = false; 
                        Mix_PlayChannel(5, treefall, 0);
                        Mix_Volume(5, MIX_MAX_VOLUME / 3);
                        addExplosion(bullet.x + BULLET_SIZE/2,bullet.y + BULLET_SIZE/2, EXPLOSION_FRAME_WIDTH, EXPLOSION_FRAME_HEIGHT, explosions);
                    }
                }
            }

            if (bullet.isPlayerBullet) {
                for (auto& enemy : enemyTanks) {
                    if (enemy.active && checkCollision(bullet.x, bullet.y, BULLET_SIZE, BULLET_SIZE, enemy.x, enemy.y, TANK_SIZE, TANK_SIZE)) {
                        bullet.active = false;
                        enemy.active = false;
                        Mix_PlayChannel(0, enemyDeath, 0);
                        addExplosion(enemy.x + TANK_SIZE/2,enemy.y + TANK_SIZE/2, EXPLOSION_FRAME_WIDTH, EXPLOSION_FRAME_HEIGHT, explosions);

                    }
                }
                for(auto &bullet2 : bullets){
                    if(!bullet2.isPlayerBullet &&bullet2.active && checkBulletCollision(bullet.x, bullet.y, bullet2.x, bullet2.y, TANK_SIZE)){
                        bullet.active = false;
                        bullet2.active = false;
                        Mix_PlayChannel(6, bulletsExplosion, 0);
                        Mix_Volume(6, MIX_MAX_VOLUME / 2);
                        addExplosion(bullet.x + BULLET_SIZE/2,bullet.y + BULLET_SIZE/2, EXPLOSION_FRAME_WIDTH, EXPLOSION_FRAME_HEIGHT, explosions);
                        
                    }
                }
            }

            if (!bullet.isPlayerBullet && playerTank.active && checkCollision(bullet.x, bullet.y, BULLET_SIZE, BULLET_SIZE, playerTank.x, playerTank.y, TANK_SIZE, TANK_SIZE)) {
                bullet.active = false;
                playerTank.active = false;
                addExplosion(playerTank.x + TANK_SIZE/2,playerTank.y + TANK_SIZE/2, EXPLOSION_FRAME_WIDTH, EXPLOSION_FRAME_HEIGHT, explosions);
                
                cout << "Player has been killed!" << endl;
            }

        }
    }

    bullets.erase(remove_if(bullets.begin(), bullets.end(), [](const Bullet& b) { return !b.active; }), bullets.end());

    enemyTanks.erase(remove_if(enemyTanks.begin(), enemyTanks.end(), [](const Tank& t) { return !t.active; }), enemyTanks.end());

    for (auto& enemy : enemyTanks) {
        if(enemy.range <= 0){
            enemy.direction = rand() % 4;
            enemy.range = 200;
        }
        int prevX = enemy.x;
        int prevY = enemy.y;

        switch (enemy.direction) {
            case 0: enemy.y -= enemy.speed; ;break;
            case 1: enemy.x += enemy.speed; break;
            case 2: enemy.y += enemy.speed; break;
            case 3: enemy.x -= enemy.speed; break;
        }

        // Keep enemy within screen bounds
        if (enemy.x <= 0){ enemy.x = 0; enemy.direction = 1;}
        if (enemy.x >= SCREEN_WIDTH - TANK_SIZE) {enemy.x = SCREEN_WIDTH - TANK_SIZE; enemy.direction = 3;}
        if (enemy.y <= 0){ enemy.y = 0;enemy.direction = 2;}
        if (enemy.y >= SCREEN_HEIGHT - TANK_SIZE){ enemy.y = SCREEN_HEIGHT - TANK_SIZE;enemy.direction = 0;}
        enemy.range -= abs(enemy.x - prevX) + abs(enemy.y - prevY);
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
        if (currentTime - enemy.lastShotTime > 4000 + rand()%4000) { // Shoot every 2 seconds
            Bullet bullet;
            bullet.x = enemy.x + TANK_SIZE / 2 - BULLET_SIZE / 2;
            bullet.y = enemy.y + TANK_SIZE / 2 - BULLET_SIZE / 2;
            bullet.direction = enemy.direction;
            bullet.active = true;
            bullet.isPlayerBullet = false;
            bullets.push_back(bullet);
            enemy.lastShotTime = currentTime;
            Mix_PlayChannel(1, enemyshootSound, 0);
            Mix_Volume(1, MIX_MAX_VOLUME / 3);
        }
        for(auto &enemy2 : enemyTanks){
            if(&enemy != &enemy2 && enemy2.active && checkCollision(enemy.x, enemy.y, TANK_SIZE, TANK_SIZE, enemy2.x, enemy2.y, TANK_SIZE, TANK_SIZE)){
                enemy.x = prevX;
                enemy.y = prevY;
                enemy.direction = rand() % 4;
            }
        }
    }
    if (!gameCompleted && enemyTanks.empty()) {
        gameCompleted = true;
        completionTime = SDL_GetTicks() - gameStartTime;
    }
}
bool loadMap(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }

    string line;
    while (getline(file, line)) {
        vector<int> row;
        stringstream ss(line);
        string cell;

        while (getline(ss, cell, ',')) {
            row.push_back(stoi(cell));
        }

        mapData.push_back(row);
    }

    file.close();
    return true;
}
bool initSDL() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        return false;
    }

    int imgFlags = IMG_INIT_PNG | IMG_INIT_JPG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        return false;
    }

    window = SDL_CreateWindow("Kill Forest", 
                            SDL_WINDOWPOS_CENTERED, 
                            SDL_WINDOWPOS_CENTERED,
                            MAP_WIDTH * TILE_SIZE,
                            MAP_HEIGHT * TILE_SIZE,
                            SDL_WINDOW_SHOWN);
    if (!window) {
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        return false;
    }

    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);

    tilesetTexture = IMG_LoadTexture(renderer, "map/map1/tileset.png");
    if (!tilesetTexture) {
        return false;
    }

    return true;
}
void renderMap() {
    SDL_RenderClear(renderer);

    const int TILESET_COLUMNS = 14;
    
    for (int y = 0; y < mapData.size(); ++y) {
        for (int x = 0; x < mapData[y].size(); ++x) {
            int tileID = mapData[y][x];
            if (tileID == 0) continue; 

            int tilesetX = (tileID ) % TILESET_COLUMNS;
            int tilesetY = (tileID ) / TILESET_COLUMNS;

            SDL_Rect srcRect = {
                tilesetX * TILE_SIZE,
                tilesetY * TILE_SIZE,
                TILE_SIZE,
                TILE_SIZE
            };

            SDL_Rect dstRect = {
                x * TILE_SIZE,
                y * TILE_SIZE,
                TILE_SIZE,
                TILE_SIZE
            };

            // Vẽ tile
            SDL_RenderCopy(renderer, tilesetTexture, &srcRect, &dstRect);
        }
    }

}
void cleanup() {
    for (auto texture : bulletFrames) {
        SDL_DestroyTexture(texture);
    }
    bulletFrames.clear();
    if(wallTexture) SDL_DestroyTexture(wallTexture);
    if (tilesetTexture) SDL_DestroyTexture(tilesetTexture);
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    if (explosionTexture) {
        SDL_DestroyTexture(explosionTexture);
        explosionTexture = nullptr;
    }
    if (font) {
        TTF_CloseFont(font);
        font = nullptr;
    }
    if (shootSound) {
        Mix_FreeChunk(shootSound);
        shootSound = nullptr;
    }
    if (menuBackgroundMusic) {
        Mix_FreeMusic(menuBackgroundMusic);
        menuBackgroundMusic = nullptr;
    }
    if (gameBackgroundMusic) {
        Mix_FreeMusic(gameBackgroundMusic);
        gameBackgroundMusic = nullptr;
    }
    
    Mix_CloseAudio();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}
bool loadMenuTextures() {
    menuBackground = IMG_LoadTexture(renderer, "map/menu/menu_background.png");
    levelSelectBackground = IMG_LoadTexture(renderer, "map/menu/level_select.png");
    guideBackground = IMG_LoadTexture(renderer, "map/menu/guide.png");
    map1Texture = IMG_LoadTexture(renderer, "map/menu/map1_button.png");
    map2Texture = IMG_LoadTexture(renderer, "map/menu/map2_button.png");
    
    if (!menuBackground || !levelSelectBackground || !guideBackground || !map1Texture || !map2Texture) {
        return false;
    }
    return true;
}
bool loadWallTexture(const char *file) {
    wallTexture = IMG_LoadTexture(renderer,file);
    if (!wallTexture)return false;
    return true;
}
bool loadTextures() {
    tankTexture = IMG_LoadTexture(renderer, "map/map1/1.png");
    enemyTexture = IMG_LoadTexture(renderer, "map/map1/2.png");
    if (!tankTexture) {
        return false;
    }
    SDL_SetTextureBlendMode(tankTexture, SDL_BLENDMODE_BLEND);
    SDL_SetTextureBlendMode(enemyTexture, SDL_BLENDMODE_BLEND);
    return true;
}
bool loadBulletFrames() {
    for (int i = 0; i < 4  ; ++i) {
        string path = "map/map1/bullet/frame_" + to_string(i) + "_delay-0.05s" + ".png";
        SDL_Surface* surface = IMG_Load(path.c_str());
        if (!surface) {
            return false;
        }

        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        if (!texture) {
            return false;
        }

        bulletFrames.push_back(texture);
    }
    for (int i = 0; i < 4  ; ++i) {
        string path = "map/map1/bullet2/frame_" + to_string(i) + "_delay-0.2s" + ".png";
        SDL_Surface* surface = IMG_Load(path.c_str());
        if (!surface) {
            return false;
        }

        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        if (!texture) {
            return false;
        }

        bulletFrames2.push_back(texture);
    }
    return true;
}
bool loadExplosionTexture() {
    explosionTexture = IMG_LoadTexture(renderer, "map/map1/explosion.png");
    if (!explosionTexture) {
        return false;
    }
    return true;
}
void renderBullet(const Bullet& bullet, vector<SDL_Texture*> bulletFrames) {
    if (!bullet.active || bulletFrames.empty()) return;

    Uint32 currentTime = SDL_GetTicks();
    if (currentTime - lastBulletFrameTime > BULLET_FRAME_DELAY) {
        currentBulletFrame = (currentBulletFrame + 1) % bulletFrames.size();
        lastBulletFrameTime = currentTime;
    }

    float scale = 1.0f; 
    SDL_Rect dstRect = {
        bullet.x,
        bullet.y,
        (int)(BULLET_SIZE * scale),
        (int)(BULLET_SIZE * scale)
    };
    SDL_RenderCopy(renderer, bulletFrames[currentBulletFrame], NULL, &dstRect);
}
void renderWall(const Wall& wall) {
    if (!wall.active || !wallTexture) return;
    
    SDL_Rect wallRect = {
        wall.x,
        wall.y,
        WALL_SIZE,
        WALL_SIZE
    };
    
    SDL_RenderCopy(renderer, wallTexture, NULL, &wallRect);
}
void renderTank(Tank& tank, int typetank) {
    if (!tank.active) return;

    Uint32 currentTime = SDL_GetTicks();
    SDL_Rect srcRect = {0, 0, FRAME_WIDTH, FRAME_HEIGHT};
    float scale = (float)TANK_SIZE / FRAME_WIDTH;
    if (tank.isShooting) {
        if (currentTime - tank.shootingStartTime > 200) { // Kết thúc sau 200ms
            tank.isShooting = false;
        }
        
        if (tank.direction >= DIR_UP_RIGHT) { // Các hướng chéo
            srcRect.y = SHOOTING_DIAGONAL_ROW * FRAME_HEIGHT;
            if(tank.direction == DIR_UP_RIGHT){
                srcRect.x = 1 * FRAME_WIDTH;
            }
            else if(tank.direction == DIR_DOWN_RIGHT){
                srcRect.x = 3 * FRAME_WIDTH;
            }
            else if(tank.direction == DIR_DOWN_LEFT){
                srcRect.x = 0;
            }
            else if(tank.direction == DIR_UP_LEFT){
                srcRect.x = 2 * FRAME_WIDTH;
            }
        } else { 
            srcRect.y = SHOOTING_ROW * FRAME_HEIGHT;
            if(tank.direction == DIR_DOWN){
                srcRect.x = 0;
            }
            else if(tank.direction == DIR_LEFT){
                srcRect.x = 2 * FRAME_WIDTH;
            }
            else if(tank.direction == DIR_RIGHT){
                srcRect.x = 3 * FRAME_WIDTH;
            }
            else if(tank.direction == DIR_UP){
                srcRect.x = 1 * FRAME_WIDTH;
            }
        }
    }
    else if (tank.isMoving) {
        if (currentTime - tank.lastFrameTime > 100) {
            tank.currentFrame = (tank.currentFrame + 1) % FRAME_COUNT;
            tank.lastFrameTime = currentTime;
        }
        
        srcRect.y = tank.currentFrame * FRAME_HEIGHT;
        if(tank.direction == DIR_DOWN||tank.direction == DIR_DOWN_LEFT){
            srcRect.x = 0;
        }
        else if(tank.direction == DIR_LEFT||tank.direction == DIR_UP_LEFT){
            srcRect.x = 2 * FRAME_WIDTH;
        }
        else if(tank.direction == DIR_RIGHT||tank.direction == DIR_DOWN_RIGHT){
            srcRect.x = 3 * FRAME_WIDTH;
        }
        else if(tank.direction == DIR_UP||tank.direction == DIR_UP_RIGHT){
            srcRect.x = 1 * FRAME_WIDTH;
        }


    }
    else {
        if(tank.direction == DIR_DOWN||tank.direction == DIR_DOWN_LEFT){
            srcRect.x = 0;
        }
        else if(tank.direction == DIR_LEFT||tank.direction == DIR_UP_LEFT){
            srcRect.x = 2 * FRAME_WIDTH;
        }
        else if(tank.direction == DIR_RIGHT||tank.direction == DIR_DOWN_RIGHT){
            srcRect.x = 3 * FRAME_WIDTH;
        }
        else if(tank.direction == DIR_UP||tank.direction == DIR_UP_RIGHT){
            srcRect.x = 1 * FRAME_WIDTH;
        }
        srcRect.y = 0;

    }
    
    SDL_Rect dstRect = {
        tank.x,
        tank.y,
        FRAME_WIDTH,
        FRAME_HEIGHT
    };
    if (scale != 1.0f) {
        dstRect.w = FRAME_WIDTH * scale;
        dstRect.h = FRAME_HEIGHT * scale;
        dstRect.x = tank.x - (dstRect.w - TANK_SIZE)/2;
        dstRect.y = tank.y - (dstRect.h - TANK_SIZE)/2;
    }
    if(typetank == 0)SDL_RenderCopy(renderer, tankTexture, &srcRect, &dstRect);
    else SDL_RenderCopy(renderer, enemyTexture, &srcRect, &dstRect);
}
void renderExplosions() {
    Uint32 currentTime = SDL_GetTicks();
    
    for (auto& exp : explosions) {
        if (!exp.active) continue;
        
        Uint32 elapsed = currentTime - exp.startTime;
        if (elapsed >= EXPLOSION_DURATION) {
            exp.active = false;
            continue;
        }
        
        int frameIndex = (elapsed / EXPLOSION_FRAME_DELAY) % EXPLOSION_FRAME_COUNT;
        
        SDL_Rect srcRect = {
            frameIndex * EXPLOSION_FRAME_WIDTH,
            0,
            EXPLOSION_FRAME_WIDTH,
            EXPLOSION_FRAME_HEIGHT
        };
        
        SDL_Rect dstRect = {
            exp.x,
            exp.y,
            EXPLOSION_FRAME_WIDTH,
            EXPLOSION_FRAME_HEIGHT
        };
        
        SDL_RenderCopy(renderer, explosionTexture, &srcRect, &dstRect);
    }
    
    explosions.erase(remove_if(explosions.begin(), explosions.end(), 
                    [](const Explosion& e) { return !e.active; }), 
                    explosions.end());
}
string formatTime(Uint32 milliseconds) {
    Uint32 seconds = milliseconds / 1000;
    Uint32 minutes = seconds / 60;
    seconds %= 60;
    milliseconds %= 1000;
    
    char buffer[20];
    snprintf(buffer, sizeof(buffer), "%02d:%02d.%03d", minutes, seconds, milliseconds);
    
    return string(buffer);
}
void handleMenuInput(SDL_Event& event) {
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);
    
    startHovered = (mouseX >= startButtonRect.x && mouseX <= startButtonRect.x + startButtonRect.w &&
                   mouseY >= startButtonRect.y && mouseY <= startButtonRect.y + startButtonRect.h);
    guideHovered = (mouseX >= guideButtonRect.x && mouseX <= guideButtonRect.x + guideButtonRect.w &&
                   mouseY >= guideButtonRect.y && mouseY <= guideButtonRect.y + guideButtonRect.h);
    
    if (event.type == SDL_MOUSEBUTTONDOWN) {
        if (startHovered) {
            currentGameState = LEVEL_SELECT;
        } else if (guideHovered) {
            currentGameState = GUIDE;
        }
    }
}
void handleLevelSelectInput(SDL_Event& event) {
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY); 
    // Kiểm tra hover
    map1Hovered = (mouseX >= map1ButtonRect.x && mouseX <= map1ButtonRect.x + map1ButtonRect.w &&
                  mouseY >= map1ButtonRect.y && mouseY <= map1ButtonRect.y + map1ButtonRect.h);
    map2Hovered = (mouseX >= map2ButtonRect.x && mouseX <= map2ButtonRect.x + map2ButtonRect.w &&
                  mouseY >= map2ButtonRect.y && mouseY <= map2ButtonRect.y + map2ButtonRect.h);
    
    // Xử lý click chuột
    if (event.type == SDL_MOUSEBUTTONDOWN) {
        if (map1Hovered) {
            currentGameState = PLAYING;
            gameCompleted = false;
            gameStartTime = SDL_GetTicks();
            loadMap("map/map1/map1.csv");
            loadWallTexture("map/map1/wall.png");
            walls = Initialize_walls("map/map1/wallmap.txt");
            Initialize_playerTank(playerTank, SCREEN_WIDTH, SCREEN_HEIGHT, TANK_SIZE);
            enemyTanks = Initialize_enemies("map/map1/enemiesmap.txt");
            bullets.clear();
            explosions.clear();
        } else if (map2Hovered) {
            currentGameState = PLAYING;
            gameCompleted = false;
            gameStartTime = SDL_GetTicks();
            loadMap("map/map2/map1.csv");
            loadWallTexture("map/map2/wall.png");
            walls = Initialize_walls("map/map2/wallmap.txt");
            Initialize_playerTank(playerTank, SCREEN_WIDTH, SCREEN_HEIGHT, TANK_SIZE);
            enemyTanks = Initialize_enemies("map/map2/enemiesmap.txt");
            bullets.clear();
            explosions.clear();
        }
    }
    
    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
        currentGameState = MENU;
    }
}
void handleGuideInput(SDL_Event& event) {
    // Xử lý phím ESC để quay lại menu
    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
        currentGameState = MENU;
    }
}
void playMenuBackgroundMusic() {
    if (!isMusicPlaying) {
        Mix_PlayMusic(menuBackgroundMusic, -1); // -1 để phát lặp vô hạn
        Mix_VolumeMusic(MIX_MAX_VOLUME / 3); // Giảm âm lượng nếu cần
        isMusicPlaying = true;
        isplayingsound = false;
    }
}
void playGameBackgroundMusic() {
    if(isMusicPlaying&&isplayingsound)return;
    Mix_HaltMusic(); 
    Mix_PlayMusic(gameBackgroundMusic, -1);
    Mix_VolumeMusic(MIX_MAX_VOLUME / 3);
    isMusicPlaying = true;
}
void stopBackgroundMusic() {
    Mix_HaltMusic();
    isMusicPlaying = false;
    isplayingsound=false;
}
int main(int argc, char* argv[]) {
    if (!initSDL() || !initAudio() || !initFont() || !loadTextures() || 
        !loadBulletFrames() || !loadExplosionTexture() || 
        !loadMenuTextures()) {
        cleanup();
        return -1;
    }

    bool running = true;
    SDL_Event event;
    
    while (running) {
        while (SDL_PollEvent(&event)||continuehandle) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            
            if(currentGameState == PLAYING&&!isplayingsound){ playGameBackgroundMusic();isplayingsound=true;}
            else {playMenuBackgroundMusic();}
            switch (currentGameState) {
                case PLAYING:
                    handleInput(event);
                    break;
                case MENU:
                    handleMenuInput(event);
                    break;
                case LEVEL_SELECT:
                    if(!menu1sound){menu1sound=true;menu2sound=false;Mix_PlayChannel(7,menu1,0);}
                    handleLevelSelectInput(event);
                    break;
                case GUIDE:
                    if(!menu2sound){menu1sound=false;menu2sound=true;Mix_PlayChannel(7,menu1,0);}
                    Mix_PlayChannel(8,menu2,0);
                    handleGuideInput(event);
                    break;
            }
        }
        
        SDL_RenderClear(renderer);
        
        switch (currentGameState) {
            case MENU:
                renderMenu();
                break;
            case LEVEL_SELECT:
                renderLevelSelect();
                break;
            case GUIDE:
                renderGuide();
                break;
            case PLAYING:
            SDL_PumpEvents();
            const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);
            if(currentKeyStates[SDL_SCANCODE_SPACE]||currentKeyStates[SDL_SCANCODE_UP] || currentKeyStates[SDL_SCANCODE_DOWN] || currentKeyStates[SDL_SCANCODE_LEFT] || currentKeyStates[SDL_SCANCODE_RIGHT]){
                continuehandle = true;
            }
            else continuehandle = false;
            update();
            renderMap();
            for (const auto& wall : walls) renderWall(wall);
            renderTank(playerTank, 0);
            for (auto& enemy : enemyTanks) renderTank(enemy, 1);
            for (const auto& bullet : bullets) {
                if (bullet.active) {
                    if(bullet.isPlayerBullet) renderBullet(bullet, bulletFrames);
                        else renderBullet(bullet, bulletFrames2);
                    }
                }
                renderExplosions();
                
                // Hiển thị thời gian chơi
                Uint32 currentTime = gameCompleted ? completionTime : (SDL_GetTicks() - gameStartTime);
                string timeText = "Time: " + formatTime(currentTime);
                renderText(timeText, 10, 10, SDL_Color{255, 0, 0, 255});
                
                // Kiểm tra kết thúc game
                if (!playerTank.active || enemyTanks.empty()) {
                    stopBackgroundMusic();
                    bool showEndGameScreen = true;
                    Uint32 endGameStartTime = SDL_GetTicks();
                    
                    // Phát âm thanh kết thúc
                    Mix_HaltChannel(-1);
                    int channel = Mix_PlayChannel(0, gameCompleted ? victory : gameover, 0);
                    
                    while (showEndGameScreen) {
                        while (SDL_PollEvent(&event)) {
                            if (event.type == SDL_QUIT) {
                                showEndGameScreen = false;
                                running = false;
                            }
                            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
                                showEndGameScreen = false;
                                currentGameState = MENU;
                            }
                            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
                                showEndGameScreen = false;
                                currentGameState = MENU;
                                Initialize_playerTank(playerTank, SCREEN_WIDTH, SCREEN_HEIGHT, TANK_SIZE);
                                enemyTanks.clear();
                                bullets.clear();
                                explosions.clear();
                                gameCompleted = false;
                                mapData.clear();
                                
                            }
                        }
                        
                        SDL_RenderClear(renderer);
                        string endText = gameCompleted ? 
                        "Mission Complete! Time: " + formatTime(completionTime) : 
                        "Game Over! You were killed.";
                        
                        renderText(endText, SCREEN_WIDTH/2 - 150, SCREEN_HEIGHT/2 - 50, SDL_Color{255, 255, 255, 255});
                        renderText("Press ESC to return to menu", SCREEN_WIDTH/2 - 150, SCREEN_HEIGHT/2 + 20, SDL_Color{255, 255, 255, 255});
                        SDL_RenderPresent(renderer);
                        
                        SDL_Delay(16);
                    }
                }
                menu1sound = false;
                menu2sound = false;
                break;
            }
            
            SDL_RenderPresent(renderer);
            SDL_Delay(16);
        }
    
    cleanup();
    return 0;
}
