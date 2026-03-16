#include "stdafx.h"
#include "simulation_players.h"

#include "simulation.h"
#include "simulation_view.h"
#include "simulation_world.h"

#include "cartographer/discord/discord_interface.h"
#include "memory/bitstream.h"
#include "networking/logic/life_cycle_manager.h"
#include "networking/session/network_session.h"
#include "shell/shell.h"

#include "H2MOD/Modules/EventHandler/EventHandler.hpp"

/* prototypes */

void simulation_player_joined_game_patch_calls(void);
void simulation_player_left_game_patch_calls(void);

/* public code */

void simulation_players_apply_patches(void)
{
	simulation_player_joined_game_patch_calls();
	simulation_player_left_game_patch_calls();
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


void c_simulation_player::set_active(
	bool active)
{
	ASSERT(exists());
	ASSERT(m_world && m_world->is_authority());
	ASSERT(active!=m_active);
	ASSERT(!m_pending_deletion);

	if (active)
	{
		s_simulation_world_view_iterator iterator;
		m_world->iterator_begin(&iterator, FLAG(_simulation_world_type_synchronous_authority) | FLAG(_simulation_world_type_distributed_authority));
		
		c_simulation_view* view;
		while (m_world->iterator_next(&iterator, &view))
		{
			ASSERT(view);

			if (view->client_in_game())
			{
				uint32 player_mask = view->get_acknowledged_player_mask();
				ASSERT(TEST_BIT(player_mask, m_player_index));
			}
		}

		// TODO
		//ASSERT(m_world->player_is_in_game(m_player_datum_index, &m_player_identifier));
	}

	m_active = active;

	return;
}

void c_simulation_player::destroy(
	void)
{
	ASSERT(exists());

	m_player_index = NONE;
	m_player_datum_index = NONE;
	m_player_type = (e_simulation_player_type)NONE;
	m_world = NULL;

	return;
}

void c_simulation_player::handle_local_input(
	struct player_action const* action)
{
	ASSERT(action);
	ASSERT(exists());
	ASSERT(is_local());

	if (m_player_type==_simulation_player_type_local_predicted ||
		m_player_type==_simulation_player_type_local_zombie)
	{
		bool decode_success;
		uint8 temporary_storage[sizeof(player_action)];
		
		c_bitstream temporary_stream(temporary_storage, sizeof(temporary_storage));

		temporary_stream.begin_writing(k_bitstream_default_alignment);
		player_action_encode(&temporary_stream, action);
		temporary_stream.finish_writing(NULL);

		temporary_stream.begin_reading();
		decode_success= player_action_decode(&temporary_stream, &m_current_action);

		ASSERT(!temporary_stream.error_occurred());

		temporary_stream.finish_reading();

		ASSERT(decode_success);

		m_current_action_time = game_time_get();
	}

	return;
}

bool __cdecl simulation_players_apply_update(simulation_player_update* player_update)
{
	return INVOKE(0x1E22E2, 0x1C930E, simulation_players_apply_update, player_update);
}

void __cdecl simulation_player_joined_game(datum player_index)
{
	s_simulation_globals* simulation_globals = simulation_get_globals();
	
	ASSERT(simulation_globals->world);

	if (simulation_globals->initialized && !simulation_globals->loading_saved_game)
	{
		simulation_globals->world->create_player(player_index);
		if (!shell_is_dedicated_server())
		{
			// Update discord player counts
			discord_interface_set_player_counts();
		}
	}

	// Remove this when new custom variant settings are finished
	c_network_session* session = NULL;
	if (network_life_cycle_in_squad_session(&session))
	{
		EventHandler::NetworkPlayerEventExecute(EventExecutionType::execute_after, session->get_player_membership(player_index)->peer_index, EventHandler::NetworkPlayerEventType::add);
	}
	return;
}

void __cdecl simulation_player_left_game(datum player_index)
{
	s_simulation_globals* simulation_globals = simulation_get_globals();
	ASSERT(simulation_globals->world);

	if (simulation_globals->initialized && !simulation_globals->loading_saved_game)
	{
		simulation_globals->world->delete_player(player_index);
		if (!shell_is_dedicated_server())
		{
			// Update discord player counts
			discord_interface_set_player_counts();
		}
	}
	return;
}

/* private code */

void simulation_player_joined_game_patch_calls(void)
{
	PatchCall(Memory::GetAddress(0x56447, 0x5E93F), simulation_player_joined_game);
	PatchCall(Memory::GetAddress(0x5647F, 0x5E977), simulation_player_joined_game);
	PatchCall(Memory::GetAddress(0x57E85, 0x6037D), simulation_player_joined_game);
	return;
}

void simulation_player_left_game_patch_calls(void)
{
	PatchCall(Memory::GetAddress(0x5633A, 0x5E832), simulation_player_left_game);
	return;
}
