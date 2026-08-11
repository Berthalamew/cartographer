#include "stdafx.h"
#include "simulation_players.h"

/* prototypes */

/* public code */

void simulation_players_apply_patches(void)
{
	return;
}

void simulation_player_collection_clear(
	s_player_collection* collection)
{
	ASSERT(collection);

	csmemset(collection, 0, sizeof(*collection));

	for (int32 player_index = 0; player_index<NUMBEROF(collection->players); ++player_index)
	{
		s_player_collection_player* collection_player = &collection->players[player_index];

		collection_player->left_game = false;
		collection_player->left_game_time = NONE;
		collection_player->controller_index = k_no_controller;
		collection_player->user_index = NONE;
	}

	return;
}

bool __cdecl simulation_players_apply_update(simulation_player_update* player_update)
{
	return INVOKE(0x1E22E2, 0x1C930E, simulation_players_apply_update, player_update);
}

