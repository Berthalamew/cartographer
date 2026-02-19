#pragma once

/* enums */

enum e_interface_tag : uint8
{
	_interface_spinner,                         // bitm
	_interface_obsolete,                        // bitm
	_interface_screen_color_table,              // colo
	_interface_hud_color_table,                 // colo
	_interface_editor_color_table,              // colo
	_interface_dialog_color_table,              // colo
	_interface_hud_globals,                     // hudg
	_interface_motion_sensor_sweep_bitmap,      // bitm
	_interface_motion_sensor_sweep_bitmap_mask, // bitm
	_interface_multiplayer_hud_bitmap,          // bitm
	_interface_unknown_field,
	_interface_hud_digits_definition,           // hud#
	_interface_motion_sensor_blip_bitmap,       // bitm
	_interface_interface_goo_map_1,             // bitm
	_interface_interface_goo_map_2,             // bitm
	_interface_interface_goo_map_3,             // bitm
	NUMBER_OF_INTERFACE_TAGS
};

/* public code */

void apply_interface_hooks(void);

datum interface_get_tag_index(e_interface_tag interface_tag_index);

void __cdecl interface_draw_bitmap(
	struct bitmap_data* bitmap,
	const real_point2d* position,
	const real_rectangle2d* rectangle,
	real32 a4,
	real32 a5,
	real32 a6);
