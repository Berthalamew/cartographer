#include "stdafx.h"
#include "simulation_watcher.h"

#include "simulation_players.h"
#include "simulation_world.h"

/* public code */

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

uint32 c_simulation_watcher::get_machine_valid_mask(
	void) const
{
	return m_machine_valid_mask;
}
