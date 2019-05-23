#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include "sdl_system.h"

//Screen dimension constants
const int sdl_system::SCREEN_WIDTH = 1280;
const int sdl_system::SCREEN_HEIGHT = 960;

sdl_system sdl_system::init()
{
	if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0 )
	{
		printf( "SDL could not initialize! %s\n", SDL_GetError() );
		return {NULL,NULL};
	}

	//Set texture filtering to linear
	if( !SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" ) )
	{
		printf( "Warning: Linear texture filtering not enabled!" );
	}

	SDL_Window* window = SDL_CreateWindow( "Tape Loop", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
	
	if( window == NULL )
	{
		printf( "Window could not be created! SDL Error: %s\n", SDL_GetError() );
		return {NULL,NULL};
	}
	
	SDL_Renderer* renderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );
	
	if( renderer == NULL )
	{
		printf( "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
		return {NULL,NULL};
	}

	//Initialize PNG loading
	int img_flags = IMG_INIT_PNG;
	if( !( IMG_Init( img_flags ) & img_flags ) )
	{
		printf( "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError() );
		return {NULL,NULL};
	}

   if( TTF_Init() == -1 )
  {
  	printf( "SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError() );
    return {NULL,NULL};   
  }

	return {window,renderer};
}

void sdl_system::quit(sdl_system& sys)
{
	SDL_DestroyRenderer(sys.renderer);
	SDL_DestroyWindow(sys.window);

	//Quit SDL subsystems
	SDL_QuitSubSystem(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
	IMG_Quit();
	TTF_Quit();
	SDL_Quit();
}