#pragma once
#include "simulation/game_interface/simulation_game_entities.h"

/* prototypes */

e_simulation_entity_type game_engine_globals_get_simulation_entity_type(void);
void game_engine_globals_set_gamestate_index(int32 gamestate_index);
int32 game_engine_globals_get_gamestate_index(void);
void game_engine_globals_set_statborg_gamestate_index(int32 gamestate_index);
int32 game_engine_globals_get_statborg_gamestate_index(void);
void game_engine_globals_set_player_gamestate_index(int16 absolute_player_index, int32 gamestate_index);
int32 game_engine_globals_get_player_gamestate_index(int16 absolute_player_index);
bool game_engine_globals_apply_player_update(int16 player_absolute_index, uint32 update_mask, int32 state_data_size, void const* state_data);
