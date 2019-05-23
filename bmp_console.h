#ifndef BMP_CONSOLE_DOT_H
#define BMP_CONSOLE_DOT_H

#include <map>
#include <SDL2/SDL.h>

struct box_size
{
	int w;
	int h;
};

struct letter_info
{
	int box_width;
	SDL_Rect source;

	int x_offset;
	int y_offset;
};

struct font_info
{
	SDL_Texture* load_font_texture(SDL_Renderer* rend) const;

	const char* file_path;

	int box_height; //this is constant?
	std::map<char,letter_info> letters;
};

struct text_writer
{
	SDL_Renderer* rend;
	SDL_Texture* font_text;
	const font_info* font_info;

	box_size write(int x,int y, const char* str);
	box_size bounds(const char* str);
};

#endif