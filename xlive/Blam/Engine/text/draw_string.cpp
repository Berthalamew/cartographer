#include "stdafx.h"
#include "draw_string.h"


/* public code */

void __cdecl draw_string_get_color(real_argb_color* color)
{
	INVOKE(0x98955, 0, draw_string_get_color, color);
	return;
}

void __cdecl draw_string_set_color(real_argb_color const* color)
{
	INVOKE(0x98A00, 0, draw_string_set_color, color);
	return;
}

void __cdecl draw_string_set_shadow_color(real_argb_color const* color)
{
	INVOKE(0x98A38, 0, draw_string_set_shadow_color, color);
	return;
}

void draw_string_set_player_color(real_argb_color const* color)
{
	INVOKE(0x9884B, 0, draw_string_set_player_color, color);
	return;
}

void __cdecl draw_string_set_font(int32 font_index)
{
	INVOKE(0x98A87, 0, draw_string_set_font, font_index);
	return;
}

void __cdecl draw_string_set_format(int32 style, int32 justification, uint32 flags, bool wrap_horizontally)
{
	INVOKE(0x98A9D, 0, draw_string_set_format, style, justification, flags, wrap_horizontally);
	return;
}

void __cdecl draw_string_set_tab_stops(const int16* tab_stops, int16 count)
{
	INVOKE(0x987F8, 0x0, draw_string_set_tab_stops, tab_stops, count);
	return;
}

void __cdecl draw_string_set_draw_mode(int32 font, int32 style, int32 justification, uint32 flags, real_argb_color const* color, real_argb_color const* shadow_color, bool wrap_horizontally)
{
	INVOKE(0x98AC4, 0, draw_string_set_draw_mode, font, style, justification, flags, color, shadow_color, wrap_horizontally);
	return;
}

bool __cdecl draw_string_set_string(const wchar_t* string)
{
	return INVOKE(0x99D59, 0, draw_string_set_string, string);
}

void __cdecl draw_string_set_position(int16 x, int16 y)
{
	INVOKE(0x98833, 0, draw_string_set_position, x, y);
	return;
}

void __cdecl draw_string_compute_bounds(const rectangle2d* bounds, const wchar_t* string, rectangle2d* text_bounds, rectangle2d* cursor_bounds, real32 scale)
{
	INVOKE(0x99D97, 0, draw_string_compute_bounds, bounds, string, text_bounds, cursor_bounds, scale);
	return;
}

void __cdecl draw_string_reset(void)
{
	INVOKE(0x98CDC, 0, draw_string_reset);
	return;
}
