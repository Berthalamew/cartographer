#include "stdafx.h"
#include "simulation_encoding.h"

#include "simulation.h"
#include "simulation_players.h"
#include "simulation_update.h"
#include "simulation_world.h"

#include "memory/bitstream.h"
#include "networking/network_event.h"

/* constants */

enum
{
	k_simulation_update_estimated_size= 6144,
};

/* enums */

/* structures */

/* prototypes */

static void __cdecl player_action_encode(c_bitstream* packet, struct player_action* action);
static bool __cdecl player_action_decode(c_bitstream* packet, struct player_action* action);
static void __cdecl simulation_machine_update_encode(c_bitstream* packet, struct simulation_machine_update* machine_update);
static bool __cdecl simulation_machine_update_decode(c_bitstream* packet, struct simulation_machine_update* machine_update);

/* public code */

void __cdecl simulation_player_update_encode(
	c_bitstream* packet,
	simulation_player_update const* player_update)
{
	ASSERT(packet);
	ASSERT(player_update);
	ASSERT(VALID_INDEX(player_update->update_type, k_simulation_player_update_type_count));

	INVOKE(0x1E06AB, 0x1C7B6B, simulation_player_update_encode, packet, player_update);
	return;
}

bool __cdecl simulation_player_update_decode(
	c_bitstream* packet,
	simulation_player_update* player_update)
{
	ASSERT(packet);
	ASSERT(player_update);

	return INVOKE(0x1E078A, 0x1C7C4A, simulation_player_update_decode, packet, player_update);
}


void __cdecl simulation_update_encode(
	c_bitstream* packet,
	struct simulation_update* update)
{
	//INVOKE(0x1E0998, 0x0, synchronous_update_encode_internal, packet, update);

	const int32 starting_pos = packet->get_current_bit_position();

	ASSERT(packet);
	ASSERT(update);

	packet->write_integer("update-number", update->update_number, SIZEOF_BITS(update->update_number));
	packet->write_bool("simulation_in_progress", update->simulation_in_progress);		//adding missing simulation_in_progress
	packet->write_integer("player-flags", update->player_action_mask, k_maximum_players);

	for (int8 player_index = 0; player_index < k_maximum_players; ++player_index)
	{
		if (TEST_BIT(update->player_action_mask, player_index))
		{
			player_action_encode(packet, &update->player_actions[player_index]);
		}
	}

	// why is unit/actor control data never encoded?????
	// h3 seems to use it , but h2 does not

	packet->write_bool("machine-update-exists", update->machine_update_valid);
	if (update->machine_update_valid)
	{
		simulation_machine_update_encode(packet, &update->machine_update);
	}

	packet->write_integer("player-update-count", update->player_update_count, k_bits_required_for_simulation_player_updates_count);
	ASSERT(update->player_update_count >= 0 && update->player_update_count <= k_maximum_simulation_player_updates);


	for (int32 update_idx = 0; update_idx<update->player_update_count; ++update_idx)
	{
		simulation_player_update_encode(packet, &update->player_updates[update_idx]);
	}
	
	packet->write_bool("flush-gamestate", update->flush_gamestate);
	packet->write_integer("verify-game-time", update->verify_game_time, SIZEOF_BITS(update->verify_game_time));
	packet->write_integer("verify-random", update->verify_random_seed, SIZEOF_BITS(update->verify_random_seed));

	const int32 pre_queues_encoded_size = (packet->get_current_bit_position()-starting_pos+ 7) / 8;
	ASSERT(pre_queues_encoded_size > 0);

	if (pre_queues_encoded_size > k_simulation_update_estimated_size)
	{
		event(
			_event_fatal,
			"networking:simulation:encoding: encoded simulation update (no queues) exceeding estimate [%d > %d]",
			pre_queues_encoded_size,
			k_simulation_update_estimated_size);
	}


	c_simulation_world* simulation_world = simulation_get_world();
	simulation_world->queue_get(_simulation_queue_bookkeeping)->encode(packet);
	simulation_world->queue_get(_simulation_queue)->encode(packet);

	return;
}

bool __cdecl simulation_update_decode(
	c_bitstream* packet,
	struct simulation_update* update)
{
	//return INVOKE(0x1E0AA2, 0x0, synchronous_update_decode_internal, packet, update);

	ASSERT(packet);
	ASSERT(update);

	bool result = true;
	update->update_number = packet->read_integer("update-number", SIZEOF_BITS(update->update_number));
	update->simulation_in_progress = packet->read_bool("simulation_in_progress"); 	//adding missing simulation_in_progress
	update->player_action_mask = packet->read_integer("player-flags", k_maximum_players);

	
	for (int8 player_index = 0; player_index < k_maximum_players; ++player_index)
	{
		if (TEST_BIT(update->player_action_mask, player_index))
		{
			result = result && player_action_decode(packet, &update->player_actions[player_index]);
		}
	}

	update->machine_update_valid = packet->read_bool("machine-update-exists");
	if (update->machine_update_valid)
	{
		result = result && simulation_machine_update_decode(packet, &update->machine_update);
	}

	update->player_update_count = packet->read_integer("player-update-count", k_bits_required_for_simulation_player_updates_count);

	if (VALID_INDEX(update->player_update_count, k_maximum_simulation_player_updates))
	{
		for (int8 update_index = 0; update_index < update->player_update_count; ++update_index)
		{
			result = result && simulation_player_update_decode(packet, &update->player_updates[update_index]);
		}
	}
	else
	{
		result = false;
	}

	update->flush_gamestate = packet->read_bool("flush-gamestate");
	update->verify_game_time = packet->read_integer("verify-game-time", SIZEOF_BITS(update->verify_game_time));
	update->verify_random_seed = packet->read_integer("verify-random", SIZEOF_BITS(update->verify_random_seed));

	c_simulation_world* simulation_world = simulation_get_world();
	
	// Validation
	result = result && simulation_world->queue_get(_simulation_queue_bookkeeping)->decode(packet);
	result = result && simulation_world->queue_get(_simulation_queue)->decode(packet);
	result = result && !packet->error_occurred();
	result = result && update->verify_game_time >= 0;
	result = result && update->update_number >= 0;

	// If something went wrong dispose of the queues
	if (!result)
	{
		simulation_world->queue_get(_simulation_queue_bookkeeping)->dispose();
		simulation_world->queue_get(_simulation_queue)->dispose();
	}

	return result;
}

/* private code */

void __cdecl player_action_encode(c_bitstream* packet, struct player_action* action)
{
	INVOKE(0x1DFE4C, 0x0, player_action_encode, packet, action);
}

bool __cdecl player_action_decode(c_bitstream* packet, struct player_action* action)
{
	return INVOKE(0x1E01CB, 0x0, player_action_decode, packet, action);
}

void __cdecl simulation_machine_update_encode(c_bitstream* packet, struct simulation_machine_update* machine_update)
{
	INVOKE(0x1E08E7, 0x0, simulation_machine_update_encode, packet, machine_update);
}

bool __cdecl simulation_machine_update_decode(c_bitstream* packet, struct simulation_machine_update* machine_update)
{
	return INVOKE(0x1E0935, 0x0, simulation_machine_update_decode, packet, machine_update);
}
