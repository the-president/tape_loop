#include "bmp_console.h"
#include <SDL2/SDL_image.h>
#include <cstring>
#include <algorithm>


box_size text_writer::write(int x,int y, const char* str)
{
	int index = 0;
	int box_x = x;
	int box_y = y;

	int w = 0;
	int line_w = 0;
	int h = font_info->box_height;

	while( str[index] != 0)
	{
		char letter = str[index];

		if(letter == '\n')
		{
			box_x = x;
			box_y += font_info->box_height;

			h += font_info->box_height;
			w = std::max(w,line_w);
			line_w = 0;
		}
		else
		{
			const letter_info& info = font_info->letters.at(letter);
			SDL_Rect dest = {box_x+ info.x_offset,box_y + info.y_offset, info.source.w, info.source.h};
			SDL_RenderCopy(rend,font_text,&info.source,&dest);

			box_x += info.box_width;
			line_w += info.box_width;
		}

		++index;
	}

	return {std::max(w,line_w),h};
}

box_size text_writer::bounds(const char* str)
{
	int w = 0;
	int line_w = 0;
	int h = font_info->box_height;

	for(int i=0;str[i] !=0;++i)
	{
		char letter = str[i];

		if(letter == '\n')
		{
			h += font_info->box_height;
			w = std::max(w,line_w);
			line_w = 0;
		}

		const letter_info& info = font_info->letters.at(letter);
		line_w += info.box_width;
	}

	return {std::max(w,line_w),h};
}

SDL_Texture* font_info::load_font_texture(SDL_Renderer* rend) const
{
	SDL_Surface* surf = IMG_Load(file_path);

	if(surf == NULL)
	{
	  printf("Unable to load image %s! SDL Error: %s\n", file_path, SDL_GetError());
	  return NULL;
	}

	SDL_Texture* texture = SDL_CreateTextureFromSurface(rend, surf);

	if(texture == NULL)
	{
		printf("Unable to create texture SDL Error: %s\n",SDL_GetError());
		return NULL;
	}

	//Get rid of old loaded surface
	SDL_FreeSurface(surf);

	return texture;
}