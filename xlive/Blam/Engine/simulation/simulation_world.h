#pragma once
#include "simulation_actors.h"
#include "simulation_entity_database.h"
#include "simulation_event_handler.h"
#include "simulation_players.h"
#include "simulation_queue.h"

#include "game/game_time.h"
#include "networking/replication/replication_event_manager.h"

/* constants */

enum
{
	k_simulation_world_maximum_synchronous_updates = 128,
	k_simulation_world_maximum_views = k_maximum_players,
};

/* enums */

enum e_simulation_queue_type
{
	_simulation_queue_bookkeeping = 0,
	_simulation_queue,

	k_simulation_queue_count
};

enum e_simulation_world_type
{
	_simulation_world_type_none = 0,
	_simulation_world_type_local,
	_simulation_world_type_synchronous_authority,
	_simulation_world_type_synchronous_client,
	_simulation_world_type_distributed_authority,
	_simulation_world_type_distributed_client,
	k_simulation_world_type_count,
};

enum e_simulation_world_state
{
	_simulation_world_state_none = 0,
	_simulation_world_state_dead,
	_simulation_world_state_disconnected,
	_simulation_world_state_joining,
	_simulation_world_state_active,
	_simulation_world_state_handoff,
	_simulation_world_state_leaving,
	k_simulation_world_state_count,
};

/* structures */

struct s_world_state_disconnected
{
	uint32 disconnected_timestamp;
};

struct s_world_state_data_joining
{
	uint32 join_start_timestamp;
	uint32 join_client_machine_mask;
};

struct s_world_state_data_active
{
	uint32 active_client_machine_mask;
};

union s_world_state_data
{
	s_world_state_disconnected disconnected;
	s_world_state_data_joining joining;
	s_world_state_data_active active;
};
ASSERT_STRUCT_SIZE(s_world_state_data, 0x8);

struct s_simulation_world_view_iterator
{
	uint32 view_type_mask;
	int32 next_world_view_index;
};

/* classes */

class c_simulation_distributed_world
{
public:
	c_replication_entity_manager m_entity_manager;
	c_replication_event_manager m_event_manager;
	c_simulation_entity_database m_entity_database;
	c_simulation_event_handler m_event_handler;	
};
ASSERT_STRUCT_SIZE(c_simulation_distributed_world, 45260);

class c_simulation_world
{
	enum e_join_progress
	{
		_join_progress_waiting = 0,
		_join_progress_ready,
		_join_progress_complete,
		_join_progress_failed,
		k_join_progress_count,
	};

public:
	void initialize_world(c_simulation_type_collection* type_collection, class c_simulation_watcher* watcher, c_simulation_distributed_world* distributed_world);
	void reset_world(void);
	void destroy_world(void);
	void update(void);

	void process_input(uint32 user_action_mask, struct player_action const* user_actions);

	void build_player_actions(struct simulation_update* update);

	void build_update(struct simulation_update* update);
	static void destroy_update(struct simulation_update* update);
	void process_pending_updates(void);
	void distribute_update(const struct simulation_update* update);

	void advance_update(const struct simulation_update* update);

	void go_out_of_sync(void);

	void attach_to_map(void);
	void detach_from_map(void);
	void time_start(int32 next_update_number);
	void time_stop(void);
	int32 time_get_available(bool* match_remote_time);
	void time_set_immediate_update(bool time_immediate_update);
	void get_machine_identifier(struct s_machine_identifier* identifier) const;
	void set_machine_identifier(struct s_machine_identifier const* identifier);
	int32 get_machine_index(void) const;
	void set_machine_index(int32 machine_index);
	
	int32 get_view_count(void) const;
	void remove_all_views(void);
	void iterator_begin(struct s_simulation_world_view_iterator* iterator, uint32 view_type_mask);
	bool iterator_next(struct s_simulation_world_view_iterator* iterator, class c_simulation_view** view) const;
	class c_simulation_view* get_authority_view(void);
	class c_simulation_view* get_client_view_by_machine_index(int32 remote_machine_index);

	class c_simulation_view* get_view_by_channel(int32 network_channel_index);

	int32 get_machine_index_by_identifier(struct s_machine_identifier const* remote_machine_identifier) const;
	void disconnect(void);

	bool claim_authority_gameworld(void);
	void handle_view_establishment(const class c_simulation_view* view, bool established);
	void handle_view_activation(const class c_simulation_view* view, bool active);
	void change_state_internal(e_simulation_world_state new_state);
	void change_state_joining(uint32 joining_client_machine_mask);
	void change_state_active(void);
	void change_state_disconnected(void);
	void change_state_dead(void);
	void change_state_handoff(void);
	void change_state_leaving(void);

	void create_player(datum player_index);
	void delete_player(datum player_index);

	bool player_is_in_game(int32 player_index, struct s_player_identifier const* player_identifier) const;

	int32 synchronous_authority_get_maximum_updates(void);

	void synchronous_authority_dispatch_update(struct simulation_update const* update);

	bool handle_synchronous_update(const struct simulation_update* update);

	void update_authority_join_initiate(void);
	
	void update_authority_join_progress(void);

	void update_authority_active(void);

	void update_authority_handoff(void);

	void update_client_join_initiate(void);

	void update_client_join_progress(void);

	void update_client_failure(void);

	void update_client_disconnection(void);

	void gamestate_flush(void);

	void simulation_queue_allocate(e_event_queue_type type, int32 encoded_size, s_simulation_queue_element** out_allocated_elem);
	void simulation_queue_free(s_simulation_queue_element* element);
	void simulation_queue_enqueue(s_simulation_queue_element* element);

	void queues_initialize(void);
	void apply_simulation_queue(const c_simulation_queue* queue);
	bool simulation_queues_empty(void) const;

	c_simulation_queue* queue_get(e_simulation_queue_type type) const;

	c_simulation_distributed_world* get_distributed_world(void) const { return m_distributed_world; }
	
	void delete_all_players(void);

	void delete_all_actors(void);

	void update_queue_reset(void);

	c_simulation_player* find_player_by_machine(s_machine_identifier const* machine_identifier, int32 user_index);

	uint32 get_acknowledged_player_mask(void) const;

	void send_player_acknowledgements_not_during_simulation_reset_in_progress(bool a1);

	bool synchronous_catchup_in_progress(void) const;

	void update_queue_retrieve_update(struct simulation_update* update);
	
	static const char* get_state_string(int32 world_state);

	void send_player_acknowledgements(bool force_acknowledgement);

	bool exists(
		void) const
	{
		return m_world_type != _simulation_world_type_none;
	}

	bool is_local(
		void) const
	{
		ASSERT(exists());

		bool is_local = m_world_type == _simulation_world_type_local;

		ASSERT(!is_local || m_view_count==0);

		return is_local;
	}

	bool is_authority(
		void) const
	{
		ASSERT(exists());
		return m_world_type != _simulation_world_type_distributed_client && m_world_type != _simulation_world_type_synchronous_client;
	}
	
	bool is_playback(
		void) const
	{
		ASSERT(exists());
		return false;
	}

	bool is_connected(
		void) const
	{
		ASSERT(exists());

		return IN_RANGE(m_world_state, _simulation_world_state_active, _simulation_world_state_leaving);
	}

	bool is_joining(
		void) const
	{
		ASSERT(exists());

		return m_world_state == _simulation_world_state_joining;
	}

	bool is_active(
		void) const
	{
		ASSERT(exists());
		return m_world_state == _simulation_world_state_active;
	}

	bool is_dead(
		void) const
	{
		ASSERT(exists());
		return m_world_state == _simulation_world_state_dead;
	}

	bool runs_simulation(
		void) const
	{
		ASSERT(exists());
		return m_world_type != _simulation_world_type_synchronous_client && !is_playback();
	}

	bool is_out_of_sync(
		void) const
	{
		ASSERT(exists());
		return !is_authority() && m_out_of_sync;
	}

	e_simulation_world_type get_world_type(
		void) const
	{
		ASSERT(exists());
		return m_world_type;
	}

	bool is_synchronous(
		void) const
	{
		return m_world_type == _simulation_world_type_synchronous_authority;
	}

	bool is_distributed(
		void) const
	{
		ASSERT(exists());
		return
			m_world_type == _simulation_world_type_distributed_authority ||
			m_world_type == _simulation_world_type_distributed_client;
	}

	int32 get_next_update_number(
		void) const
	{
		ASSERT(exists());
		return m_next_update_number;
	}
	
	int32 get_time(
		void) const
	{
		ASSERT(exists());
		return (int32)game_time_get();
	}

	e_simulation_world_state get_state(
		void) const
	{
		ASSERT(exists());
		return m_world_state;
	}
	
	bool attached_to_map(
		void) const
	{
		ASSERT(exists());
		return m_attached_to_map;
	}

	bool time_running(
		void) const
	{
		ASSERT(exists());
		return m_time_running;
	}

	c_replication_entity_manager* get_entity_manager(
		void)
	{
		ASSERT(exists());
		ASSERT(is_distributed());
		ASSERT(m_distributed_world);

		return &m_distributed_world->m_entity_manager;
	}

	c_replication_event_manager* get_event_manager(
		void)
	{
		ASSERT(exists());
		ASSERT(is_distributed());
		ASSERT(m_distributed_world);

		return &m_distributed_world->m_event_manager;
	}

	c_simulation_entity_database* get_entity_database(
		void)
	{
		ASSERT(exists());
		ASSERT(is_distributed());
		ASSERT(m_distributed_world);

		return &m_distributed_world->m_entity_database;
	}

	c_simulation_event_handler* get_event_handler(
		void)
	{
		ASSERT(exists());
		ASSERT(is_distributed());
		ASSERT(m_distributed_world);

		return &m_distributed_world->m_event_handler;
	}

	void queues_update_statistics(void) const
	{
		for (int32 i = 0; i < k_simulation_queue_count; i++)
		{
			queue_get((e_simulation_queue_type)i)->build_statistics();
		}
	}

	bool queue_describe(e_simulation_queue_type type, const s_simulation_queue_stats** out_stats) const
	{
		return queue_get(type)->get_statistics(out_stats);
	}
	
private:
	class c_simulation_watcher* m_watcher;
	c_simulation_distributed_world* m_distributed_world;
	e_simulation_world_type m_world_type;
	bool m_local_machine_identifier_valid;
	s_machine_identifier m_local_machine_identifier;
	int32 m_local_machine_index;
	e_simulation_world_state m_world_state;
	s_world_state_data m_world_state_data;
	bool m_time_running;
	bool m_time_immediate_update;
	uint8 gap_26[2];
	int32 m_next_update_number;
	bool m_out_of_sync;
	bool m_out_of_sync_determinism_failure;
	bool m_gamestate_flushed;
	bool m_attached_to_map;
	int32 m_unsuccessful_join_attempts;
	uint32 m_last_active_timestamp;
	int32 m_next_view_establishment_identifier;
	int32 m_joining_total_wait_msec;
	int32 m_view_count;
	c_simulation_view* m_views[k_simulation_world_maximum_views];
	int32 m_player_count; // guessed name for potential use, field is completely unused
	c_simulation_player m_players[k_maximum_players];
	c_simulation_actor m_actors[k_network_maximum_players_per_session];
	bool m_synchronous_gamestate_read_in_progress;
	int32 m_synchronous_gamestate_write_progress;
	void* m_synchronous_gamestate_write_buffer;
	int32 m_synchronous_catchup_initiation_failure_timestamp;
	int32 m_update_queue_next_update_number_to_dequeue;
	int32 m_update_queue_latest_update_number_received;
	int32 m_update_queue_length;
	struct s_simulation_queued_update* m_update_queue_head;
	struct s_simulation_queued_update* m_update_queue_tail;
	int32 _pad_12AC;

	e_join_progress update_joining_view(class c_simulation_view* view);
	void update_establishing_view(class c_simulation_view* view);
	void verify_player_activation(void) const;
	void update_player_activation(void);
	bool synchronous_gamestate_write_in_progress(void) const;
	void synchronous_gamestate_clear(void);
	void update_queue_start(int32 next_update_number);
	void update_queue_stop(void);
	bool update_queue_handle_server_update(const struct simulation_update* update);
	int32 update_queue_get_available_updates(void) const;
	int32 update_queue_get_next_expected_update_number(void) const;
	void distributed_authority_dispatch_player_actions(uint32 player_valid_mask, const struct player_action* player_actions);
	void distributed_authority_dispatch_actor_control(uint32 actor_valid_mask, const struct unit_control_data* actor_control);
	void attach_simulation_queues_to_update(struct simulation_update* update);
};
ASSERT_STRUCT_SIZE(c_simulation_world, 0x12B0);

/* prototypes */

void simulation_world_apply_patches(void);
