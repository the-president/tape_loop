#include "graphical_interface.h"

#include <cmath>
#include <cstdio>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

// a quick no-op function (for callbacks)

//=============================================================================
// THE STATIC STUFF
//=============================================================================
button_src graphical_interface::play_button_bounds = {{0,0,32,32},{0,32,32,32}};
button_src graphical_interface::record_button_bounds = {{32,0,32,32},{32,32,32,32}};
button_src graphical_interface::feedback_button_bounds = {{64,0,32,32},{64,32,32,32}};
button_src graphical_interface::erase_button_bounds = {{96,0,32,32},{96,32,32,32}};
SDL_Rect graphical_interface::knob_body_src = {128,0,32,32};
SDL_Rect graphical_interface::knob_pointer_src = {128,32,32,32};

SDL_Texture* graphical_interface::sprites = NULL;

bool graphical_interface::init_static_assets(SDL_Renderer* rend)
{
	const char* sprite_sheet = "../assets/sprites.png";

	SDL_Surface* surf = IMG_Load(sprite_sheet);

	if(surf == NULL)
	{
	  printf("Unable to load image %s! SDL Error: %s\n", sprite_sheet, SDL_GetError());
	  return false;
	}

	SDL_Texture* texture = SDL_CreateTextureFromSurface(rend, surf);

	if(texture == NULL)
	{
		printf("Unable to create texture SDL Error: %s\n",SDL_GetError());
		return false;
	}

	//Get rid of old loaded surface
	SDL_FreeSurface(surf);

	graphical_interface::sprites = texture;

	return true;
}

//=============================================================================
// THE GUI STATE
//=============================================================================
graphical_interface::graphical_interface(SDL_Renderer* rend,SDL_Texture* font, const font_info* font_info):
rend(rend),
writer{rend,font,font_info},
panel_stack(),
offset_x(0),
offset_y(0),
last_rect{0,0,0,0}
{
	memset(temp_buff,0,sizeof(char)*256);
}

int graphical_interface::mouse_y_delta()
{
	return -(current.mouse_pos.y - prev.mouse_pos.y);
}

bool graphical_interface::end_click()
{
	return current.left_button.button == UP && prev.left_button.button == DOWN;
}

void graphical_interface::start_frame()
{
	SDL_SetRenderDrawColor(rend, 0x00, 0x00, 0x00, 0x00);
	SDL_RenderClear(rend);
}

void graphical_interface::end_frame()
{
	//transfer the gui state
	std::memcpy(&prev,&current,sizeof(gui_state));
	SDL_RenderPresent(rend);
}

void graphical_interface::process_event(SDL_Event& ev)
{
	switch(ev.type)
	{
		case SDL_MOUSEBUTTONDOWN:


			if(ev.button.button == SDL_BUTTON_LEFT)
			{
				current.left_button.button = DOWN;
				current.left_button.down_pos = SDL_Point{ev.button.x,ev.button.y};
			}

		break;

		case SDL_MOUSEBUTTONUP:

			if(ev.button.button == SDL_BUTTON_LEFT)
			{
				current.left_button.button = UP;
				current.left_button.up_pos = SDL_Point{ev.button.x,ev.button.y};
			}

		break;

		case SDL_MOUSEMOTION:
			current.mouse_pos = SDL_Point{ev.motion.x,ev.motion.y};

		break;

		default:
		break;
	}
}

void graphical_interface::reset_state()
{
	memset(temp_buff,0,sizeof(char)*256);
	memset(&current,0,sizeof(gui_state));
	memset(&prev,0,sizeof(gui_state));
}

//=============================================================================
// LAYOUT HELPOS!
//=============================================================================
void graphical_interface::panel(int x,int y, int w, int h)
{
	panel_info panel{x+offset_x,y+offset_y,{x + offset_x, y +offset_y,w,h}};
	last_rect = {0,0,0,0};

	if(panel_stack.empty())
	{
		SDL_RenderSetClipRect(rend,&panel.clipper);
	}
	else
	{
		SDL_Rect desired_bounds = panel.clipper;
		panel_info& top = panel_stack.top();
		SDL_IntersectRect(&desired_bounds,&top.clipper,&panel.clipper);
		SDL_RenderSetClipRect(rend,&panel.clipper);
	}

	panel_stack.push(panel);

	SDL_SetRenderDrawColor(rend, 0x55, 0xFF, 0xFF, 0x00);
	SDL_RenderDrawRect(rend,&panel.clipper);

	offset_x = panel.ox;
	offset_y = panel.oy;
}

void graphical_interface::panel_end()
{
	if(panel_stack.empty())
	{
		return;
	}

	panel_info& panel_bounds = panel_stack.top();
	panel_stack.pop();

	//ok we have to correct the rect for the offset
	last_rect = panel_bounds.clipper;

	if(panel_stack.empty())
	{
		SDL_RenderSetClipRect(rend,NULL);
		offset_x = 0;
		offset_y =0;
	}
	else
	{
		panel_info& top = panel_stack.top();
		SDL_RenderSetClipRect(rend,&top.clipper);
		offset_x = top.ox;
		offset_y = top.oy;

		//ok we have to correct the rect for the offset
		last_rect.x -= offset_x;
		last_rect.y -= offset_y;
	}
}

SDL_Rect graphical_interface::get_last_rect()
{
	return last_rect;
}

void graphical_interface::set_layout_rect(int x,int y, int w, int h)
{
	last_rect = {x,y,w,h};
}

int graphical_interface::left_end()
{
	return last_rect.x;
}

int graphical_interface::right_end()
{
	return last_rect.x + last_rect.w;
}

int graphical_interface::top()
{
	return last_rect.y;
}

int graphical_interface::bottom()
{
	return last_rect.y + last_rect.h;
}

int graphical_interface::center_x()
{
	return last_rect.x + (last_rect.w/2);
}

int graphical_interface::center_y()
{
	return last_rect.y + (last_rect.h/2);
}

//=============================================================================
// PICTURE BUTTONS
//=============================================================================
bool graphical_interface::img_button(int x,int y, SDL_Rect& off_src,SDL_Rect& on_src)
{
	SDL_Rect bounds{x+offset_x,y+offset_y,32,32};

	last_rect = SDL_Rect{x,y,32,32};

	//ok if it's clicked down fill the rect
	if(current.left_button.button == DOWN && SDL_PointInRect(&current.left_button.down_pos,&bounds))
	{
		SDL_RenderCopy(rend,sprites,&on_src,&bounds);
	}
	else
	{
		SDL_RenderCopy(rend,sprites,&off_src,&bounds);
	}

	if(end_click() && SDL_PointInRect(&current.left_button.down_pos,&bounds) && SDL_PointInRect(&current.left_button.up_pos,&bounds))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool graphical_interface::img_state_button(int x,int y, SDL_Rect& off_src,SDL_Rect& on_src,bool state)
{
	SDL_Rect bounds{x+offset_x,y+offset_y,32,32};
	last_rect = SDL_Rect{x,y,32,32};

	//ok if it's clicked down fill the rect
	if(state)
	{
		SDL_RenderCopy(rend,sprites,&on_src,&bounds);
	}
	else
	{
		SDL_RenderCopy(rend,sprites,&off_src,&bounds);
	}

	if(end_click() && SDL_PointInRect(&current.left_button.down_pos,&bounds) && SDL_PointInRect(&current.left_button.up_pos,&bounds))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool graphical_interface::play_button(int x,int y,bool& state)
{
	return img_state_button(x,y,play_button_bounds.off,play_button_bounds.on,state);
}

bool graphical_interface::record_button(int x,int y,bool& state)
{
	return img_state_button(x,y,record_button_bounds.off,record_button_bounds.on,state);
}

bool graphical_interface::erase_button(int x,int y,bool& state)
{
	return img_state_button(x,y,erase_button_bounds.off,erase_button_bounds.on,state);
}

bool graphical_interface::feedback_button(int x,int y,bool& state)
{
	return img_state_button(x,y,feedback_button_bounds.off,feedback_button_bounds.on,state);
}
//=============================================================================

//=============================================================================
// WIDGIES
//=============================================================================
bool graphical_interface::button(int x,int y, const char* label)
{
	box_size label_bounds = writer.bounds(label);
	SDL_Rect bounds{x+offset_x,y+offset_y,label_bounds.w+4,label_bounds.h+4};

	last_rect = SDL_Rect{x,y,bounds.w,bounds.h};

	SDL_SetRenderDrawColor(rend, 0x55, 0xFF, 0xFF, 0x00);
	//ok if it's clicked down fill the rect
	if(current.left_button.button == DOWN && SDL_PointInRect(&current.left_button.down_pos,&bounds))
	{
		SDL_RenderFillRect(rend,&bounds);
	}
	else
	{
		SDL_RenderDrawRect(rend,&bounds);
	}
	
	writer.write(bounds. x+ 2, bounds.y+2,label);

	if(end_click() && SDL_PointInRect(&current.left_button.down_pos,&bounds) && SDL_PointInRect(&current.left_button.up_pos,&bounds))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool graphical_interface::fs_button(int x,int y, int w, int h, const char* label)
{
	SDL_Rect bounds{x+offset_x,y+offset_y,w,h};
	last_rect = SDL_Rect{x,y,w,h};

	SDL_SetRenderDrawColor(rend, 0x55, 0xFF, 0xFF, 0x00);
	//ok if it's clicked down fill the rect
	if(current.left_button.button == DOWN && SDL_PointInRect(&current.left_button.down_pos,&bounds))
	{
		SDL_RenderFillRect(rend,&bounds);
	}
	else
	{
		SDL_RenderDrawRect(rend,&bounds);
	}
	
	if(label != NULL)
	{
		int mid_y = bounds.y + h/2;
		int mid_x = bounds.x + w/2;
		box_size sz = writer.bounds(label);
		int write_x = mid_x - sz.w/2;
		int write_y = mid_y - sz.h/2;

		writer.write(write_x, write_y,label);	
	}
	
	if(end_click() && SDL_PointInRect(&current.left_button.down_pos,&bounds) && SDL_PointInRect(&current.left_button.up_pos,&bounds))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool graphical_interface::select_button(int x, int y, const char* label, bool selected)
{
	box_size label_bounds = writer.bounds(label);
	SDL_Rect bounds{x+offset_x,y+offset_y,label_bounds.w+4,label_bounds.h+4};

	last_rect = SDL_Rect{x,y,bounds.w,bounds.h};

	SDL_SetRenderDrawColor(rend, 0x00, 0xaa, 0xaa, 0x00);

	if(selected)
	{
		SDL_RenderFillRect(rend,&bounds);
	}
	else
	{
		SDL_RenderDrawRect(rend,&bounds);
	}

	writer.write(bounds.x+ 2, bounds.y+2,label);

	if(end_click() && SDL_PointInRect(&current.left_button.down_pos,&bounds) && SDL_PointInRect(&current.left_button.up_pos,&bounds))
	{
		return true;
	}
	else
	{
		return false;
	}
}

void graphical_interface::double_bar(int x,int y,double& val,double min, double max)
{
	SDL_Rect bounds{x+offset_x,y+offset_y,100,30};
	last_rect = SDL_Rect{x,y,bounds.w,bounds.h};

	double range = max - min;

	SDL_SetRenderDrawColor(rend, 0x55, 0xFF, 0xFF, 0x00);
	SDL_RenderDrawRect(rend,&bounds);

	SDL_SetRenderDrawColor(rend, 0x00, 0xaa, 0xaa, 0x00);
	if(current.left_button.button == DOWN && SDL_PointInRect(&current.left_button.down_pos,&bounds))
	{
		int xpos = current.mouse_pos.x;

		if(xpos > (bounds.x + bounds.w-1))
		{
			val = max;
		}
		else if(xpos <= bounds.x+1)
		{
			val = min;
		}
		else 
		{
			val = min + (range * (double)(xpos-(bounds.x+1))/(bounds.w-2) );
		}
	}
		
	int fill_w;

	if( val <= min)
	{
		fill_w = 0;
	}
	else if (val >= max)
	{
		fill_w = bounds.w-2;
	}
	else
	{
		double fill = (double)(bounds.w-2) * ((val-min)/range);
		fill_w = (int)std::trunc(fill);
	}

	SDL_Rect in_bounds{bounds.x+1,bounds.y+1,fill_w,bounds.h-2};
	SDL_RenderFillRect(rend,&in_bounds);

	std::snprintf(temp_buff,12,"%5.4f",val);
	writer.write(bounds.x + ((bounds.w/2) - 8), bounds.y + ((bounds.h/2) - 8),temp_buff);
}

void graphical_interface::double_knob(int x, int y,double& val,double min, double max,double range_div)
{
	double range = max - min;

	SDL_Rect bounds = {x+offset_x,y+offset_y,32,32};
	last_rect = SDL_Rect{x,y,bounds.w,bounds.h};

	if(current.left_button.button == DOWN && SDL_PointInRect(&current.left_button.down_pos,&bounds))
	{
		int delta = mouse_y_delta();
		double inc = range/range_div;


		val = std::clamp( val + (delta*inc),min,max);
	}
		
	double angle;
	double base = -135;
	double max_rot = 270;

	if( val <= min)
	{
		angle = base;
	}
	else if (val >= max)
	{
		angle = base+max_rot;
	}
	else
	{
		angle = base + (((val-min)/range) * max_rot); 
	}

	SDL_RenderCopy(rend,sprites,&knob_body_src,&bounds);
	SDL_RenderCopyEx(rend, sprites, &knob_pointer_src, &bounds, angle, NULL, SDL_FLIP_NONE);
}

void graphical_interface::int_bar(int x,int y, int& val,int min, int max)
{
	SDL_Rect bounds{x+offset_x,y+offset_y,100,30};
	last_rect = SDL_Rect{x,y,bounds.w,bounds.h};

	double range = max - min;

	SDL_SetRenderDrawColor(rend, 0x55, 0xFF, 0xFF, 0x00);
	SDL_RenderDrawRect(rend,&bounds);

	SDL_SetRenderDrawColor(rend, 0x00, 0xaa, 0xaa, 0x00);
	if(current.left_button.button == DOWN && SDL_PointInRect(&current.left_button.down_pos,&bounds))
	{
		int xpos = current.mouse_pos.x;

		if(xpos > (bounds.x + bounds.w-1))
		{
			val = max;
		}
		else if(xpos <= bounds.x+1)
		{
			val = min;
		}
		else 
		{
			val = min + (range * (xpos-(bounds.x+1))/(bounds.w-2) );
		}
	}
		
	int fill_w;

	if( val <= min)
	{
		fill_w = 0;
	}
	else if (val >= max)
	{
		fill_w = bounds.w-2;
	}
	else
	{
		fill_w = (bounds.w-2) * ((val-min)/range);
	}

	SDL_Rect in_bounds{bounds.x+1,bounds.y+1,fill_w,bounds.h-2};
	SDL_RenderFillRect(rend,&in_bounds);

	std::snprintf(temp_buff,12,"%d",val);
	writer.write(bounds.x + ((bounds.w/2) - 8), bounds.y + ((bounds.h/2) - 8),temp_buff);
}

void graphical_interface::int_knob(int x, int y, int& val,int min, int max, int mult)
{
	double range = max - min;

	SDL_Rect bounds = {x+offset_x,y+offset_y,32,32};
	last_rect = SDL_Rect{x,y,bounds.w,bounds.h};

	if(current.left_button.button == DOWN && SDL_PointInRect(&current.left_button.down_pos,&bounds))
	{
		int delta = mouse_y_delta();
		val = std::clamp( val + (delta*mult),min,max);
	}
		
	double angle;
	double base = -135;
	double max_rot = 270;

	if( val <= min)
	{
		angle = base;
	}
	else if (val >= max)
	{
		angle = base+max_rot;
	}
	else
	{
		angle = base + (( (double)(val-min)/range) * max_rot); 
	}

	SDL_RenderCopy(rend,sprites,&knob_body_src,&bounds);
	SDL_RenderCopyEx(rend, sprites, &knob_pointer_src, &bounds, angle, NULL, SDL_FLIP_NONE);
}

//=============================================================================
// SPECIAL WIDGIES
//=============================================================================
// IDK Why but this is faster than std::max / std::min
static inline double flmax(double a,double b)
{
	if(a>b)
	{
		return a;
	}
	else
	{
		return b;
	}
}

static inline double flmin(double a,double b)
{
	if(a<b)
	{
		return a;
	}
	else
	{
		return b;
	}
}

void graphical_interface::tape_view(int x, int y, tape_lop& lop)
{
	int abs_x = x+offset_x;
	int abs_y = y+offset_y;

	int const w = 1274;
	int const h = 192;

	SDL_Rect old_clipper{0,0,0,0};
	SDL_RenderGetClipRect(rend,&old_clipper);

	SDL_Rect bounds = {abs_x,abs_y,w,h};
	SDL_Rect in_bounds = {abs_x + 2,abs_y + 2,w-2,h-2};
	last_rect = {x,y,w,h};

	//ok first lets get the window
	int window_start = 0;
	int window_end = lop.tape.size();

	int samps_per_line = (window_end - window_start)/in_bounds.w;

	//draw the outline
	SDL_SetRenderDrawColor(rend, 0x55, 0x55, 0xFF, 0x00);
	SDL_RenderDrawRect(rend,&bounds);

	SDL_RenderSetClipRect(rend,&in_bounds);
	if(samps_per_line == 0 )
	{
		//we will have a separate drawing path here
	}
	else if (samps_per_line > 0)
	{
		int write_bucket = (lop.write_head/samps_per_line);

		int mid_y = in_bounds.y + (in_bounds.h/2);

		SDL_SetRenderDrawColor(rend, 0xFF, 0xFF, 0x55, 0x00);
		int write_x = in_bounds.x + write_bucket;
		SDL_RenderDrawLine(rend, write_x ,in_bounds.y+in_bounds.h, write_x, in_bounds.y);	
		SDL_SetRenderDrawColor(rend, 0xaa, 0x00, 0x00, 0x00);

		for(int bucket=0;bucket<in_bounds.w;++bucket)
		{
			double top = 0;
			double bottom = 0;
			int buck_base = bucket*samps_per_line;

			for(int i=0;i<samps_per_line;++i)
			{
				int tape_pos =  buck_base + i;
				double tape_val = lop.tape.at(tape_pos);

				top=flmax(top,tape_val);
				bottom=flmin(bottom,tape_val);
			}

			int x = in_bounds.x + bucket;
			int tval = (int)std::trunc( (top) * (in_bounds.h/2));
			int bval = (int)std::trunc( (bottom) * (in_bounds.h/2));
			SDL_RenderDrawLine(rend,x,mid_y - bval,x,mid_y-tval);	
		}

		for(read_head& rh : lop.read_heads)
		{
			if(rh.active)
			{
				int buck = rh.position(lop.write_head,lop.tape.size()) / samps_per_line;
				int xpos = in_bounds.x + buck;
				SDL_SetRenderDrawColor(rend, 0xaa, 0x00, 0xaa, 0x00);
				SDL_RenderDrawLine(rend,xpos,in_bounds.y+in_bounds.h,xpos,in_bounds.y);		
			}
		}
	}

	SDL_RenderSetClipRect(rend,&old_clipper);
}

//=============================================================================
// LABELS
//=============================================================================
void graphical_interface::flat_label(int x,int y,const char* text)
{
	box_size bx = writer.write(x+offset_x,y+offset_y,text);	
	last_rect = {x,y,bx.w,bx.h};
}

void graphical_interface::format_label(int x, int y, const char* format, ...)
{
	va_list argptr;
  va_start(argptr, format);
  vsnprintf(temp_buff,256, format, argptr);
  va_end(argptr);

  box_size bx = writer.write(x+offset_x,y+offset_y,temp_buff);
	last_rect = {x,y,bx.w,bx.h};
}

void graphical_interface::state_label(int x,int y,const char* true_text, const char* false_text,bool& state)
{
	box_size bx;

	if(state)
	{
		bx = writer.write(x,y+offset_y,true_text);
		
	}
	else
	{
		bx = writer.write(x,y+offset_y,false_text);	
	}

	last_rect = {x,y,bx.w,bx.h};
}

void graphical_interface::double_label(int x,int y,const char* pre_text,double& val)
{
	std::snprintf(temp_buff,256,"%s %5.4f",pre_text,val);
	box_size bx = writer.write(x+offset_x,y+offset_y,temp_buff);
	last_rect = {x,y,bx.w,bx.h};
}

void graphical_interface::int_label(int x,int y,const char* pre_text,int& val)
{
	std::snprintf(temp_buff,256,"%s %d",pre_text,val);
	box_size bx = writer.write(x+offset_x,y+offset_y,temp_buff);
	last_rect = {x,y,bx.w,bx.h};
}