#pragma once

#include "machine_id.h"
#include "game/game_results.h"
#include "game/players.h"
#include "memory/ring_buffer.h"
#include "networking/delivery/network_channel.h"
#include "networking/messages/network_message_type_collection.h"
#include "networking/replication/replication_control_view.h"
#include "networking/replication/replication_entity_manager_view.h"
#include "networking/replication/replication_event_manager_view.h"
#include "networking/replication/replication_scheduler.h"
#include "simulation/simulation_view_telemetry.h"

/* enums */

enum e_simulation_view_type : int16
{
	_simulation_view_type_none = 0,
	_simulation_view_type_synchronous_to_remote_authority,
	_simulation_view_type_synchronous_to_remote_client,
	_simulation_view_type_distributed_to_remote_authority,
	_simulation_view_type_distributed_to_remote_client,
	k_simulation_view_type_count,
};

enum e_simulation_view_establishment_mode
{
	_simulation_view_establishment_mode_none = 0,
	_simulation_view_establishment_mode_connected,
	_simulation_view_establishment_mode_established,
	_simulation_view_establishment_mode_ready,
	_simulation_view_establishment_mode_joining,
	_simulation_view_establishment_mode_active,
	k_simulation_view_establishment_mode_count,
	k_simulation_view_establishment_mode_bits = 3,
};


enum e_simulation_view_reason
{
	_simulation_view_reason_none = 0,
	_simulation_view_reason_channel_disconnect,
	_simulation_view_reason_out_of_sync,
	_simulation_view_reason_failed_to_join,
	_simulation_view_reason_synchronous_block,
	_simulation_view_reason_synchronous_catchup_fail,
	_simulation_view_reason_remote_ended,
	_simulation_view_reason_mode_error,
	_simulation_view_reason_player_activation_error,
	_simulation_view_reason_replication_entity_error,
	_simulation_view_reason_replication_event_error,
	_simulation_view_reason_replication_game_results_error,
	_simulation_view_reason_no_longer_authority,
	k_simulation_view_reason_count,
};

/* classes */

class c_simulation_view
{
public:
	char const* get_view_description(void) const;
	
	char const* get_death_reason_string(uint32 death_reason) const;

	bool observer_channel_game_results_backlogged(void);
	void observer_channel_set_waiting_on_backlog(e_network_message_type message_type);

	void send_message(e_network_message_type message_type, int32 message_size, const void* message_payload);

	void kill_view(e_simulation_view_reason death_reason);

	void set_view_establishment(e_simulation_view_establishment_mode establishment_mode, int32 establishment_identifier);

	void update(void);

	uint32 get_acknowledged_player_mask(void) const;

	bool handle_synchronous_update(struct simulation_update const* update);

	void dispatch_synchronous_actions(uint32 valid_user_mask, struct player_action const* user_actions);

	void dispatch_synchronous_update(struct simulation_update const* update);

	uint32 synchronous_client_get_acknowledged_update_number(void) const;

	void synchronous_client_block(bool block);

	void update_view_activation_state(void);

	bool synchronous_catchup_in_progress(void) const;

	void synchronous_catchup_terminate(void);
	
	void synchronous_catchup_send_data(void);

	bool synchronous_catchup_complete(struct simulation_update const* update);

	void distributed_join_abort(void);

	bool exists(
		void) const
	{
		return m_view_type!=_simulation_view_type_none;
	}
	
	bool active(
		void) const
	{
		ASSERT(exists());
		return m_simulation_active;
	}

	int32 get_machine_index(
		void) const
	{
		ASSERT(exists());
		ASSERT(m_remote_machine_index!=NONE);

		return m_remote_machine_index;
	}

	e_simulation_view_type view_type(
		void) const
	{
		ASSERT(exists());
		return m_view_type;
	}

	bool is_client_view(
		void) const
	{
		return
			m_view_type == _simulation_view_type_synchronous_to_remote_client ||
			m_view_type == _simulation_view_type_distributed_to_remote_client;
	}

	bool is_distributed(
		void) const
	{
		ASSERT(exists());

		return
			m_view_type == _simulation_view_type_distributed_to_remote_authority ||
			m_view_type == _simulation_view_type_distributed_to_remote_client;
	}
	
	class c_simulation_world* get_world(
		void) const
	{
		return m_world;
	}

	int32 get_channel_index(
		void) const
	{
		return m_channel_index;
	}

	e_simulation_view_establishment_mode get_view_establishment_mode(
		void) const
	{
		ASSERT(exists());
		ASSERT(m_view_establishment_mode>=0 && m_view_establishment_mode<k_simulation_view_establishment_mode_count);

		return m_view_establishment_mode;
	}

	int32 get_view_establishment_identifier(
		void) const
	{
		ASSERT(exists());
		ASSERT(m_view_establishment_identifier==NONE || m_view_establishment_identifier>=0);

		return m_view_establishment_identifier;
	}
	
	bool is_dead(
		e_simulation_view_reason* death_reason) const
	{
		ASSERT(exists());
		ASSERT(m_view_death_reason >= 0 && m_view_death_reason < k_simulation_view_reason_count);

		if (death_reason)
		{
			*death_reason = m_view_death_reason;
		}

		return m_view_death_reason!=_simulation_view_reason_none;
	}

	bool ready_to_establish(
		void) const
	{
		return m_view_establishment_mode==_simulation_view_establishment_mode_connected &&
			m_remote_establishment_mode==_simulation_view_establishment_mode_connected;
	}

	bool client_in_game(
		void) const
	{
		bool in_game = false;

		ASSERT(exists());
		ASSERT(is_client_view());
		
		switch (m_view_type)
		{
		case _simulation_view_type_synchronous_to_remote_client:
			in_game = m_view_establishment_mode >= _simulation_view_establishment_mode_active;
			break;
		case _simulation_view_type_distributed_to_remote_client:
			in_game = m_view_establishment_mode >= _simulation_view_establishment_mode_ready;
			break;
		default:
			unreachable();
		}

		return in_game;
	}

	bool established(
		void) const
	{
		ASSERT(exists());

		return m_channel_index != NONE ? m_channel_interface.established() : false;
	}


private:
	int16 m_identifier;
	e_simulation_view_type m_view_type;
	datum m_view_datum_index;
	class c_simulation_distributed_view* m_distributed_view;
	class c_simulation_world* m_world;
	uint32 m_world_view_index;
	s_machine_identifier m_remote_machine_identifier;
	int32 m_remote_machine_index;
	// TODO: This should really be EVENTS_ENABLED
#ifdef SIMULATION_VIEW_DEBUG
	char m_remote_machine_name[16];
	char m_view_description[32];
#endif
	class c_network_observer* m_observer;
	uint32 m_observer_channel_index;
	e_simulation_view_reason m_view_death_reason;
	e_simulation_view_establishment_mode m_view_establishment_mode;
	int32 m_view_establishment_identifier;
	e_simulation_view_establishment_mode m_remote_establishment_mode;
	int32 m_remote_establishment_identifier;
	int32 m_channel_index;
	int32 m_channel_connection_identifier;
	c_network_channel_simulation_interface m_channel_interface;
	bool m_simulation_active;
	uint32 m_simulation_player_acknowledged_mask;
	int32 m_synchronous_received_action_number;
	int32 m_synchronous_acknowledged_update_number;
	bool m_synchronous_client_blocked;
	int32 m_synchronous_client_block_timestamp;
	int32 m_synchronous_catchup_attempt_count;
	void* m_synchronous_catchup_buffer;
	int32 m_synchronous_catchup_buffer_size;
	c_ring_stream m_synchronous_catchup_stream;
	int32 m_synchronous_catchup_buffer_offset;
	int32 m_synchronous_catchup_stream_items;
};
ASSERT_STRUCT_SIZE(c_simulation_view, 0xB4);

class c_game_results_replicator
{
public:
	void handle_view_establishment(bool simulation_established);
	void update(void);
	void start_sending_updates(void);
	void stop_sending_updates(void);
	void start_receiving_updates(void);
	void stop_receiving_updates(void);
	void send_game_results_update(void);

	bool has_fatal_error(
		void) const
	{
		return m_fatal_error;
	}

private:
	class c_simulation_view* m_view;
	bool m_fatal_error;
	bool m_sending_updates;
	bool m_receiving_updates;
	int32 m_update_number;
	s_game_results_incremental m_game_results_incremental;
	int32 m_update_timestamp;
};
ASSERT_STRUCT_SIZE(c_game_results_replicator, 19420);

class c_simulation_distributed_view
{
public:
	int16 identifier;
	c_replication_scheduler m_replication_scheduler;
	c_replication_entity_manager_view m_entity_view;
	c_replication_event_manager_view m_event_view;
	c_replication_control_view m_control_view;
	c_simulation_view_telemetry_provider m_telemetry_provider;
	c_game_results_replicator m_game_results_replicator;
};
ASSERT_STRUCT_SIZE(c_simulation_distributed_view, 0xAC3C);

/* globals */

extern const char* g_simulation_view_reason_strings[k_simulation_view_reason_count];
