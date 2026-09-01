#pragma once

/* prototypes */

void simulation_game_action_apply_patches(void);

void __cdecl simulation_action_game_engine_globals_create(void);
void __cdecl simulation_action_game_engine_globals_update(uint32 flags);
void __cdecl simulation_action_game_engine_globals_delete(void);
void __cdecl simulation_action_game_statborg_create(void);
void __cdecl simulation_action_game_statborg_update(uint32 flags);
void simulation_action_game_statborg_delete(void);

void __cdecl simulation_action_game_engine_player_create(int16 player_absolute_index);
void __cdecl simulation_action_game_engine_player_update(int16 player_absolute_index, uint32 flags);
void simulation_action_game_engine_player_delete(int16 player_absolute_index);

void __cdecl simulation_action_object_create(int32 object_index);
void __cdecl simulation_action_object_update(int32 object_index, uint32 update_mask);

void __cdecl simulation_action_object_force_update(int32 object_index, uint32 flags);

void __cdecl simulation_action_object_delete(int32 object_index);
void __cdecl simulation_action_object_detach_from_gamestate_and_delete(int32 object_index);

void __cdecl simulation_action_breakable_surfaces_create(int32 group_index);
void __cdecl simulation_action_breakable_surfaces_delete(void);

void __cdecl simulation_action_pickup_equipment(int32 unit_datum_index, int32 grenade_tag_index);
