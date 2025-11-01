#include<SDL3/SDL.h> 
#include<SDL3/SDL_main.h> 
#include<stdbool.h>
#include<stdio.h>
#include<stdlib.h>

#include"chip_8.h"

#define SDL_FLAGS SDL_INIT_VIDEO
#define WINDOW_TITLE "Open Window"
#define WINDOW_WIDTH 64
#define WINDOW_HEIGHT 32
#define SCALE 20

struct Game{
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Texture *background;
	SDL_Event event;
	bool is_running;
	bool keypad[16];
};
bool display[WINDOW_HEIGHT][WINDOW_WIDTH] = {0}; 

bool game_init_sdl(struct Game *g);
bool game_new(struct Game **game);
void game_free(struct Game **game);
void game_events(struct Game *g,int* key);
bool draw(struct Game *g,int x,int y,int N,int data);
void render_screen(struct Game *g);
bool clear_screen(struct Game *g);

bool game_init_sdl(struct Game *g){
	//printf("%d\n",SDL_FLAGS);
	if(!SDL_Init(SDL_FLAGS)){ // Inits the SDL system as a whole
		if(debug_flag)
			fprintf(stderr,"Error initialising SDL3: %s\n",SDL_GetError());
		return false;
	}

	g->window = SDL_CreateWindow(WINDOW_TITLE,WINDOW_WIDTH * SCALE,WINDOW_HEIGHT * SCALE,0);
	if(!g->window){
		if(debug_flag)
			fprintf(stderr,"Error Creating window: %s\n",SDL_GetError());
		return false;
	}

	g->renderer = SDL_CreateRenderer(g->window,NULL);
	if(!g->renderer){
		if(debug_flag)
			fprintf(stderr,"Error Creating renderer: %s\n",SDL_GetError());
		return false;
	}

	return true;
}

bool game_new(struct Game **game){
	*game = calloc(1,sizeof(struct Game));

	if(*game == NULL){
		if(debug_flag)
			fprintf(stderr,"Error while allocating memory\n");
		return false;
	}

	struct Game *g = *game; //Just to not modify the code beneath this

	if(!game_init_sdl(g)){
		if(debug_flag)
			fprintf(stderr,"Failed to init SDL: %s\n",SDL_GetError());
		return false;
	}
	g->is_running = true;

	return true;
}

void game_free(struct Game **game){
	if(*game){
		struct Game *g = *game;
		if(g->window){
			SDL_DestroyWindow(g->window);
			g->window = NULL;
		}
		if(g->renderer){
			SDL_DestroyRenderer(g->renderer);
			g->window = NULL;
		}

		SDL_Quit();

		free(g);
		g = NULL;
		*game = NULL;
		
		if(debug_flag)
			printf("Quiting SDL\n");
	}
}

void game_events(struct Game *g,int* key){
		while(SDL_PollEvent(&(g->event))){
			switch (g->event.type){
				case SDL_EVENT_QUIT:
					//printf("Quit has been pressed\n");
					g->is_running = false;
					break;

				case SDL_EVENT_KEY_UP:
				case SDL_EVENT_KEY_DOWN:
                			bool isPressed = (g->event.type == (SDL_EVENT_KEY_DOWN));
                			switch (g->event.key.scancode){
						case SDL_SCANCODE_ESCAPE: g->is_running = false; break;
                    				case SDL_SCANCODE_X: g->keypad[0x0] = isPressed;break;
                    				case SDL_SCANCODE_1: g->keypad[0x1] = isPressed;break;
				                case SDL_SCANCODE_2: g->keypad[0x2] = isPressed;break;
                    				case SDL_SCANCODE_3: g->keypad[0x3] = isPressed;break;
                    				case SDL_SCANCODE_Q: g->keypad[0x4] = isPressed;break;
                    				case SDL_SCANCODE_W: g->keypad[0x5] = isPressed;break;
                    				case SDL_SCANCODE_E: g->keypad[0x6] = isPressed;break;
                    				case SDL_SCANCODE_A: g->keypad[0x7] = isPressed;break;
                    				case SDL_SCANCODE_S: g->keypad[0x8] = isPressed;break;
                    				case SDL_SCANCODE_D: g->keypad[0x9] = isPressed;break;
                    				case SDL_SCANCODE_Z: g->keypad[0xA] = isPressed;break;
                    				case SDL_SCANCODE_C: g->keypad[0xB] = isPressed;break;
                    				case SDL_SCANCODE_4: g->keypad[0xC] = isPressed;break;
                    				case SDL_SCANCODE_R: g->keypad[0xD] = isPressed;break;
                    				case SDL_SCANCODE_F: g->keypad[0xE] = isPressed;break;
                    				case SDL_SCANCODE_V: g->keypad[0xF] = isPressed;break;
					} 
			}
	}
}

bool draw(struct Game *g,int x,int y,int N,int data){
	/*The natural ways of rendering pixels is to first traverse the height and then the width.
	 *
	 *			(x)
	 *              <-------width-------->
	 *            ^ |~~~~~~~~~~~~~~~~~~~~|
	 *	      |	|		     |
	 *	      | |		     |
	 *   (y) height |       display	     |	     
	 *	      | |                    |
	 *	      | |                    |
	 *	      v |~~~~~~~~~~~~~~~~~~~~|
	 *
	 * So i.e. I will first go down and then go right. The top left is (0,0) and the bottom right
	 * is (max_x,max_y). Hence the display is defined in terms of display[height][widht] and the 
	 * pixels in display[y][x].
	 */

	bool vf_flag = 0;

	x %= 64;
	y %= 32;

	for(int i = 0; i < N; i++){
		for(int bit = 0; bit < 8; bit++){
    			int pixel = (memory[data + i] >> (7 - bit)) & 1;
			int x_pos = x + bit;
			int y_pos = y + i;
			
			if(debug_flag){
				printf("Num is : %b\n",data); 
				printf("Shifted num is : %08b\n",data >> (7 - bit)); 
				printf("=> Putting %d at %dx%d\n",(data >> (7 - bit)) & 1,y_pos,x_pos);
			}
		
			if(pixel && display[y_pos][x_pos])
            			vf_flag = 1;
		
        		display[y_pos][x_pos] ^= pixel;
	}
}
	return vf_flag;
}

void render_screen(struct Game *g){
    	SDL_SetRenderDrawColor(g->renderer, 0, 0, 0, 255);
    	SDL_RenderClear(g->renderer);

    	SDL_SetRenderDrawColor(g->renderer, 255, 255, 255, 255); // White colour
  	for(int y = 0; y < WINDOW_HEIGHT; y++) {
        	for(int x = 0; x < WINDOW_WIDTH; x++) {
            		if (display[y][x]) {
                		SDL_FRect rect = {x * SCALE, y * SCALE, SCALE, SCALE};
                		SDL_RenderFillRect(g->renderer, &rect);
            		}
        	}
    	}

    	SDL_RenderPresent(g->renderer); // update the rendering content
}

bool clear_screen(struct Game *g){
	memset(display, 0, sizeof(display));

    	SDL_SetRenderDrawColor(g->renderer, 0, 0, 0, 255);
    	SDL_RenderClear(g->renderer);
    	SDL_RenderPresent(g->renderer);
    	return true;
}
