all: build run

build:
	g++ -o main.exe \
	src/main.cpp src/Game.cpp src/Wall.cpp src/Bullets.cpp src/Tank.cpp src/Explosion.cpp \
	-Isdl/include/SDL2 -Lsdl/lib \
	-lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf
run:
	.\main.exe