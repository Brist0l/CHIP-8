#ifndef DISPLAY_H
#define DISPLAY_H

#include<SDL3/SDL.h> 
#include<SDL3/SDL_main.h>

struct Game{
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Texture *background;
	SDL_Event event;
	bool is_running;
};

bool game_new(struct Game **game);
void game_run(struct Game *g);
void game_free(struct Game **game);
void game_draw(struct Game *g);

#endif
