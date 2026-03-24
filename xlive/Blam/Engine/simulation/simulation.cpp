#include "stdafx.h"
#include "simulation.h"

#include "simulation_queue_global_events.h"
#include "simulation_entity_database.h"
#include "simulation_event_handler.h"
#include "simulation_watcher.h"
#include "simulation_world.h"

#include "cseries/profile.h"
#include "game/game.h"
#include "game/players.h"
#include "math/random_math.h"
#include "memory/bitstream.h"
#include "networking/network_event.h"
#include "objects/objects.h"
#include "units/units.h"
#include "simulation/game_interface/simulation_game_action.h"

/* prototypes */

static void simulation_synchronous_game_patches(void);

/* public code */

void simulation_apply_patches(
	void)
{
	simulation_event_handler_apply_patches();
	simulation_world_apply_patches();
	simulation_entity_database_apply_patches();
	simulation_game_action_apply_patches();

	WriteJmpTo(Memory::GetAddress(0x1AE6D8, 0x1A8932), simulation_reset);
	simulation_synchronous_game_patches();
	return;
}

s_simulation_globals* simulation_get_globals()
{
	return Memory::GetAddress<s_simulation_globals*>(0x5178D0, 0x520B60);
}

c_simulation_world* simulation_get_world()
{
	return simulation_get_globals()->world;
}

bool simulation_engine_initialized()
{
	return simulation_get_globals()->initialized;
}

bool simulation_reset_in_progress()
{
	return simulation_get_globals()->simulation_reset_in_progress;
}

void __cdecl simulation_update(void)
{
	INVOKE(0x1AE7C5, 0x1A8A1F, simulation_update);
	return;
}

bool simulation_starting_up(
	void)
{
	const s_simulation_globals* simulation_globals = simulation_get_globals();

	bool result = false;
	if (simulation_globals->initialized)
	{
		ASSERT(simulation_globals->world);
		if (!simulation_globals->simulation_aborted && simulation_globals->world->exists())
		{
			result = !simulation_globals->world->is_active();
		}
	}

	return result;
}

bool simulation_aborted(
	void)
{
	const s_simulation_globals* simulation_globals = simulation_get_globals();
	return simulation_globals->initialized && simulation_globals->simulation_aborted;
}

void simulation_notify_reset_complete(void)
{
	s_simulation_globals* simulation_globals = simulation_get_globals();

	if (!game_is_playback())// // make sure simulation is still in resetting
	{
		ASSERT(simulation_globals->simulation_reset_in_progress);

		if (!simulation_globals->world->is_authority())
		{
			// dont need this if we are not authority
			simulation_globals->world->send_player_acknowledgements(true);
		}
		else
		{
			event(
				_event_warning,
				"networking:simulation: not calling send_player_acknowledgements() on reset complete as we have become the authority"
			);
		}
	}
	simulation_globals->simulation_reset_in_progress = false;

	return;
}

void simulation_notify_reset_initiate(
	void)
{
	s_simulation_globals* simulation_globals = simulation_get_globals();
	simulation_globals->simulation_reset_in_progress = true;

	return;
}

void simulation_notify_going_active(
	void)
{
	if (game_is_campaign() && game_is_cooperative())
	{
		players_update_for_checkpoint();
	}

	return;
}

void simulation_reset_immediate(
	void)
{
	s_simulation_globals* simulation_globals = simulation_get_globals();

	ASSERT(simulation_globals->initialized);

	if (simulation_globals->simulation_reset_in_progress)
	{
		event(
			_event_message,
			"networking:simulation: calling simulation_reset_immediate() with a simulation reset already in progress",

		);
	}

	simulation_globals->simulation_reset_in_progress = true;

	event(_event_message, "networking:simulation: resetting simulation world");

	ASSERT(simulation_globals->world);
	ASSERT(simulation_globals->world->exists());
	ASSERT(!simulation_globals->world->is_authority());

	simulation_globals->world->reset_world();

	if (simulation_globals->world->runs_simulation())
	{
		simulation_queue_game_global_event_insert(_simulation_queue_game_global_event_type_reset_map);
		// ### TODO figure out these
		// simulation_gamestate_entities_build_clear_flags();
		// simulation_queue_gamestates_delete_insert();
		simulation_queue_game_global_event_insert(_simulation_queue_game_global_event_type_simulation_reset_complete);
	}
	else
	{
		simulation_globals->simulation_reset_in_progress = false;
	}

	return;
}

// FIXME: figure out why this function is being called on clients...
void __cdecl simulation_reset(
	void)
{
	s_simulation_globals* simulation_globals = simulation_get_globals();
	ASSERT(simulation_globals->world);
	//ASSERT(simulation_globals->world->is_authority());	 FIXME

	if (simulation_globals->simulation_in_initial_state)
	{
		simulation_globals->simulation_in_initial_state = false;
	}
	else
	{
		// this will use the main game simulation reset code
		// but we don't need it
		//simulation_globals->simulation_reset_pending = true;

		// instead, call reset directly
		simulation_reset_immediate();
	}
	return;
}

bool simulation_in_progress(
	void)
{
	bool result = false;

	if (simulation_engine_initialized()
		&& game_in_progress()
		&& game_get_active_structure_bsp_index()!=NONE
		&& !simulation_get_globals()->simulation_aborted)
	{
		ASSERT(simulation_get_globals()->world);
		if (simulation_get_world()->is_active())
		{
			result = true;
		}
	}

	return result;
}

bool simulation_query_object_is_predicted(datum object_index)
{
	return game_is_predicted() && object_get(object_index)->object.simulation_entity_index != NONE;
}

void __cdecl simulation_process_input(uint32 player_action_mask, const player_action* player_actions)
{
	//INVOKE(0x1ADDA9, 0x1A8160, simulation_process_input, player_action_mask, player_actions);
	
	s_simulation_globals* simulation_globals = simulation_get_globals();
	
	ASSERT(simulation_globals->initialized);
	ASSERT(simulation_globals->world);
	ASSERT(game_in_progress());

	if (!simulation_globals->simulation_aborted && simulation_globals->world->exists())
	{
		simulation_globals->world->process_input(player_action_mask, player_actions);
	}

	return;
}

c_simulation_type_collection* simulation_get_type_collection()
{
	return c_simulation_type_collection::get();
}

void simulation_apply_before_game(
	const struct simulation_update* update)
{
	profile_attribute_enter(2, _profile_attribution_subsystem_9);

	s_simulation_globals* simulation_globals =simulation_get_globals();

	ASSERT(update != NULL);
	ASSERT(simulation_globals->initialized);
	ASSERT(simulation_globals->world);
	ASSERT(game_in_progress());

	simulation_globals->world->queues_update_statistics();

	for (int32 i = 0; i < k_maximum_players; i++)
	{
		datum control_unit_index = update->control_unit_index[i];
		if (TEST_BIT(update->unit_control_mask, i) && unit_try_and_get(control_unit_index))
		{
			unit_control(control_unit_index, &update->unit_control[i]);
		}
	}
	
	if (update->machine_update_valid)
	{
		players_set_machines(update->machine_update.machine_valid_mask, update->machine_update.identifiers);
	}

	// Player activation code
	/* Moved so we can activate in the queue
	s_simulation_globals* globals = simulation_get_globals();
	if (update->player_update_count > 0)
	{
		bool fatal_error = false;
		for (int32 player_update_index = 0; simulation_players_apply_update(&update->player_updates[player_update_index]); player_update_index++)
		{
			// Get out of here if we've overflown
			if (player_update_index >= update->player_update_count) 
			{
				fatal_error = true;
				break;
			}
		}
		
		//  Set bool to true ONLY if overflown is false, don't change otherwise
		if (!fatal_error)
		{
			globals->fatal_error = true;
		}
	}*/

	simulation_globals->world->apply_simulation_queue(&update->bookkeeping_simulation_queue);

	if (update->game_simulation_queue.queued_count() <= 0)
	{
		ASSERT(update->game_simulation_queue.queued_size_in_bytes() == 0);
	}
	else
	{
		ASSERT(update->game_simulation_queue.queued_size_in_bytes() > 0);

		simulation_globals->world->apply_simulation_queue(&update->game_simulation_queue);

		// purge any deletion pending object during this update
		// if simulation is not in progress
		if (!update->simulation_in_progress)
		{
			objects_purge_deleted_objects();
		}
	}

	if (update->flush_gamestate && !simulation_globals->world->is_authority())
	{
		ASSERT(!game_is_distributed());

		simulation_globals->world->gamestate_flush();
	}

	profile_attribute_exit(2, _profile_attribution_subsystem_9);

	return;
}

void simulation_apply_after_game(const struct simulation_update* update)
{
	// This never did anything
	return;
}

void simulation_build_update(
	struct simulation_update* update)
{
	profile_attribute_enter(2, _profile_attribution_subsystem_9);

	s_simulation_globals* simulation_globals = simulation_get_globals();

	ASSERT(simulation_globals->initialized);
	ASSERT(simulation_globals->world);

	vassert(!simulation_globals->simulation_aborted, "simulation aborted inside game update", NULL);

	ASSERT(simulation_globals->world->exists());
	ASSERT(game_in_progress());
	ASSERT(update);

	csmemset(update, 0, sizeof(*update));

	simulation_globals->world->build_update(update);

	if ((!simulation_globals->world->is_authority() || simulation_globals->world->is_playback()) &&
		(!simulation_globals->world->is_distributed() || simulation_globals->world->is_playback()) &&
		!simulation_globals->world->is_out_of_sync())
	{
		bool out_of_sync = false;

		if (update->flush_gamestate)
		{
			simulation_globals->world->gamestate_flush();
		}

		random_seed_allow_use();

		if (update->update_number!=simulation_globals->world->get_next_update_number())
		{
			event(
				_event_error,
				"simulation:global: OUT OF SYNC, update number differs, update [#%d] != next [#%d]",
				update->update_number,
				simulation_globals->world->get_next_update_number()
			);

			out_of_sync = true;
		}
		else if (update->verify_game_time!=simulation_globals->world->get_time())
		{
			event(
				_event_error,
				"simulation:global: OUT OF SYNC, update time differs, update [#%d] time [%d] != local time %d",
				update->update_number,
				update->verify_game_time,
				simulation_globals->world->get_time()
			);

			out_of_sync = true;
		}
		else if (update->verify_random_seed!=get_random_seed())
		{
			event(
				_event_error,
				"simulation:global: OUT OF SYNC, random seed differs, update [#%d] time [%d] seed [0x%08X] (local seed [0x%08X])",
				update->update_number,
				update->verify_game_time,
				update->verify_random_seed,
				get_random_seed()
			);

			out_of_sync = true;
		}

		random_seed_disallow_use();

		if (out_of_sync)
		{
			simulation_globals->world->go_out_of_sync();
		}
	}
	
	profile_attribute_exit(2, _profile_attribution_subsystem_9);

	return;
}

void simulation_update_aftermath(
	const struct simulation_update* update)
{
	// INVOKE(0x1ADEA9, 0x1A8260, simulation_update_aftermath, update);

	profile_attribute_enter(2, _profile_attribution_subsystem_9);

	s_simulation_globals* simulation_globals = simulation_get_globals();

	ASSERT(update);
	ASSERT(simulation_globals->initialized);
	ASSERT(simulation_globals->world);
	ASSERT(game_in_progress());

	if (simulation_globals->world->is_authority())
	{
		simulation_globals->world->distribute_update(update);
	}

	simulation_globals->world->advance_update(update);

	profile_attribute_exit(2, _profile_attribution_subsystem_9);

	return;
}

c_simulation_view* simulation_get_remote_view_by_channel(
	int32 network_channel_index)
{
	c_simulation_view* view= NULL;

	s_simulation_globals* simulation_globals = simulation_get_globals();

	if (simulation_globals->initialized)
	{
		ASSERT(simulation_globals->world);
		if (simulation_globals->world->exists())
		{
			view = simulation_globals->world->get_view_by_channel(network_channel_index);
		}
	}

	return view;
}

bool simulation_update_write_to_buffer(
	const struct simulation_update* update,
	int32 buffer_size,
	uint8* buffer,
	int32* out_update_length)
{
	c_bitstream message(buffer, buffer_size);

	ASSERT(update);
	ASSERT(buffer);
	ASSERT(out_update_length);
	
	message.begin_writing(k_default_profiles_count);
	simulation_update_encode(&message, update);
	
	*out_update_length = message.get_space_used_in_bytes();
	message.finish_writing(NULL);

	bool result = true;
	if (message.begin_consistency_check())
	{
		struct simulation_update encoded_update;
		csmemset(&encoded_update, 0, sizeof(encoded_update));
		if (simulation_update_decode(&message, &encoded_update))
		{
			message.finish_consistency_check();
			if (simulation_update_compare(update, &encoded_update))
			{
				// Consistency check complete
			}
			else
			{
				result = false;
			}
		}
		else
		{
			result = false;
		}
	}

	return result;
}

bool simulation_update_read_from_buffer(
	struct simulation_update* update,
	int32 buffer_size,
	uint8* buffer)
{
	c_bitstream message(buffer, buffer_size);
	
	ASSERT(update);
	ASSERT(buffer);
	
	csmemset(update, 0, sizeof(*update));
	message.begin_reading();

	bool result = false;
	if (simulation_update_decode(&message, update))
	{
		message.finish_reading();
		result = true;
	}
	else
	{
		event(_event_warning, "networking:simulation: failed to read simulation update, decode failed");
	}

	return result;
}

void simulation_update_pregame(
	void)
{
	ASSERT(!simulation_in_progress());

	s_simulation_globals* simulation_globals = simulation_get_globals();

	if (simulation_globals->initialized && game_in_progress() && !simulation_globals->simulation_aborted)
	{
		if (simulation_globals->watcher->need_to_generate_updates())
		{
			struct simulation_update update;
			simulation_build_update(&update);

			ASSERT(!update.simulation_in_progress);

			random_seed_allow_use();
			simulation_apply_before_game(&update);
			random_seed_disallow_use();
			
			simulation_update_aftermath(&update);
			c_simulation_world::destroy_update(&update);
		}
		else
		{
			simulation_globals->world->queues_update_statistics();
		}
	}
	return;
}

void simulation_destroy_update(
	struct simulation_update* update)
{
	ASSERT(update);
	s_simulation_globals* simulation_globals = simulation_get_globals();
	simulation_globals->world->destroy_world();
	
	return;
}

bool __cdecl simulation_get_machine_active_in_game(s_machine_identifier* machine_identifier)
{
	return INVOKE(0x1AE0CB, 0x1A8482, simulation_get_machine_active_in_game, machine_identifier);
}

void simulation_build_machine_update(
	bool* machine_update_valid,
	simulation_machine_update* machine_update)
{
	s_simulation_globals* simulation_globals = simulation_get_globals();

	ASSERT(simulation_globals->initialized);
	ASSERT(simulation_globals->world);
	ASSERT(simulation_globals->world->exists());
	ASSERT(simulation_globals->world->runs_simulation());
	ASSERT(simulation_globals->watcher);
	ASSERT(game_in_progress());

	simulation_globals->watcher->generate_machine_update(machine_update_valid, machine_update);
	return;
}

void simulation_build_player_updates(
	int32* player_update_count,
	int32 maximum_player_update_count,
	simulation_player_update* player_updates)
{
	s_simulation_globals* simulation_globals = simulation_get_globals();

	ASSERT(simulation_globals->initialized);
	ASSERT(simulation_globals->world);
	ASSERT(simulation_globals->world->runs_simulation());
	ASSERT(simulation_globals->watcher);
	ASSERT(game_in_progress());

	simulation_globals->watcher->generate_player_updates(player_update_count, maximum_player_update_count, player_updates);
	for (int32 i = 0; i < *player_update_count; ++i)
	{
		simulation_queue_player_update_insert(&player_updates[i]);
	}

	return;
}

void simulation_fatal_error(
	void)
{
	s_simulation_globals* simulation_globals = simulation_get_globals();

	ASSERT(simulation_globals->initialized);
	event(_event_error, "simulation:global: fatal error raised at time [%d]", game_time_get());

	simulation_globals->simulation_fatal_error = true;

	return;
}

void __cdecl simulation_remove_view_from_world(
	c_simulation_view* view)
{
	INVOKE(0x1ADF7E, 0x0, simulation_remove_view_from_world, view);
	return;
}

/* private code */

static void simulation_synchronous_game_patches(
	void)
{
	PatchCall(Memory::GetAddress(0x1AE002), simulation_update_encode);
	PatchCall(Memory::GetAddress(0x1ED08E), simulation_update_encode);
	PatchCall(Memory::GetAddress(0x1AE084), simulation_update_decode);
	PatchCall(Memory::GetAddress(0x1ED0A3), simulation_update_decode);
	return;
}
