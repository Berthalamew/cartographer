#pragma once
#include "tag_files/tag_reference.h"

struct s_user_interface_tag_globals
{
	// Explaination("Shared Globals", "This is a reference to the ui shared globals tag")
	tag_reference shared_globals;	// wigl

	// Explaination("Screen Widgets", "These are the screen widgets")
	s_tag_block screen_widgets;	// s_user_interface_widget_reference

	// Explaination("Multiplayer Variant Settings Interface", "This blob defines the ui for setting multiplayer game variant parameters")
	tag_reference mp_variant_settings_ui;	// goof

	// Explaination("Game Hopper Localization Strings", "This is for the loc game hopper strings")
	tag_reference game_hopper_descriptions;	// unic
};
ASSERT_STRUCT_SIZE(s_user_interface_tag_globals, 32);
