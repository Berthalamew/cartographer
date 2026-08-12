#pragma once
#include "user_interface_shared_globals.h"

#include "math/color_math.h"
#include "tag_files/tag_block.h"
#include "tag_files/tag_reference.h"

enum e_screen_widget_flags : int
{
	screen_widget_flag_quarter_screen_dialog = FLAG(0),
	screen_widget_flag_multiple_panes_are_for_list_flavor_items = FLAG(1),
	screen_widget_flag_no_header_text = FLAG(2),
	screen_widget_flag_half_screen_dialog = FLAG(3),
	screen_widget_flag_large_dialog = FLAG(4),
	screen_widget_flag_disable_overlay_effect = FLAG(5)
};

enum e_button_key_type : short
{
	button_key_type_none = 0,
	button_key_type_a_select_b_back = 1,
	button_key_type_a_select_b_Cancel = 2,
	button_key_type_a_enter_b_cancel = 3,
	button_key_type_y_xbl_players = 4,
	button_key_type_x_friend_options = 5,
	button_key_type_x_clan_options = 6,
	button_key_type_x_recent_players_options = 7,
	button_key_type_x_options = 8,
	button_key_type_a_select = 9,
	button_key_type_x_settings_a_select_b_back = 10,
	button_key_type_x_delete_a_select_b_done = 11,
	button_key_type_a_accept = 12,
	button_key_type_b_cancel = 13,
	button_key_type_y_xbox_live_players_a_select_b_back = 14,
	button_key_type_y_xbox_live_players_a_select_b_cancel = 15,
	button_key_type_y_xbox_live_players_a_enter_b_cancel = 16,
	button_key_type_y_xbox_live_players_a_select = 17,
	button_key_type_y_xbox_live_players_a_select_b_done = 18,
	button_key_type_y_xbox_live_players_a_accept = 19,
	button_key_type_y_xbox_live_players_b_cancel = 20,
	button_key_type_x_delete_a_select_b_back = 21,
	button_key_type_a_ok = 22
};

enum e_animation_index : short
{
	none = 0,
	animation_index_00 = 1,
	animation_index_01 = 2,
	animation_index_02 = 3,
	animation_index_03 = 4,
	animation_index_04 = 5,
	animation_index_05 = 6,
	animation_index_06 = 7,
	animation_index_07 = 8,
	animation_index_08 = 9,
	animation_index_09 = 10,
	animation_index_10 = 11,
	animation_index_11 = 12,
	animation_index_12 = 13,
	animation_index_13 = 14,
	animation_index_14 = 15,
	animation_index_15 = 16,
	animation_index_16 = 17,
	animation_index_17 = 18,
	animation_index_18 = 19,
	animation_index_19 = 20,
	animation_index_20 = 21,
	animation_index_21 = 22,
	animation_index_22 = 23,
	animation_index_23 = 24,
	animation_index_24 = 25,
	animation_index_25 = 26,
	animation_index_26 = 27,
	animation_index_27 = 28,
	animation_index_28 = 29,
	animation_index_29 = 30,
	animation_index_30 = 31,
	animation_index_31 = 32,
	animation_index_32 = 33,
	animation_index_33 = 34,
	animation_index_34 = 35,
	animation_index_35 = 36,
	animation_index_36 = 37,
	animation_index_37 = 38,
	animation_index_38 = 39,
	animation_index_39 = 40,
	animation_index_40 = 41,
	animation_index_41 = 42,
	animation_index_42 = 43,
	animation_index_43 = 44,
	animation_index_44 = 45,
	animation_index_45 = 46,
	animation_index_46 = 47,
	animation_index_47 = 48,
	animation_index_48 = 49,
	animation_index_49 = 50,
	animation_index_50 = 51,
	animation_index_51 = 52,
	animation_index_52 = 53,
	animation_index_53 = 54,
	animation_index_54 = 55,
	animation_index_55 = 56,
	animation_index_56 = 57,
	animation_index_57 = 58,
	animation_index_58 = 59,
	animation_index_59 = 60,
	animation_index_60 = 61,
	animation_index_61 = 62,
	animation_index_62 = 63,
	animation_index_63 = 64
};

enum e_text_flags : int
{
	text_flag_left_justify_text = FLAG(0),
	text_flag_right_justify_text = FLAG(1),
	text_flag_pulsating_text = FLAG(2),
	text_flag_callout_text = FLAG(3),
	text_flag_small_31_char_buffer = FLAG(4)
};

enum e_custom_font : short
{
	custom_font_terminal = 0,
	custom_font_body_text = 1,
	custom_font_title = 2,
	custom_font_super_large_font = 3,
	custom_font_large_body_text = 4,
	custom_font_split_screen_hud_message = 5,
	custom_font_full_screen_hud_message = 6,
	custom_font_english_body_text = 7,
	custom_font_hud_number_text = 8,
	custom_font_subtitle_font = 9,
	custom_font_main_menu_font = 10,
	custom_font_text_chat_font = 11
};

enum e_button_flags : int
{
	button_flag_doesnt_ta_vertically = FLAG(0),
	button_flag_doesnt_tab_horizontally = FLAG(1)
};

#define k_maximum_number_of_button_widget_blocks 64
struct s_button_widget_reference
{
	e_text_flags text_flags;
	e_animation_index animation_index;
	short intro_animation_delay_milliseconds;
	short pad;

	e_custom_font custom_font;
	real_argb_color text_color;
	rectangle2d bounds;

	// bitm
	tag_reference bitmap;
	/// from top-left
	point2d bitmap_offset;
	string_id string_id;
	short render_depth_bias;
	short mouse_region_top_offset;
	
	e_button_flags button_flags;
};
ASSERT_STRUCT_SIZE(s_button_widget_reference, 0x3C);

enum e_list_reference_flags : int
{
	list_reference_flag_list_wraps = FLAG(0),
	list_reference_flag_interactive = FLAG(1)
};

enum e_skin_index : short
{
	skin_index_default = 0,
	skin_index_squad_lobby_player_list = 1,
	skin_index_settings_list = 2,
	skin_index_playlist_entry_list = 3,
	skin_index_variants = 4,
	skin_index_game_browser = 5,
	skin_index_online_player_menu = 6,
	skin_index_game_setup_menu = 7,
	skin_index_playlist_contents_display = 8,
	skin_index_player_profile_picker = 9,
	skin_index_mp_map_selection = 10,
	skin_index_main_menu_list = 11,
	skin_index_color_picker = 12,
	skin_index_profile_picker = 13,
	skin_index_y_menu_recent_list = 14,
	skin_index_pcr_team_stats = 15,
	skin_index_pcr_player_stats = 16,
	skin_index_pcr_kill_stats = 17,
	skin_index_pcr_pvp_stats = 18,
	skin_index_pcr_medal_stats = 19,
	skin_index_matchmaking_progress = 20,
	skin_index_default_5 = 21,
	skin_index_default_6 = 22,
	skin_index_advanced_settings_list = 23,
	skin_index_live_game_browser = 24,
	skin_index_default_wide = 25,
	skin_index_unused26 = 26,
	skin_index_unused27 = 27,
	skin_index_unused28 = 28,
	skin_index_unused29 = 29,
	skin_index_unused30 = 30,
	skin_index_unused31 = 31,
};

// This is a stupid enum but it's sized as a short in the original game so this is how things have to be
enum e_boolean_value : short
{
	boolean_value_false = 0,
	boolean_value_true = 1
};

enum e_value_type : short
{
	value_type_integer_number = 0,
	value_type_floating_point_number = 1,
	value_type_boolean = 2,
	value_type_text_string = 3
};

#define k_maximum_text_value_pairs_per_block 100
struct s_text_value_pair_reference_UNUSED
{
	// Explaination("OBSOLETE", "this is all obsolete")

	e_value_type value_type;

	// Explaination("Value", "Enter the value in the box corresponding to the value type you specified above")    
	e_boolean_value boolean_value;

	int integer_value;
	float fp_value;
	string_id text_value_stringid;
	/* Explaination("Text Label", "This is text string associated with data when it has the value specified above.
	The string comes from the screen's string list tag.")*/
	string_id text_label_stringid;
};
ASSERT_STRUCT_SIZE(s_text_value_pair_reference_UNUSED, 20);

#define k_max_list_reference_block_count 1
struct s_list_reference
{
	e_list_reference_flags flags;
	e_skin_index skin_index;
	short num_visible_items;
	point2d bottom_left;

	e_animation_index animation_index;
	short intro_animation_delay_milliseconds;

	// Explaination("UNUSED", "This is unused")
	tag_block<s_text_value_pair_reference_UNUSED> uNUSED;
};
ASSERT_STRUCT_SIZE(s_list_reference, 24);

enum e_table_view_list_reference_flags : int
{
	table_view_list_reference_flag_unused = FLAG(0),
};

#define k_maximum_number_of_table_view_list_cell_blocks 8
struct table_view_list_item_reference_block
{
	e_text_flags text_flags;
	short cell_width;
	short pad;
	point2d bitmap_topleft_if_there_is_a_bitmap;

	// bitm
	tag_reference bitmap_tag;
	string_id string;
	short renderDepthBias;
	short pad2;
};
ASSERT_STRUCT_SIZE(table_view_list_item_reference_block, 0x1C);

enum e_table_view_list_row_reference_flags : int
{
	table_view_list_row_reference_flag_unused = FLAG(0),
};

#define k_maximum_number_of_table_view_list_row_blocks 16
struct s_table_view_list_row_reference_OBSOLETE
{
	e_table_view_list_row_reference_flags flags;
	short row_height;
	short pad;
	tag_block<table_view_list_item_reference_block> rowCells;
};
ASSERT_STRUCT_SIZE(s_table_view_list_row_reference_OBSOLETE, 16);

#define k_maximum_number_of_table_view_list_blocks 1
struct s_table_view_list_reference_OBSOLETE
{
	e_table_view_list_reference_flags flags;
	e_animation_index animation_index;
	short intro_animation_delay_milliseconds;

	e_custom_font custom_font;
	short pad;
	real_argb_color text_color;
	point2d topleft;
	tag_block<s_table_view_list_row_reference_OBSOLETE> table_rows;
};
ASSERT_STRUCT_SIZE(s_table_view_list_reference_OBSOLETE, 40);

#define k_maximum_number_of_text_blocks 64
struct s_text_block_reference
{
	e_text_flags text_flags;
	e_animation_index animation_index;
	short intro_animation_delay_milliseconds;
	short pad0;

	e_custom_font custom_font;
	real_argb_color text_color;
	rectangle2d text_bounds;
	string_id string;
	short render_depth_bias;
	short pad1;
};
ASSERT_STRUCT_SIZE(s_text_block_reference, 44);


#define k_maximum_text_value_pairs_per_block 100
struct s_text_value_pair_block_UNUSED
{
	// Explaination("OBSOLETE", "this is all obsolete")
	char name[k_tag_string_length];
	tag_block<s_text_value_pair_reference_UNUSED> text_value_pairs;
};
ASSERT_STRUCT_SIZE(s_text_value_pair_block_UNUSED, 40);

enum e_hud_block_reference_flags : int
{
	hud_block_reference_flag_ignore_for_list_skin_size = FLAG(0),
	hud_block_reference_flag_needs_valid_rank = FLAG(1)
};

#define k_maximum_number_of_hud_blocks 64
struct s_hud_block_reference
{
	e_hud_block_reference_flags flags;

	e_animation_index animation_index;
	short intro_animation_delay_milliseconds;
	short render_depth_bias;
	short starting_bitmap_sequence_index;

	// bitm
	tag_reference bitmap;
	// shad
	tag_reference shader;

	rectangle2d bounds;
};
ASSERT_STRUCT_SIZE(s_hud_block_reference, 36);

enum class e_table_order : char
{
	table_order_row_major = 0,
	table_order_column_major = 1,
};

#define k_maximum_number_of_player_blocks 64
struct s_player_block_reference
{
	int pad;

	// skin
	tag_reference skin;
	point2d bottomleft;
	e_table_order table_order;
	byte maximum_player_count;
	byte row_count;
	byte column_count;
	short row_height;
	short column_width;
}; 
ASSERT_STRUCT_SIZE(s_player_block_reference, 24);

#define k_maximum_number_of_window_pane_tag_blocks 16
struct s_window_pane_reference
{
	short pad;
	e_animation_index animation_index;

	// Explaination("Button Definitions", "If the pane contains buttons, define them here")
	tag_block<s_button_widget_reference> buttons;

	// Explaination("List Definition", "If the pane contains a list, define it here")
	tag_block<s_list_reference> list_block;

	// Explaination("OBSOLETE Table View Definition", "If the pane contains a table-view, define it here")
	tag_block<s_table_view_list_reference_OBSOLETE> table_view;

	// Explaination("Flavor Item Blocks", "Define additional flavor items here")
	tag_block<s_text_block_reference> text_blocks;

	tag_block<s_bitmap_block_reference> bitmap_blocks;

	tag_block<s_ui_model_scene_reference> model_scene_blocks;

	// Explaination("UNUSED", "these are all OBSOLETE")
	tag_block<s_text_value_pair_block_UNUSED> textvalue_blocks;

	tag_block<s_hud_block_reference> hud_blocks;

	tag_block<s_player_block_reference> player_blocks;
};
ASSERT_STRUCT_SIZE(s_window_pane_reference, 0x4C);

// ### TODO Finish this
struct s_user_interface_screen_widget_definition
{
	/* Explaination("Notes on screen widgets:", 
	"- the widget coordinate system is a left-handed system (+x to the right, +y up, +z into the screen)
	with the origin centered in the display(regardless of display size)
	- for widget component placement, all coordinates you define in the tag specifiy the object's
	placement prior to the application of any animation
	- all coordinates you define are local to that object
	- all text specific to objects in the screen comes from the screen's string list tag
	all of the string indices you may need to specify will refer to the screen's string list tag
	- a pane may contain either buttons OR a list OR a table - view, but never a combination of those
	(widget won't function correctly if you try that)
	- all text is centered unless you specify otherwise")*/

	// Explaination("Flags", "Set misc. screen behavior here")

	e_screen_widget_flags flags;
	int16 screen_id;				// e_user_interface_screen_id

	// Explaination("Button Key", 
	// "The labels here are just a guide; the actual string used comes from the Nth position of this button key entry as found in the ui globals button key string list tag")
	e_button_key_type button_key_type;

	// Explaination("Default Text Color", "Any ui elements that don't explicitly set a text color will use this color")
	real_argb_color text_color;
	// Explaination("Screen Text", "All text specific to this screen")
	// TagReference("unic")
	tag_reference string_list_tag;
	// Explaination("Panes", "Define the screen's panes here (normal screens have 1 pane, tab-view screens have 2+ panes)")
	tag_block<s_window_pane_reference> panes;
};