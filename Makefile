snake: main.c
	cc -o snake main.c `sdl2-config --cflags --libs` -lSDL2_ttf
