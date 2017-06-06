#include "gem.h"
#include <SDL2/SDL.h>
#include <ncurses.h>

#define SCREEN_WIDTH  800
#define SCREEN_HEIGHT 800

#define PROCESSOR_SPEED     2*1024*1024
#define SCREEN_REFRESH_RATE 60

#define CYCLES_PER_REFRESH (PROCESSOR_SPEED/SCREEN_REFRESH_RATE)

#define SCREEN_ADDR    0x200
#define NUM_PIXELS     0x400
#define PIXELS_PER_ROW 0x20
#define PIXEL_SIZE     (SCREEN_WIDTH/PIXELS_PER_ROW)

SDL_Window   *g_window = NULL;
SDL_Renderer *g_renderer = NULL;

void print_mos( gem_mos *m ) {
	printf("|---------------------|\n");
	printf("|       A: 0x%02X       |\n",  m->a);
	printf("|---------------------|\n");
	printf("|  X: 0x%02X |  Y: 0x%02X |\n", m->x, m->y);
	printf("|---------------------|\n");
	printf("| SR: 0x%02X | SP: 0x%02X |\n", m->sr, m->sp);
	printf("|---------------------|\n");
	printf("|     PC: 0x%04X      |\n", m->pc);
	printf("|---------------------|\n");
}

void
print_ncurses( gem_mos *m ) {
    move(0,0);
    printw("|---------------------|\n");
	printw("|       A: 0x%02X       |\n",  m->a);
    printw("|---------------------|\n");
	printw("|  X: 0x%02X |  Y: 0x%02X |\n", m->x, m->y);
	printw("|---------------------|\n");
	printw("| SR: 0x%02X | SP: 0x%02X |\n", m->sr, m->sp);
	printw("|---------------------|\n");
	printw("|     PC: 0x%04X      |\n", m->pc);
	printw("|---------------------|\n");
    printw("%s\n", gem_get_disasm()); 
}

int init() {
	
	/* Start by setting up the SDL display system */
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("Couldn't initialize SDL: %s\n", SDL_GetError());
		return 0;
	}

	/* Now create a window. Also creates a renderer for drawing. We'll   */
	/* use this a lot                                                    */
	if ( SDL_CreateWindowAndRenderer(
                SCREEN_WIDTH, 
                SCREEN_HEIGHT, 
                0, 
                &g_window, 
                &g_renderer
            )
        ) {
		printf("Couldn't create window and renderer: %s\n", SDL_GetError());
		return 0;
	}
	
	return 1;
}

int user_quit() {
	SDL_Event event;
	
	while(SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT) {
			return 1;
		}
	}

	return 0;
}

void pixel_code_to_color( GEM_MEMORY_BYTE pixel, uint8_t *r, uint8_t *g, 
        uint8_t *b ) {
    switch(pixel) {
    case 0x0:
        *r = *g = *b = 0;
        break;
    case 0x1:
        *r = *g = *b = 255;
        break; 
    case 0x2:
        *r = 136;
        *g = * b = 0;
        break;
    case 0x3:
        *r = 170;
        *g = 255;
        *b = 238;
        break;
    case 0x4:
        *r = 204;
        *g = 68;
        *b = 204;
        break;
    case 0x5:
        *r = 0;
        *g = 204;
        *b = 85;
        break;
    case 0x6:
        *r = 0;
        *g = 0;
        *b = 170;
        break;
    case 0x7:
        *r = 238;
        *g = 238;
        *b = 119;
        break;
    case 0x8:
        *r = 221;
        *g = 136;
        *b = 85;
        break;
    case 0x9:
        *r = 102;
        *g = 68;
        *b = 0;
        break;
    case 0xA:
        *r = 255;
        *g = 119;
        *b = 119;
        break;
    case 0xB:
        *r = 51;
        *g = 51;
        *b = 51;
        break;    
    case 0xC:
        *r = 119;
        *g = 119;
        *b = 119;
        break; 
    case 0xD:
        *r = 170;
        *g = 255;
        *b = 102;
        break;    
    case 0xE:
        *r = 0;
        *g = 136;
        *b = 255;
        break;  
    case 0xF:
        *r = 187;
        *g = 187;
        *b = 187;
        break;  
    default:
        *r = *g = *b = 0;
    }
}

void update_display( gem_mos *m ) {
	int i;
	GEM_MEMORY_BYTE pixel;
	uint8_t r,g,b;
	
	for( i=0; i<NUM_PIXELS; i++ ) {
		pixel = m->memory[SCREEN_ADDR+i];
       
        pixel_code_to_color( pixel, &r, &g, &b ); 

        SDL_Rect rect = { i%PIXELS_PER_ROW * PIXEL_SIZE, 
                i/PIXELS_PER_ROW * PIXEL_SIZE, 
                PIXEL_SIZE, 
                PIXEL_SIZE 
        };
        SDL_SetRenderDrawColor( g_renderer, r, g, b, 255 );
        SDL_RenderFillRect( g_renderer, &rect );
	}

    SDL_RenderPresent( g_renderer );
}

void quit()
{
	SDL_DestroyRenderer(g_renderer);
	g_renderer = NULL;
	
	//Destroy window
	SDL_DestroyWindow( g_window );
	g_window = NULL;

	//Quit SDL subsystems
	SDL_Quit();
}

void
run_graphical( gem_mos *m ) {
	uint32_t current_time;
   	init();

	while(!user_quit()) {
        current_time = SDL_GetTicks();
        gem_mos_run_for( m, CYCLES_PER_REFRESH );
        update_display(m);
        SDL_Delay( 1000/SCREEN_REFRESH_RATE - (SDL_GetTicks() - current_time));
	}
	
	quit();
}

void
run_headless( gem_mos *m ) {
    char quit = '\0';

    initscr();
    while(quit!= 'q') {
        if(gem_mos_step(m) < 0) {
            printw("ERROR: OPCODE: 0x%02X\n", gem_get_opcode()); 
            // printf("ERROR: OPCODE: 0x%02X\n", gem_get_opcode()); 
            timeout(-1);
            getch();
            break;
        }
        // printf("%s\n", gem_get_disasm());
        print_ncurses(m);
        timeout(1);
        quit = getch();
    }
    endwin();
}

int main( int argc, char *argv[] ) {
	gem_mos  mos;

	gem_mos_init(&mos);

	if(argc < 2) {
		return 1;
	}

	gem_mos_load_rom_at( &mos, argv[1], 0 );
    mos.pc = GEM_ROM_ADDR;

    run_headless( &mos ); 
		
    /* print the final processor state */
    print_mos( &mos );

	return 0;
}
