#pragma once
#include "simulation_encoding.h"
#include "simulation_type_collection.h"
#include "simulation_watcher.h"
#include "simulation_world.h"

#include "game/player_control.h"
#include "units/unit_control.h"

enum
{
	k_maximum_simulation_player_updates = 64,
	k_bits_required_for_simulation_player_updates_count = 5
};

struct simulation_update
{
	int32 simulation_time;
	bool simulation_in_progress;
	uint32 player_action_mask;
	int32 field_C;
	player_action player_actions[k_maximum_players];
	uint32 unit_control_mask;
	datum control_unit_index[k_maximum_players];
	unit_control_data unit_control[k_maximum_players];
	bool machine_update_valid;
	simulation_machine_update machine_update;
	int32 player_update_count;
	simulation_player_update player_updates[k_maximum_simulation_player_updates];
	bool flush_gamestate;
	int32 game_time_ticks;
	uint32 random_seed;
};
ASSERT_STRUCT_SIZE(struct simulation_update, 0x3BD8);

struct s_simulation_globals
{
	bool initialized;
	bool simulation_fatal_error;
	bool simulation_aborted;
	int32 field_4;
	bool simulation_in_initial_state;
	bool simulation_reset_pending;
	bool simulation_reset_in_progress;
	bool loading_saved_game;
	c_simulation_world* world;
	c_simulation_watcher* watcher;
	c_simulation_type_collection* type_collection;
};
ASSERT_STRUCT_SIZE(s_simulation_globals, 24);

c_simulation_world* simulation_get_world();
s_simulation_globals* simulation_get_globals();

void simulation_reset(void);
bool simulation_reset_in_progress();

void __cdecl simulation_update(void);

bool simulation_starting_up(void);

void simulation_notify_reset_complete(void);

void simulation_notify_reset_initiate(void);

void simulation_notify_going_active(void);

bool simulation_in_progress(void);
void simulation_destroy_update(void);
bool simulation_query_object_is_predicted(datum object_datum);
c_simulation_type_collection* simulation_get_type_collection();

void __cdecl simulation_apply_before_game(const struct simulation_update* update);

void simulation_apply_after_game(const struct simulation_update* update);

void __cdecl simulation_build_update(struct simulation_update* update);

void __cdecl simulation_update_aftermath(const struct simulation_update* update);

void simulation_update_pregame(void);

void __cdecl simulation_process_input(uint32 player_action_mask, const player_action* player_actions);

bool __cdecl simulation_get_machine_active_in_game(s_machine_identifier* machine_identifier);

void simulation_apply_patches(void);
