#pragma once

/* prototypes */

void simulation_apply_patches(void);

void __cdecl simulation_player_joined_game(int32 player_index);

void __cdecl simulation_player_left_game(int32 player_index);

class c_simulation_world* simulation_get_world();

void simulation_reset(void);
bool simulation_reset_in_progress(void);

void __cdecl simulation_update(void);

bool simulation_starting_up(void);

void simulation_notify_reset_complete(void);

void simulation_notify_reset_initiate(void);

void simulation_notify_going_active(void);

bool simulation_in_progress(void);
void simulation_destroy_update(struct simulation_update* update);
bool simulation_query_object_is_predicted(datum object_datum);
class c_simulation_type_collection* simulation_get_type_collection();

void __cdecl simulation_apply_before_game(const struct simulation_update* update);

void simulation_apply_after_game(const struct simulation_update* update);

void simulation_fatal_error(void);

void __cdecl simulation_build_update(struct simulation_update* update);

void __cdecl simulation_update_aftermath(const struct simulation_update* update);

void simulation_update_pregame(void);

void __cdecl simulation_process_input(uint32 player_action_mask, struct player_action const* player_actions);

bool __cdecl simulation_get_machine_active_in_game(struct s_machine_identifier* machine_identifier);

void __cdecl simulation_build_player_updates(int32* player_update_count, int32 maximum_player_update_count, struct simulation_player_update* player_updates);
