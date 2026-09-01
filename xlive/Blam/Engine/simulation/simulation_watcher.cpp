#include "stdafx.h"
#include "simulation_watcher.h"

#include "simulation_players.h"
#include "simulation_world.h"

#include "game/game.h"
#include "game/game_options.h"
#include "networking/network_globals.h"
#include "networking/network_event.h"
#include "networking/logic/network_life_cycle.h"
#include "networking/session/network_session.h"

/* public code */

void c_simulation_watcher::initialize_watcher(
	class c_simulation_world* world)
{
	ASSERT(world);
	ASSERT(m_world==NULL);

	m_world = world;
	m_session = NULL;
	m_observer = NULL;
	
	reset_tracking_arrays();

	return;
}

void c_simulation_watcher::destroy_watcher(
	void)
{
	if (m_observer)
	{
		m_observer->deregister_owner(_network_observer_owner_simulation, this);
		m_observer = NULL;
	}

	m_world = NULL;
	m_session = NULL;
	
	return;
}

void c_simulation_watcher::setup_connection(
	void)
{
	ASSERT(m_session==NULL);

	if ((game_is_campaign() || game_is_multiplayer()) && !game_is_playback())
	{
		c_network_session* session = NULL;

		if (network_initialized() && network_life_cycle_in_game_session(&session) && session->get_session_membership_unsafe(NULL, NULL))
		{
			m_session = session;
		}
	}

	if (m_session)
	{
		int32 local_peer_index;
		int32 host_peer_index;

		c_network_observer* observer = NULL;

		ASSERT(m_world!=NULL);

		const s_session_membership* membership = m_session->get_session_membership_unsafe(&local_peer_index, &host_peer_index);

		ASSERT(membership!=NULL);

		network_life_cycle_get_observer(&observer);

		ASSERT(observer!=NULL);

		m_observer = observer;
		m_observer->register_owner(_network_observer_owner_simulation, this);

		s_machine_identifier machine_identifiers[NUMBEROF(m_machine_identifiers)];
		uint32 machine_valid_mask = MASK(membership->peer_count);

		csmemset(&machine_identifiers, 0, sizeof(machine_identifiers));

		for (int32 peer_index = 0; peer_index<membership->peer_count; ++peer_index)
		{
			transport_secure_address_extract_identifier(&membership->peers[peer_index].secure_address, (s_transport_unique_identifier*)&machine_identifiers[peer_index]);
		}

		ASSERT(host_peer_index>=0 && host_peer_index<membership->peer_count);

		m_machine_last_local_membership_update_number = m_session->get_local_session_membership_update_number();
		m_machine_last_membership_update_number = membership->update_number;
		m_machine_valid_mask = machine_valid_mask;
		m_local_machine_index = local_peer_index;
		csmemcpy(m_machine_identifiers, machine_identifiers, sizeof(m_machine_identifiers));

		m_world->set_machine_identifier(&m_machine_identifiers[m_local_machine_index]);
		m_world->set_machine_index(m_local_machine_index);

		event(_event_message, "simulation:watcher: setup_connection initial machines 0x%08X (local #%d)", m_machine_valid_mask, m_local_machine_index);

		m_machine_update_pending = true;
		m_changes_pending_acknowledgement = true;
	}
	else
	{
		s_game_options const* options = game_options_get();

		m_machine_valid_mask = options->machines.valid_machine_mask;
		csmemcpy(m_machine_identifiers, options->machines.machines, sizeof(m_machine_identifiers));
		m_machine_update_pending = false;
		m_local_machine_index = NONE;

		if (options->machines.local_machine_exists)
		{
			for (int32 machine_index= 0; machine_index<NUMBEROF(m_machine_identifiers); ++machine_index)
			{
				if (TEST_BIT(m_machine_valid_mask, machine_index) &&
					!csmemcmp(&m_machine_identifiers[machine_index], &options->machines.local_machine_identifier, sizeof(m_machine_identifiers[machine_index])))
				{
					ASSERT(m_local_machine_index==NONE);

					m_local_machine_index = machine_index;
				}
			}
		}

		if (m_local_machine_index!=NONE)
		{
			m_world->set_machine_identifier(&m_machine_identifiers[m_local_machine_index]);
			m_world->set_machine_index(m_local_machine_index);
		}
	}

	return;
}

bool c_simulation_watcher::need_to_generate_updates(void)
{
	ASSERT(m_world->exists());

	bool result = INVOKE_TYPE(0x1D4B42, 0x1C188C, bool(__thiscall*)(c_simulation_watcher*), this);
	return (result || !m_world->simulation_queues_empty()) && m_world->is_distributed() && m_world->is_authority();
}

void c_simulation_watcher::generate_player_updates(int32* player_update_count, int32 maximum_player_update_count, simulation_player_update* player_updates)
{
	INVOKE_TYPE(0x1D5D24, 0x1C2932, void(__thiscall*)(c_simulation_watcher*, int32*, int32, simulation_player_update*), this, player_update_count, maximum_player_update_count, player_updates);
	return;
}

void c_simulation_watcher::generate_machine_update(bool* machine_update_valid, simulation_machine_update* machine_update)
{
	INVOKE_TYPE(0x1D5CDC, 0x1C28EA, void(__thiscall*)(c_simulation_watcher*, bool*, simulation_machine_update*), this, machine_update_valid, machine_update);
	return;
}

void c_simulation_watcher::maintain_connection(void)
{
	return INVOKE_TYPE(0x1D6531, 0x1C3141, void(__thiscall*)(c_simulation_watcher*), this);
}

void c_simulation_watcher::reset_tracking_arrays(
	void)
{
	m_machine_last_local_membership_update_number = NONE;
	m_machine_last_membership_update_number = NONE;
	m_player_last_local_membership_update_number = NONE;
	m_machine_valid_mask = 0;
	m_local_machine_index = NONE;
	csmemset(m_machine_identifiers, 0, sizeof(m_machine_identifiers));
	m_machine_update_pending = false;
	
	simulation_player_collection_clear(&m_player_collection);

	m_player_collection_machine_valid_mask = 0;
	csmemset(m_player_collection_machine_identifiers, 0, sizeof(m_player_collection_machine_identifiers));
	
	event(_event_message, "simulation:watcher: tracking arrays reset");
	m_changes_pending_acknowledgement = true;

	return;
}


int32 c_simulation_watcher::get_machine_index_by_identifier(
	struct s_machine_identifier const* remote_machine_identifier) const
{
	int32 machine_index = NONE;

	ASSERT(remote_machine_identifier);

	for (int32 test_machine_index = 0; test_machine_index<NUMBEROF(m_machine_identifiers); ++test_machine_index)
	{
		if (TEST_BIT(m_machine_valid_mask, test_machine_index) && 
			!csmemcmp(&m_machine_identifiers[test_machine_index], remote_machine_identifier, sizeof(*remote_machine_identifier)))
		{
			ASSERT(machine_index==NONE);
			machine_index = test_machine_index;
		}
	}
	
	return machine_index;
}

bool c_simulation_watcher::get_player_is_in_game(
	int32 player_index,
	s_player_identifier const* player_identifier) const
{
	bool player_in_game = false;

	ASSERT(player_index>=0 && player_index<k_maximum_players);
	ASSERT(player_identifier);

	if (TEST_BIT(m_player_collection.player_valid_mask, player_index))
	{
		s_player_collection_player const* player = &m_player_collection.players[player_index];

		if (csmemcmp(player_identifier, player, sizeof(*player)))
		{
			if (!m_player_collection.players[player_index].left_game)
			{
				player_in_game = true;
			}
		}
	}

	return player_in_game;
}


bool c_simulation_watcher::get_machine_is_host(
	struct s_machine_identifier const* machine_identifier) const
{
	bool is_host = false;

	ASSERT(machine_identifier);

	if (m_session && m_session->established() && get_machine_index_by_identifier(machine_identifier)!=NONE)
	{
		int32 host_peer_index;
		s_transport_unique_identifier host_machine_identifier;

		s_session_membership const* membership = m_session->get_session_membership(NULL, &host_peer_index);

		ASSERT(membership);

		ASSERT(host_peer_index>=0 && host_peer_index<membership->peer_count);

		transport_secure_address_extract_identifier(&membership->peers[host_peer_index].secure_address, &host_machine_identifier);

		if (!csmemcmp(machine_identifier, &host_machine_identifier, sizeof(*machine_identifier)))
		{
			is_host = true;
		}
	}

	return is_host;
}

bool c_simulation_watcher::get_machine_connectivity(
	struct s_machine_identifier const* machine_identifier) const
{
	bool connectivity = false;

	ASSERT(machine_identifier);
	
	if (m_observer)
	{
		if (get_machine_index_by_identifier(machine_identifier)!=NONE)
		{
			int32 observer_channel_index = m_observer->observer_channel_find_by_machine_identifier(_network_observer_owner_simulation, (s_transport_unique_identifier const*)machine_identifier);

			if (observer_channel_index!=NONE)
			{
				if (m_observer->observer_channel_connected(_network_observer_owner_simulation, observer_channel_index))
				{
					connectivity = true;
				}
			}
		}
	}

	return connectivity;
}

uint32 c_simulation_watcher::get_machine_valid_mask(
	void) const
{
	return m_machine_valid_mask;
}
