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
	bool keypad[16];
};

bool game_init_sdl(struct Game *g);
bool game_load_media(struct Game *g);
bool game_new(struct Game **game);
void game_free(struct Game **game);
void game_events(struct Game *g,int* key);
void game_draw(struct Game *g);
bool draw(struct Game *g,int x,int y,int N,int data);
void render_screen(struct Game *g);
bool clear_screen(struct Game *g);

#endif
