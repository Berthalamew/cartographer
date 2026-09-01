#include "stdafx.h"
#include "simulation_encoding.h"

#include "simulation.h"

#include "cseries/profile.h"
#include "memory/bitstream.h"
#include "networking/network_event.h"
#include "scenario/scenario.h"
#include "structures/structure_bsp_definitions.h"


/* constants */

enum
{
	k_simulation_update_estimated_size= 6144,
};

/* enums */

/* structures */

/* prototypes */

/* public code */

void simulation_write_quantized_position(
	c_bitstream* packet,
	real_point3d const* position,
	int32 axis_encoding_bit_count,
	bool fixup_quantized_position_inside_bsp)
{
	long_point3d point_quantization;

	real_rectangle3d* bounds = &global_structure_bsp_get()->world_bounds;
	
	quantize_real_point3d(position, bounds, axis_encoding_bit_count, &point_quantization);

	if (fixup_quantized_position_inside_bsp)
	{
		real_point3d quantized_point;

		dequantize_real_point3d(&point_quantization, bounds, axis_encoding_bit_count, &quantized_point);

		if (scenario_leaf_index_from_point(&quantized_point)==NONE &&
			scenario_leaf_index_from_point(position)!=NONE)
		{
			static long_point3d wiggle_table[14] =
			{
				{ { 0, 0, 1 } },
				{ { 1, 1, 1 } },
				{ { -1, 1, 1 } },
				{ { 1, -1, 1 } },
				{ { -1, -1, 1 } },
				{ { 1, 1, -1 } },
				{ { -1, 1, -1 } },
				{ { 1, -1, -1 } },
				{ { -1, -1, -1 } },
				{ { 1, 0, 0 } },
				{ { 0, 1, 0 } },
				{ { -1, 0, 0 } },
				{ { 0, -1, 0 } },
				{ { 0, 0, -1 } }
			};

			for (int32 wiggle_index= 0; wiggle_index<NUMBEROF(wiggle_table); ++wiggle_index)
			{
				bool wiggle_point_quantization_is_valid = true;

				long_point3d wiggled_point_quantization;

				for (int32 axis=0; axis<NUMBEROF(point_quantization.n); ++axis)
				{
					wiggled_point_quantization.n[axis] = point_quantization.n[axis] + wiggle_table[wiggle_index].n[axis];

					// Ivalid if outside bit range
					if (wiggled_point_quantization.n[axis]<0 || wiggled_point_quantization.n[axis]>=(int32)FLAG(axis_encoding_bit_count))
					{
						wiggle_point_quantization_is_valid = false;
						break;
					}
				}

				if (wiggle_point_quantization_is_valid)
				{
					dequantize_real_point3d(&wiggled_point_quantization, bounds, axis_encoding_bit_count, &quantized_point);

					if (scenario_leaf_index_from_point(&quantized_point))
					{
						point_quantization = wiggled_point_quantization;
						break;
					}
				}
			}
		}
	}

	packet->write_point3d("point-quantization", &point_quantization, axis_encoding_bit_count);

	return;
}

void simulation_read_quantized_position(
	class c_bitstream* packet,
	real_point3d* position,
	int32 axis_encoding_bit_count)
{
	long_point3d point_quantization;

	packet->read_point3d("point-quantization", &point_quantization, axis_encoding_bit_count);
	dequantize_real_point3d(&point_quantization, &global_structure_bsp_get()->world_bounds, axis_encoding_bit_count, position);

	return;
}

void __cdecl simulation_player_update_encode(c_bitstream* packet, const simulation_player_update* player_update)
{
	ASSERT(packet);
	ASSERT(player_update);
	ASSERT(VALID_INDEX(player_update->update_type, k_simulation_player_update_type_count));

	INVOKE(0x1E06AB, 0x1C7B6B, simulation_player_update_encode, packet, player_update);
	return;
}

bool __cdecl simulation_player_update_decode(c_bitstream* packet, simulation_player_update* player_update)
{
	ASSERT(packet);
	ASSERT(player_update);

	return INVOKE(0x1E078A, 0x1C7C4A, simulation_player_update_decode, packet, player_update);
}

void __cdecl player_action_encode(c_bitstream* packet, const struct player_action* action)
{
	INVOKE(0x1DFE4C, 0x0, player_action_encode, packet, action);
}

bool __cdecl player_action_decode(c_bitstream* packet, struct player_action* action)
{
	return INVOKE(0x1E01CB, 0x0, player_action_decode, packet, action);
}

void __cdecl simulation_machine_update_encode(c_bitstream* packet, const struct simulation_machine_update* machine_update)
{
	INVOKE(0x1E08E7, 0x0, simulation_machine_update_encode, packet, machine_update);
}

bool __cdecl simulation_machine_update_decode(c_bitstream* packet, struct simulation_machine_update* machine_update)
{
	return INVOKE(0x1E0935, 0x0, simulation_machine_update_decode, packet, machine_update);
}

bool player_action_compare(
	struct player_action const* action1,
	struct player_action* action2)
{
	bool result = false;

	ASSERT(action1);
	ASSERT(action2);

	if (
		c_bitstream::compare_quantized_reals(action1->facing.yaw, action2->facing.yaw, 0.f, (2.f* _pi), 13, false, true) &&
		c_bitstream::compare_quantized_reals(action1->facing.pitch, action2->facing.pitch, -_pi, _pi, 12, false, false) &&
		c_bitstream::compare_quantized_reals(action1->throttle.x, action2->throttle.x, -1.f, 1.f, 5, true, false) &&
		c_bitstream::compare_quantized_reals(action1->throttle.y, action2->throttle.y, -1.f, 1.f, 5, true, false) &&
		c_bitstream::compare_quantized_reals(action1->trigger, action2->trigger, 0.f, 1.f, 5, false, false) && 
		c_bitstream::compare_quantized_reals(action1->secondary_trigger, action2->secondary_trigger, 0.f, 1.f, 5, false, false) &&
		c_bitstream::compare_quantized_reals(action1->aim_assist_data.primary_auto_aim_level, action2->aim_assist_data.primary_auto_aim_level, 0.f, 1.f, 4, false, false) &&
		c_bitstream::compare_quantized_reals(action1->aim_assist_data.secondary_auto_aim_level, action2->aim_assist_data.secondary_auto_aim_level, 0.f, 1.f, 4, false, false)
	)
	{
		result = true;
		
		action2->facing = action1->facing;
		action2->throttle = action1->throttle;
		action2->trigger = action1->trigger;
		action2->secondary_trigger = action1->secondary_trigger;
		action2->aim_assist_data.primary_auto_aim_level = action1->aim_assist_data.primary_auto_aim_level;
		action2->aim_assist_data.secondary_auto_aim_level = action1->aim_assist_data.secondary_auto_aim_level;
	}
	return result;
}

bool simulation_update_compare(
	struct simulation_update const* update1,
	struct simulation_update* update2)
{
	bool result = true;

	if (result)
	{
		if (update1->player_action_mask != update2->player_action_mask)
		{
			result = false;
		}
	}

	if (result)
	{
		for (int32 i = 0; i < NUMBEROF(update1->player_actions); ++i)
		{
			if (TEST_BIT(update1->player_action_mask, i))
			{
				result = result && player_action_compare(
					&update1->player_actions[i], 
					&update2->player_actions[i]
				);
			}
		}
	}

	return result;
}


void __cdecl simulation_update_encode(
	c_bitstream* packet,
	const struct simulation_update* update)
{
	//INVOKE(0x1E0998, 0x0, synchronous_update_encode_internal, packet, update);

	const int32 starting_pos = packet->get_space_used_in_bits();

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

	const int32 pre_queues_encoded_size = (7 - starting_pos + packet->get_space_used_in_bits()) / 8;
	ASSERT(pre_queues_encoded_size > 0);

	if (pre_queues_encoded_size > k_simulation_update_estimated_size)
	{
		event(
			_event_fatal,
			"networking:simulation:encoding: encoded simulation update (no queues) exceeding estimate [%d > %d]",
			pre_queues_encoded_size,
			k_simulation_update_estimated_size);
	}

	update->bookkeeping_simulation_queue.encode(packet);
	update->game_simulation_queue.encode(packet);

	return;
}

bool __cdecl simulation_update_decode(
	c_bitstream* packet, 
	struct simulation_update* update)
{
	//return INVOKE(0x1E0AA2, 0x0, synchronous_update_decode_internal, packet, update);
	
	bool result = true;

	ASSERT(packet);
	ASSERT(update);

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
	
	// Validation
	result = result && update->bookkeeping_simulation_queue.decode(packet);
	result = result && update->game_simulation_queue.decode(packet);
	result = result && !packet->error_occurred();
	result = result && update->verify_game_time >= 0;
	result = result && update->update_number >= 0;

	// If something went wrong dispose of the queues
	if (!result)
	{
		update->bookkeeping_simulation_queue.dispose();
		update->game_simulation_queue.dispose();
	}

	return result;
}

/* private code */

