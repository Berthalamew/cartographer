#include "stdafx.h"
#include "game_engine_simulation.h"

#include "game.h"
#include "game_engine.h"

/* public code */

e_simulation_entity_type game_engine_globals_get_simulation_entity_type(
	void)
{
	e_simulation_entity_type entity_type = k_simulation_entity_type_none;

	if (current_game_engine())
	{
		entity_type = current_game_engine()->get_game_engine_entity_type();
	}

	return entity_type;
}

void game_engine_globals_set_gamestate_index(
	int32 gamestate_index)
{
	game_engine_globals_get()->game_engine_gamestate_index = gamestate_index;

	return;
}

int32 game_engine_globals_get_gamestate_index(
	void)
{
	int32 gamestate_index = NONE;

	if (current_game_engine())
	{
		gamestate_index = game_engine_globals_get()->game_engine_gamestate_index;
	}

	return gamestate_index;
}

void game_engine_globals_set_statborg_gamestate_index(
	int32 gamestate_index)
{
	game_engine_globals_get()->statborg_gamestate_index = gamestate_index;

	return;
}

int32 game_engine_globals_get_statborg_gamestate_index(
	void)
{
	int32 gamestate_index = NONE;

	if (current_game_engine())
	{
		gamestate_index = game_engine_globals_get()->statborg_gamestate_index;
	}

	return gamestate_index;
}

void game_engine_globals_set_player_gamestate_index(
	int16 absolute_player_index,
	int32 gamestate_index)
{
	ASSERT(current_game_engine());
	ASSERT(absolute_player_index>=0 && absolute_player_index<k_maximum_multiplayer_players);

	game_engine_globals_get()->player_gamestate_indices[absolute_player_index] = gamestate_index;

	return;
}

int32 game_engine_globals_get_player_gamestate_index(
	int16 absolute_player_index)
{
	int32 gamestate_index = NONE;

	ASSERT(absolute_player_index>=0 && absolute_player_index<k_maximum_multiplayer_players);

	if (current_game_engine())
	{
		gamestate_index = game_engine_globals_get()->player_gamestate_indices[absolute_player_index];
	}

	return gamestate_index;
}

bool game_engine_globals_apply_player_update(
	int16 player_absolute_index,
	uint32 update_mask,
	int32 state_data_size,
	void const* state_data)
{
	bool update_success;

	ASSERT(current_game_engine());

	update_success = current_game_engine()->apply_player_update(player_absolute_index, update_mask, state_data_size, state_data);

	return update_success;
}
