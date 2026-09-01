#include "stdafx.h"
#include "game_engine.h"

#include "saved_games/player_profile.h"
#include "simulation/game_interface/simulation_game_action.h"

/* public code */

c_game_engine* current_game_engine(void)
{
	return get_game_mode_engines()[game_engine_globals_get()->game_engine_index];
}

s_game_engine_globals* game_engine_globals_get(void)
{
	return *Memory::GetAddress<s_game_engine_globals**>(0x4BF8F8, 0x4EA028);
}

s_simulation_player_netdebug_data* game_engine_get_netdebug_data(datum player_index)
{
	return &game_engine_globals_get()->netdebug_data[DATUM_INDEX_TO_ABSOLUTE_INDEX(player_index)];
}

c_game_engine** get_game_mode_engines()
{
	return Memory::GetAddress<c_game_engine**>(0x4D8548, 0x4F3CE4);
}

void game_engine_game_ending(
	void)
{
	if (current_game_engine())
	{
		current_game_engine()->game_ending();

		simulation_action_game_engine_globals_delete();
		simulation_action_game_statborg_delete();

		for (int16 player_absolute_index= 0; player_absolute_index<k_maximum_players; ++player_absolute_index)
		{
			simulation_action_game_engine_player_delete(player_absolute_index);
		}
	}

	return;
}

bool __cdecl game_engine_is_team_ever_active(
	e_game_team team_index)
{
	bool is_team_ever_active = INVOKE(0x6E858, 0x0, game_engine_is_team_ever_active, team_index);

	return is_team_ever_active;
}

void __cdecl game_engine_apply_map_patches(void)
{
	INVOKE(0x6EFDB, 0x0, game_engine_apply_map_patches);
	return;
}

bool __cdecl game_engine_get_change_colors(s_player_appearance* player_profile, e_game_team team_index, real_rgb_color* change_colors)
{
	return INVOKE(0x6E5C3, 0x6D1BF, game_engine_get_change_colors, player_profile, team_index, change_colors);
}

bool __cdecl game_engine_variant_cleanup(uint16* flags)
{
	return INVOKE(0x5B720, 0x3D380, game_engine_variant_cleanup, flags);
}

void __cdecl game_engine_player_activated(datum player_index)
{
	INVOKE(0x6A29E, 0x69CB6, game_engine_player_activated, player_index);
	return;
}

bool __cdecl game_engine_team_is_enemy(e_game_team a, e_game_team b)
{
	return INVOKE(0x6ADA3, 0x6A5DE, game_engine_team_is_enemy, a, b);
}

void __cdecl game_engine_update_after_game(void)
{
	INVOKE(0x7156A, 0x7006B, game_engine_update_after_game);
	return;
}

void __cdecl game_engine_update(void)
{
	INVOKE(0x7590F, 0x727EA, game_engine_update);
	return;
}

void __cdecl game_engine_render(void)
{
	INVOKE(0x6A60F, 0x0, game_engine_render);
	return;
}

bool game_engine_running(
	void)
{
	bool result = current_game_engine() != NULL;

	return result;
}
