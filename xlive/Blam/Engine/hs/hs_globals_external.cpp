#include "stdafx.h"
#include "hs_globals_external.h"

#include "main/main.h"
#include "main/main_game.h"
#include "main/main_render.h"
#include "main/main_time.h"
#include "simulation/simulation.h"

/* macros */

#ifdef HS_COMPILER_ENABLED
#define HS_GLOBAL_EXTERNAL_CREATE(global_name, name, type, address) \
static const hs_global_external (global_name) =	\
{												\
	(name),										\
	(type),										\
	(address)									\
}
#else
#define HS_GLOBAL_EXTERNAL_CREATE(global_name, name, type, address) \
static const hs_global_external (global_name) =	\
{												\
	(type),										\
	(address)									\
}
#endif

/* constants */

HS_GLOBAL_EXTERNAL_CREATE(debug_no_drawing_definition, "debug_no_drawing", _hs_type_boolean, &debug_no_drawing);
HS_GLOBAL_EXTERNAL_CREATE(debug_force_all_player_views_to_default_definition, "debug_force_all_player_views_to_default_player", _hs_type_boolean, &debug_force_all_player_views_to_default_player);
HS_GLOBAL_EXTERNAL_CREATE(debug_render_freeze_definition, "debug_render_freeze", _hs_type_boolean, &debug_render_freeze);
HS_GLOBAL_EXTERNAL_CREATE(debug_render_horizontal_splitscreen_definition, "debug_render_horizontal_splitscreen", _hs_type_boolean, &debug_render_horizontal_splitscreen);
HS_GLOBAL_EXTERNAL_CREATE(debug_load_panic_to_main_menu_definition, "debug_load_panic_to_main_menu", _hs_type_boolean, &debug_load_panic_to_main_menu);
HS_GLOBAL_EXTERNAL_CREATE(display_framerate_definition, "display_framerate", _hs_type_boolean, &display_framerate);
HS_GLOBAL_EXTERNAL_CREATE(display_frame_deltas_definition, "display_frame_deltas", _hs_type_boolean, &display_frame_deltas);
HS_GLOBAL_EXTERNAL_CREATE(console_pauses_game_definition, "console_pauses_game", _hs_type_boolean, &debug_console_pauses_game);
HS_GLOBAL_EXTERNAL_CREATE(framerate_infinite_definition, "framerate_infinite", _hs_type_boolean, &debug_disable_frame_rate_throttle);

HS_GLOBAL_EXTERNAL_CREATE(sim_entity_validate_definition, "sim_entity_validate", _hs_type_boolean, &g_simulation_entity_validate);


HS_GLOBAL_EXTERNAL_CREATE(console_status_string_render_definition, "console_status_string_render", _hs_type_boolean, NULL);

const hs_global_external* hs_external_globals[] =
{
	&debug_no_drawing_definition,
	&debug_force_all_player_views_to_default_definition,
	&debug_render_freeze_definition,
	&debug_render_horizontal_splitscreen_definition,
	&debug_load_panic_to_main_menu_definition,
	&display_framerate_definition,
	&display_frame_deltas_definition,
	&console_status_string_render_definition,
	&console_pauses_game_definition,
	&framerate_infinite_definition,

	/* TODO: add missing globals here */

	&sim_entity_validate_definition,

	/* TODO: add missing globals here */
};

const int16 k_hs_external_global_count = NUMBEROF(hs_external_globals);	// original is 1043
