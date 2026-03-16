#pragma once
#include "simulation_players.h"
#include "networking/delivery/network_channel.h"
#include "networking/network_constants.h"

/* classes */

class c_simulation_watcher : c_network_channel_owner
{
public:
	bool need_to_generate_updates(void);

	void generate_player_updates(int32* player_update_count, int32 maximum_player_update_count, struct simulation_player_update* player_updates);

	void generate_machine_update(bool* machine_update_valid, struct simulation_machine_update* machine_update);

	void maintain_connection(void);

	int32 get_machine_index_by_identifier(struct s_machine_identifier const* remote_machine_identifier) const;
	bool get_player_is_in_game(int32 player_index, struct s_player_identifier const* player_identifier) const;

	uint32 get_machine_valid_mask(void) const;

private:
	class c_simulation_world* m_world;
	class c_network_observer* m_observer;
	class c_network_session* m_session;
	int32 m_machine_last_local_membership_update_number;
	int32 m_machine_last_membership_update_number;
	int32 m_player_last_local_membership_update_number;
	uint32 m_machine_valid_mask;
	int32 m_local_machine_index;
	s_machine_identifier m_machine_identifiers[k_network_maximum_machines_per_session];
	bool m_machine_update_pending;
	s_player_collection m_player_collection;
	uint32 m_player_collection_machine_valid_mask;
	s_machine_identifier m_player_collection_machine_identifiers[k_network_maximum_machines_per_session];
	bool m_changes_pending_acknowledgement;
};
ASSERT_STRUCT_SIZE(c_simulation_watcher, 0xB3C);
