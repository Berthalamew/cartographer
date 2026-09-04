#pragma once
#include "game_variant.h"

#include "game/players.h"
#include "input/input_abstraction.h"

/* constants */

enum
{
	k_default_profiles_count = 1
};

/* enums */

enum e_saved_game_profile_input_preference_flags
{
	_saved_game_profile_input_preference_bit_controller_look_inversion = 0x0,
	_saved_game_profile_input_preference_bit_mouse_look_inversion,
	_saved_game_profile_input_preference_bit_vibration_disabled,
	_saved_game_profile_input_preference_bit_4,
	_saved_game_profile_input_preference_bit_controller_auto_look_centering,
	_saved_game_profile_input_preference_bit_mouse_auto_look_centering,
	_saved_game_profile_input_preference_bit_mouse_dual_wield_inversion,
	k_saved_game_profile_input_preferences_count
};

/* structures */

struct s_saved_game_profile_variant_info
{
	int8 data[192];
	s_game_variant variant;
};

struct s_saved_game_profile_input_binds
{
	uint32 button_count;
	s_input_button buttons[5];
};

struct s_saved_game_profile_input_preferences
{
	c_flags_no_init<e_saved_game_profile_input_preference_flags, uint32, k_saved_game_profile_input_preferences_count> flags;
	e_button_preset_types controller_button_layout;
	e_joystick_preset_types controller_thumbstick_layout;
	int8 controller_sensitivity;
	int8 mouse_sensitivity;
	e_custom_keyboard_preset_types keyboard_preset_type;
	int8 pad[3];
	s_saved_game_profile_input_binds input_binds[NUMBER_OF_EXTENDED_CONTROL_BUTTONS];
};
ASSERT_STRUCT_SIZE(s_saved_game_profile_input_preferences, 3660);

struct s_saved_game_player_profile
{
	int32 valid_maybe;
	int32 unk;
	wchar_t name[32];
	int8 data[160];
	s_saved_game_profile_variant_info variant;
	int8 data2[20];
	s_saved_game_profile_input_preferences input_preferences;
	int8 data3[16];
	s_player_appearance appearance;
	int8 gap2[176];
};
ASSERT_STRUCT_SIZE(s_saved_game_player_profile, 4616);

/* prototypes */

void __cdecl saved_game_player_profile_set_default_variant(void* saved_game_variant);

void saved_game_player_profile_default_new(s_saved_game_player_profile* profile, int32 default_profile_type);

bool saved_game_player_profile_read_file(uint32 enumerated_file_index, s_saved_game_player_profile* profile);

bool __cdecl saved_game_player_profile_read_post_verify_profile_traits(struct s_player_appearance* profile);

bool saved_game_player_profile_load(uint32 enumerated_file_index, s_saved_game_player_profile* profile);

void player_profile_traits_initialize(struct s_player_appearance* appearance);
