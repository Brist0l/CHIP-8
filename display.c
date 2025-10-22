#include<SDL3/SDL.h> 
#include<SDL3/SDL_main.h> 
#include<stdbool.h>
#include<stdio.h>
#include<stdlib.h>

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
};

bool game_init_sdl(struct Game *g);
bool game_load_media(struct Game *g);
bool game_new(struct Game **game);
void game_free(struct Game **game);
void game_events(struct Game *g);
void game_draw(struct Game *g);
bool clear_screen(struct Game *g);

bool game_init_sdl(struct Game *g){
	//printf("%d\n",SDL_FLAGS);
	if(!SDL_Init(SDL_FLAGS)){ // Inits the SDL system as a whole
		fprintf(stderr,"Error initialising SDL3: %s\n",SDL_GetError());
		return false;
	}

	g->window = SDL_CreateWindow(WINDOW_TITLE,WINDOW_WIDTH * SCALE,WINDOW_HEIGHT * SCALE,0);
	if(!g->window){
		fprintf(stderr,"Error Creating window: %s\n",SDL_GetError());
		return false;
	}

	g->renderer = SDL_CreateRenderer(g->window,NULL);
	if(!g->renderer){
		fprintf(stderr,"Error Creating renderer: %s\n",SDL_GetError());
		return false;
	}

	return true;
}

bool game_load_media(struct Game *g){

	if(!g->background){
		fprintf(stderr,"Error Creating renderer: %s\n",SDL_GetError());
		return false;
	}
	
	return true;
}

bool game_new(struct Game **game){
	*game = calloc(1,sizeof(struct Game));

	if(*game == NULL){
		fprintf(stderr,"Error while allocating memory\n");
		return false;
	}

	struct Game *g = *game; //Just to not modify the code beneath this

	if(!game_init_sdl(g)){
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
	
		printf("Quiting SDL\n");
	}
}

void game_events(struct Game *g){
		while(SDL_PollEvent(&(g->event))){
			//printf("Polling has started\n");
			switch (g->event.type){
				case SDL_EVENT_QUIT:
					printf("Quit has been pressed\n");
					g->is_running = false;
					break;
				case SDL_EVENT_KEY_DOWN:
					printf("Key has been pressed\n");
					switch(g->event.key.scancode){
						case SDL_SCANCODE_ESCAPE:
							printf("Escape has been pressed\n");
							g->is_running = false;
							break;
						case SDL_SCANCODE_Q:
							printf("Q has been pressed\n");
							g->is_running = false;
							break;
						default:
							break;

					}
					break;
				default:
					break;
			}
		}

}

void game_draw(struct Game *g){
	bool display[WINDOW_HEIGHT][WINDOW_WIDTH] = {0};
    //	display[5][5] = 1; // Just one pixel to test
	int draw[5] = {0xF0,0x80,0xF0,0x80,0x80};

	int bit_mask = 1;
	int num_count = 0;

	for(int i = 0;i < 5;i++){
		//printf("Incremented\n");
		for(int j = 0;j < 8;j++){
			//printf("Num is : %b\n",draw[num_count]);
			//printf("Shifted num is : %08b\n",draw[num_count] >> (7 - j));
			//printf("=> Putting %d at %dx%d\n",(draw[num_count] >> (7 - j)) & bit_mask,i,j);
			
			display[i][j] = (draw[num_count] >> (7 - j)) & bit_mask;

		}
		num_count++;
	}	
	
    	SDL_SetRenderDrawColor(g->renderer, 0, 0, 0, 255);
    	SDL_RenderClear(g->renderer); // clear the current rendering target with the drawing colour

    	SDL_SetRenderDrawColor(g->renderer, 255, 255, 255, 255);
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

void draw(struct Game *g,int x,int y,int data){
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

	static bool display[WINDOW_HEIGHT][WINDOW_WIDTH] = {0}; 
    	//display[y][x] = 1;

	int bit_mask = 1;
	int j = 0;

	for(int _x = x;_x <= x + 8;_x++){
		printf("Num is : %b\n",data);
		printf("Shifted num is : %08b\n",data >> (7 - j));
		printf("=> Putting %d at %dx%d\n",(data >> (7 - j)) & bit_mask,y,_x);

		display[y][_x] ^= ((data >> (7 - j++)) & bit_mask);
	}

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
	printf("Entered the clear_screen");
    	SDL_SetRenderDrawColor(g->renderer, 0, 0, 0, 255);
    	SDL_RenderClear(g->renderer);
    	SDL_RenderPresent(g->renderer); // update the rendering content
	
	return true;
}
