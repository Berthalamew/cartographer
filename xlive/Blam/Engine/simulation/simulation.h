#pragma once
#include "simulation_encoding.h"
#include "simulation_players.h"
#include "simulation_queue.h"

#include "units/unit_control.h"

/* macros */

//#define H2_SIMULATION_UPDATE_STRUCT

/* constants */

enum
{
	k_maximum_simulation_player_updates = 64,
	k_bits_required_for_simulation_player_updates_count = 5
};

/* structures */

struct simulation_update
{
	int32 update_number;
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
	int32 verify_game_time;
	uint32 verify_random_seed;

// Added queues to simulation update
#ifndef H2_SIMULATION_UPDATE_STRUCT
	c_simulation_queue bookkeeping_simulation_queue;
	c_simulation_queue game_simulation_queue;
#endif
};
#ifdef H2_SIMULATION_UPDATE_STRUCT
ASSERT_STRUCT_SIZE(struct simulation_update, 0x3BD8);
#endif

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
	class c_simulation_world* world;
	class c_simulation_watcher* watcher;
	class c_simulation_type_collection* simulation_type_collection;
};
ASSERT_STRUCT_SIZE(s_simulation_globals, 24);

struct s_simulation_queued_update
{
	struct simulation_update update;
	struct s_simulation_queued_update* next_node;
};

/* prototypes */

void simulation_apply_patches(void);

class c_simulation_world* simulation_get_world();
s_simulation_globals* simulation_get_globals();

void simulation_reset(void);
bool simulation_reset_in_progress();

void __cdecl simulation_update(void);

bool simulation_starting_up(void);

bool simulation_aborted(void);

void simulation_notify_reset_complete(void);

void simulation_notify_reset_initiate(void);

void simulation_notify_going_active(void);
void simulation_reset_immediate(void);
bool simulation_in_progress(void);
void simulation_destroy_update(struct simulation_update* update);
bool simulation_query_object_is_predicted(datum object_datum);
class c_simulation_type_collection* simulation_get_type_collection();

void simulation_apply_before_game(const struct simulation_update* update);

void simulation_apply_after_game(const struct simulation_update* update);

void simulation_build_update(struct simulation_update* update);

void simulation_update_aftermath(const struct simulation_update* update);

class c_simulation_view* simulation_get_remote_view_by_channel(int32 network_channel_index);

bool simulation_update_write_to_buffer(const struct simulation_update* update, int32 buffer_size, uint8* buffer, int32* out_update_length);

bool simulation_update_read_from_buffer(struct simulation_update* update, int32 buffer_size, uint8* buffer);

void simulation_update_pregame(void);

void __cdecl simulation_process_input(uint32 player_action_mask, const struct player_action* player_actions);

bool __cdecl simulation_get_machine_active_in_game(s_machine_identifier* machine_identifier);

void simulation_build_machine_update(bool* machine_update_valid, struct simulation_machine_update* machine_update);

void simulation_build_player_updates(int32* player_update_count, int32 maximum_player_update_count, struct simulation_player_update* player_updates);

void simulation_fatal_error(void);

void __cdecl simulation_remove_view_from_world(class c_simulation_view* view);
