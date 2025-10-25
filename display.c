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
uint8_t game_events(struct Game *g);
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

uint8_t game_events(struct Game *g){
		while(SDL_PollEvent(&(g->event))){
			switch (g->event.type){
				case SDL_EVENT_QUIT:
					//printf("Quit has been pressed\n");
					g->is_running = false;
					break;
				case SDL_EVENT_KEY_DOWN:
					//printf("Key has been pressed\n");
					switch(g->event.key.scancode){
						case SDL_SCANCODE_ESCAPE:
							printf("Escape has been pressed\n");
							g->is_running = false;
							break;
						case SDL_SCANCODE_1:
							printf("1 has been pressed\n");
							return 0x0;
						case SDL_SCANCODE_2:
							printf("2 has been pressed\n");
							return 0x1;
						case SDL_SCANCODE_3:
							printf("3 has been pressed\n");
							return 0x2;
						case SDL_SCANCODE_4:
							printf("4 has been pressed\n");
							return 0x3;
						case SDL_SCANCODE_Q:
							printf("Q has been pressed\n");
							return 0x4;
						case SDL_SCANCODE_W:
							printf("W has been pressed\n");
							return 0x5;
						case SDL_SCANCODE_E:
							printf("E has been pressed\n");
							return 0x6;
						case SDL_SCANCODE_R:
							printf("R has been pressed\n");
							return 0x7;
						case SDL_SCANCODE_A:
							printf("A has been pressed\n");
							return 0x8;
						case SDL_SCANCODE_S:
							printf("S has been pressed\n");
							return 0x9;
						case SDL_SCANCODE_D:
							printf("D has been pressed\n");
							return 0xA;
						case SDL_SCANCODE_F:
							printf("F has been pressed\n");
							return 0xB;
						case SDL_SCANCODE_Z:
							printf("Z has been pressed\n");
							return 0xC;
						case SDL_SCANCODE_X:
							printf("X has been pressed\n");
							return 0xD;
						case SDL_SCANCODE_C:
							printf("C has been pressed\n");
							return 0xE;
						case SDL_SCANCODE_V:
							printf("V has been pressed\n");
							return 0xF;
						default:
							return 16;

					}
					break;
				default:
					return 16;
			}
		}

		return 16;
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
	bool vf_flag = 0;

	for(int _x = x;_x <= x + 8;_x++){
		printf("Num is : %b\n",data);
		printf("Shifted num is : %08b\n",data >> (7 - j));
		printf("=> Putting %d at %dx%d\n",(data >> (7 - j)) & bit_mask,y,_x);

		display[y][_x] ^= ((data >> (7 - j++)) & bit_mask);
		if(display[y][_x] == 0 && vf_flag == 0)
			vf_flag = 1;
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
