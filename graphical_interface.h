#ifndef GRAPHICAL_INTERFACE_DOT_H
#define GRAPHICAL_INTERFACE_DOT_H

#include <SDL2/SDL.h>
#include <stack>
#include <cstring> //for memcpy
#include "bmp_console.h"
#include "tape_lop.h"

struct tape_lop;

enum button_state
{
	UP,
	DOWN
};

struct mouse_state
{
	button_state button = UP;
	SDL_Point down_pos;
	SDL_Point up_pos;
};

struct gui_state
{
	mouse_state left_button;
	SDL_Point mouse_pos;
};

struct button_src
{
	SDL_Rect off;
	SDL_Rect on;
};

struct panel_info
{
	int ox;
	int oy;
	SDL_Rect clipper;
};

class graphical_interface
{
	protected:
		static SDL_Texture* sprites;
		
		static button_src play_button_bounds;
		static button_src record_button_bounds;
		static button_src erase_button_bounds;
		static button_src feedback_button_bounds;
		static SDL_Rect knob_body_src;
		static SDL_Rect knob_pointer_src;

		gui_state prev;
		gui_state current;
		SDL_Renderer* rend;

		text_writer writer;

		char temp_buff[256]; //this is at a "safe range"

		bool end_click();
		int mouse_y_delta();

		bool img_button(int x,int y, SDL_Rect& off_src,SDL_Rect& on_src);
		bool img_state_button(int x,int y, SDL_Rect& off_src,SDL_Rect& on_src,bool state);

		//for panels
		std::stack<panel_info> panel_stack;
		int offset_x;
		int offset_y;

		//for layout help
		SDL_Rect last_rect;

	public:

		SDL_Rect get_last_rect();
		void set_layout_rect(int x,int y, int w, int h);
		int left_end();
		int right_end();
		int top();
		int bottom();
		int center_x();
		int center_y();

		static bool init_static_assets(SDL_Renderer* rend);

		void start_frame();	
		void end_frame();	
		void reset_state();
		void process_event(SDL_Event& ev);

		//layouts
		void panel(int x,int y, int w, int h);
		void panel_end();
		
		//widgies
		bool button(int x,int y, const char* label);
		bool fs_button(int x, int y, int w,int h,const char* label);
		bool select_button(int x, int y, const char* label, bool selected);

		void float_bar(int x,int y, float& val,float min, float max);
		void float_knob(int x, int y, float& val,float min, float max,float range_div=200);

		void int_bar(int x,int y, int& val,int min, int max);
		void int_knob(int x, int y, int& val,int min, int max, int mult = 1);

		bool play_button(int x,int y,bool& state); //32-32
		bool record_button(int x,int y,bool& state); //32-32
		bool erase_button(int x,int y,bool& state); //32-32
		bool feedback_button(int x,int y,bool& state); //32-32

		void tape_view(int x, int y, tape_lop& lop); //800-160

		//labels
		void flat_label(int x,int y,const char* text);
		void format_label(int x, int y, const char* format, ...);
		void state_label(int x,int y,const char* true_text, const char* false_text,bool& state);
		void float_label(int x,int y,const char* pre_text, float& val);
		void int_label(int x,int y,const char* pre_text, int& val);

		graphical_interface(SDL_Renderer* rend,SDL_Texture* font, const font_info* font_info);
};

#endif