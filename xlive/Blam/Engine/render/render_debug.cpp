#include "stdafx.h"
#include "render_debug.h"

#include "game/game.h"
#include "hs/hs_runtime.h"

/* structures */

struct cache_entry
{
	int16 type;
	int16 layer;
	real32 sort_key;
	union
	{
		// _render_debug_type_circle
		struct { real_plane3d plane; int16 projection_axis; bool projection_sign; real_point2d center;  real32 radius; real_argb_color color; real32 offset; } circle;
		// _render_debug_type_point
		struct { real_point3d point; real32 scale; real_argb_color color; } point;
		// _render_debug_type_line
		struct { real_point3d point0; real_point3d point1; real_argb_color color0; real_argb_color color1; } line;
		// _render_debug_type_sphere
		struct { real_point3d center; real32 radius; real_argb_color color; } sphere;
		// _render_debug_type_cylinder
		struct { real_point3d base; real_vector3d height; real32 width; real_argb_color color; } cylinder;
		// _render_debug_type_pill
		struct { real_point3d base; real_vector3d height; real32 width; real_argb_color color; } pill;
		// _render_debug_type_box, _render_debug_type_box_outline
		struct { real_rectangle3d bounds; real_argb_color color; } box;
		// _render_debug_type_triangle
		struct { real_point3d point0; real_point3d point1; real_point3d point2; real_argb_color color; } triangle;
		// _render_debug_type_string
		struct  { int32 string_index; int16 tab_stops[16]; int16 tab_stop_count; } string;
		// _render_debug_type_string_at_point
		struct { int32 string_index; real_point3d point; real_argb_color color; } string_at_point;
		// _render_debug_type_box2d_outline
		struct { real_rectangle2d bounds; real_argb_color color; } box2d_outline;
	};
};


/* prototypes */

static void render_debug_add_cache_entry(int16 type, ...);

/* public code */

void render_debug(void)
{
	if (game_in_progress())
	{
		render_debug_scripting();
	}
	return;
}

void render_debug_string(const char* string)
{
	ASSERT(string);
	render_debug_add_cache_entry(_render_debug_type_string, string);
	return;
}

/* private code */

static void render_debug_add_cache_entry(int16 type, ...)
{
	// TODO: implement
	return;
}
