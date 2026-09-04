#include "stdafx.h"
#include "player_profile.h"

#include "saved_game_files_async_windows.h"

/* public code */

void __cdecl saved_game_player_profile_set_default_variant(void* saved_game_variant)
{
	INVOKE(0x98FD, 0, saved_game_player_profile_set_default_variant, saved_game_variant);
	return;
}

void saved_game_player_profile_default_new(s_saved_game_player_profile* profile, int32 default_profile_type)
{
	s_gamepad_input_preferences input_preferences;

	csmemset(profile, 0, sizeof(s_saved_game_player_profile));
	profile->valid_maybe = true;
	ustrncpy(profile->name, L"Guest", NUMBEROF(s_saved_game_player_profile::name));
	profile->appearance.change_color_index[0] = _player_color_colbat;
	profile->appearance.change_color_index[1] = _player_color_white;
	profile->appearance.change_color_index[2] = _player_color_white;
	profile->appearance.change_color_index[3] = _player_color_white;
	profile->appearance.emblem_info.foreground_emblem = _emblem_foreground_seventh_column;
	profile->appearance.emblem_info.background_emblem = _emblem_background_solid;
	SET_BIT(profile->unk, 0, true);
	profile->input_preferences.controller_sensitivity = 3;
	profile->input_preferences.mouse_sensitivity = 3;
	profile->input_preferences.flags.clear();

	if(default_profile_type)
	{
		if(default_profile_type == 1)
		{
			profile->input_preferences.controller_button_layout = _button_preset_default;
			profile->input_preferences.controller_thumbstick_layout = _joystick_preset_default;
			profile->input_preferences.flags.set(_saved_game_profile_input_preference_bit_controller_look_inversion, true);
			profile->input_preferences.flags.set(_saved_game_profile_input_preference_bit_mouse_look_inversion, true);
		}
	}
	else
	{
		profile->input_preferences.controller_button_layout = _button_preset_default;
		profile->input_preferences.controller_thumbstick_layout = _joystick_preset_default;
	}
	saved_game_player_profile_set_default_variant(&profile->variant);
	input_abstraction_get_default_preferences(&input_preferences, _joystick_preset_default, _button_preset_default, _custom_keyboard_preset_right_hold, 0);
	input_abstraction_set_controller_settings_from_preferences(&input_preferences, &profile->input_preferences);
}

bool saved_game_player_profile_read_file(uint32 enumerated_file_index, s_saved_game_player_profile* profile)
{
	ASSERT(profile);

	int8 buffer[sizeof(s_saved_game_player_profile)];

	bool result = saved_games_async_helper_read_file(enumerated_file_index, buffer, sizeof(s_saved_game_player_profile));

	if (result)
		csmemcpy(profile, buffer, sizeof(s_saved_game_player_profile));

	// todo: cleanup and define the structure here
	//*(int16*)profile->data[0x1E] = 0;
	//if (profile->gap2[0x29] >= 3u)
	//	profile->gap2[0x29] = 0;

	if (!saved_game_player_profile_read_post_verify_profile_traits(&profile->appearance))
	{
		csmemset(&profile->appearance, 0, sizeof(s_player_appearance));
	}
	
	return result;
}

bool __cdecl saved_game_player_profile_read_post_verify_profile_traits(s_player_appearance* appearance)
{
	return INVOKE(0x54f82, 0, saved_game_player_profile_read_post_verify_profile_traits, appearance);
}

bool saved_game_player_profile_load(uint32 enumerated_file_index, s_saved_game_player_profile* profile)
{
	ASSERT(profile);

	if(enumerated_file_index == NONE)
	{
		saved_game_player_profile_default_new(profile, 0);
		return true;
	}
	else
	{
		return saved_game_player_profile_read_file(enumerated_file_index, profile);
	}
}

void player_profile_traits_initialize(
	s_player_appearance* appearance)
{
	appearance->change_color_index[0] = _player_color_white;
	appearance->change_color_index[1] = _player_color_white;
	appearance->change_color_index[2] = _player_color_white;
	appearance->change_color_index[3] = _player_color_white;
	appearance->player_character_type = _character_type_masterchief;
	appearance->emblem_info.foreground_emblem = _emblem_foreground_seventh_column;
	appearance->emblem_info.background_emblem = _emblem_background_solid;
	appearance->emblem_info.emblem_flags.clear();
	appearance->gap_48 = 0;
	appearance->gap_4C = 0;
	return;
}
