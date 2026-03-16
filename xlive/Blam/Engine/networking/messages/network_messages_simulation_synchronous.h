#pragma once
#include "game/player_constants.h"
#include "game/player_control.h"
#include "simulation/simulation.h"

/* enums */

enum e_synchronous_gamestate_message_type
{
	_synchronous_gamestate_message_initiate_join = 0,
	_synchronous_gamestate_message_gamestate_finish,
	_synchronous_gamestate_message_catchup,
	k_number_of_synchronous_gamestate_message_types,
};

/* structures */

struct s_network_message_synchronous_join
{
	int32 next_update_number;
};

struct s_network_message_synchronous_gamestate
{
	int32 gamestate_offset;
	int32 gamestate_size;
};

struct s_network_message_synchronous_actions
{
	int32 action_number;
	int32 client_update_number;
	bool go_out_of_sync;
	uint32 valid_user_mask;
	player_action actions[k_number_of_users];
};

struct s_network_message_synchronous_catchup
{
	int16 type;	// e_synchronous_gamestate_message_type
	int16 update_length;
	int32 next_update_number;
};

struct s_network_message_synchronous_update
{
	struct simulation_update update;
};

/* classes */

class c_network_message_synchronous_update
{
	static void encode(class c_bitstream* packet, int32 message_storage_size, void const* message_storage);
	static bool decode(class c_bitstream* packet, int32 message_storage_size, void* message_storage);
};

/* prototypes */

void __cdecl network_message_types_register_simulation_synchronous(class c_network_message_type_collection* message_collection);
