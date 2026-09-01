#include "stdafx.h"
#include "simulation_world.h"

#include "simulation.h"
#include "simulation_encoding.h"
#include "simulation_entity_database.h"
#include "simulation_gamestate_entities.h"
#include "simulation_queue_events.h"
#include "simulation_queue_entities.h"
#include "simulation_queue_global_events.h"
#include "simulation_view.h"
#include "simulation_watcher.h"

#include "cache/pc_texture_cache.h"
#include "game/game.h"
#include "game/players.h"
#include "math/random_math.h"
#include "memory/bitstream.h"
#include "networking/network_configuration.h"
#include "networking/network_event.h"
#include "networking/network_memory.h"
#include "networking/network_time.h"
#include "saved_games/game_state_procs.h"

/* typedefs */

typedef void(__thiscall* t_c_simulation_world__initialize_world)(c_simulation_world*, c_simulation_type_collection*, c_simulation_watcher*, c_simulation_distributed_world*);
typedef void(__thiscall* t_c_simulation_world__destroy_world)(c_simulation_world*);

/* prototypes */

/* globals */

static t_c_simulation_world__initialize_world p_c_simulation_world__initialize_world;
static t_c_simulation_world__destroy_world p_c_simulation_world__destroy_world;

static c_simulation_queue g_simulation_queues[k_simulation_queue_count];

/* public code */

CLASS_HOOK_DECLARE_LABEL(c_simulation_world__send_player_acknowledgements_not_during_simulation_reset_in_progress, c_simulation_world::send_player_acknowledgements_not_during_simulation_reset_in_progress);
static void __declspec(naked) jmp_send_player_acknowledgements_not_during_simulation_reset_in_progress()
{
	CLASS_HOOK_JMP(c_simulation_world__send_player_acknowledgements_not_during_simulation_reset_in_progress, c_simulation_world::send_player_acknowledgements_not_during_simulation_reset_in_progress);
}

CLASS_HOOK_DECLARE_LABEL(c_simulation_world__initialize_world, c_simulation_world::initialize_world);
static void __declspec(naked) jmp_initialize_world(void)
{
	CLASS_HOOK_JMP(c_simulation_world__initialize_world, c_simulation_world::initialize_world);
}

CLASS_HOOK_DECLARE_LABEL(c_simulation_world__destroy_world, c_simulation_world::destroy_world);
static void __declspec(naked) jmp_destroy_world(void)
{
	CLASS_HOOK_JMP(c_simulation_world__destroy_world, c_simulation_world::destroy_world);
}

CLASS_HOOK_DECLARE_LABEL(c_simulation_world__update, c_simulation_world::update);
static void __declspec(naked) jmp_update(void)
{
	CLASS_HOOK_JMP(c_simulation_world__update, c_simulation_world::update);
}

void simulation_world_apply_patches(
	void)
{
	DETOUR_ATTACH(p_c_simulation_world__initialize_world, Memory::GetAddress<t_c_simulation_world__initialize_world>(0x1DDB4E, 0x1C500E), jmp_initialize_world);
	DETOUR_ATTACH(p_c_simulation_world__destroy_world, Memory::GetAddress<t_c_simulation_world__destroy_world>(0x1DE0A9, 0x1C5569), jmp_destroy_world);
	
	PatchCall(Memory::GetAddress(0x1DD9FB, 0x1C4EBB), jmp_send_player_acknowledgements_not_during_simulation_reset_in_progress);
	PatchCall(Memory::GetAddress(0x1AE872, 0x0), jmp_update);
	return;
}

void c_simulation_world::initialize_world(
	c_simulation_type_collection* type_collection,
	c_simulation_watcher* watcher,
	c_simulation_distributed_world* distributed_world)
{
	ASSERT(type_collection);
	ASSERT(watcher);
	ASSERT(m_world_type == _simulation_world_type_none);

	m_watcher = watcher;

	switch (game_simulation_get())
	{
	case _game_simulation_local:
		m_world_type = _simulation_world_type_local;
		break;
	case _game_simulation_synchronous_client:
		m_world_type = _simulation_world_type_synchronous_client;
		break;
	case _game_simulation_synchronous_server:
		m_world_type = _simulation_world_type_synchronous_authority;
		break;
	case _game_simulation_distributed_client:
		m_world_type = _simulation_world_type_distributed_client;
		break;
	case _game_simulation_distributed_server:
		m_world_type = _simulation_world_type_distributed_authority;
		break;
	default:
		unreachable();
		break;
	}

	ASSERT(m_world_type > _simulation_world_type_none && m_world_type < k_simulation_world_type_count);

	m_world_state = _simulation_world_state_none;
	m_time_running = false;
	m_time_immediate_update = false;
	m_attached_to_map = false;
	m_out_of_sync = false;
	m_next_update_number = 0;
	m_gamestate_flushed = false;
	m_unsuccessful_join_attempts = 0;
	m_last_active_timestamp = network_time_get();
	m_next_view_establishment_identifier = 0;
	m_joining_total_wait_msec = 0;

	if (is_distributed())
	{
		ASSERT(distributed_world);
		m_distributed_world = distributed_world;
		m_distributed_world->m_entity_manager.initialize();
		m_distributed_world->m_event_manager.initialize(&m_distributed_world->m_entity_manager);
		m_distributed_world->m_entity_database.initialize(this, &m_distributed_world->m_entity_manager, type_collection);
		m_distributed_world->m_event_handler.initialize(this, &m_distributed_world->m_event_manager, type_collection, &m_distributed_world->m_entity_database);
	}
	else
	{
		m_synchronous_gamestate_read_in_progress = false;
		m_synchronous_gamestate_write_progress = NONE;
		m_synchronous_gamestate_write_buffer = NULL;
		m_synchronous_catchup_initiation_failure_timestamp = NONE;
	}

	if (!is_playback())
	{
		queues_initialize();
	}

	if (!runs_simulation())
	{
		m_update_queue_length = NULL;
		m_update_queue_head = NULL;
		m_update_queue_tail = NULL;
		update_queue_reset();
	}

	m_view_count = 0;
	csmemset(m_views, 0, sizeof(m_views));
	m_local_machine_identifier_valid = false;
	m_local_machine_index = NONE;

	change_state_disconnected();

	return;
}

void c_simulation_world::reset_world(
	void)
{
	ASSERT(!is_authority());

	m_time_immediate_update = false;
	m_out_of_sync = false;
	m_gamestate_flushed = false;

	if (is_distributed())
	{
		ASSERT(m_distributed_world);

		m_distributed_world->m_entity_manager.reset();
		m_distributed_world->m_event_manager.reset();
		m_distributed_world->m_entity_database.reset();
		m_distributed_world->m_event_handler.reset();
		simulation_gamestate_entities_notify_simulation_world_reset();
		delete_all_actors();
	}

	if (runs_simulation())
	{
		// during reset, discard just simulation updates
		// not bookkeeping updates
		queue_get(_simulation_queue)->clear();
	}

	if (time_running())
	{
		time_stop();
	}

	if (!runs_simulation())
	{
		update_queue_reset();
	}

	return;
}

void c_simulation_world::destroy_world(
	void)
{
	ASSERT(m_world_type != _simulation_world_type_none);

	disconnect();

	if (!is_distributed())
	{
		ASSERT(m_synchronous_gamestate_write_progress == NONE);
		ASSERT(m_synchronous_gamestate_write_buffer == NULL);
	}

	// Make sure we're not running when we trying to destroy the world
	ASSERT(!time_running());


	ASSERT(m_view_count >= 0 && m_view_count <= k_simulation_world_maximum_views);

	while (m_view_count > 0)
	{
		simulation_remove_view_from_world(m_views[m_view_count - 1]);
	}

	delete_all_players();
	delete_all_actors();

	if (is_distributed())
	{
		ASSERT(m_distributed_world);

		m_distributed_world->m_event_handler.destroy();
		m_distributed_world->m_entity_database.destroy();
		m_distributed_world->m_entity_manager.destroy();
		m_distributed_world->m_event_manager.destroy();
		simulation_gamestate_entities_notify_simulation_world_reset();
	}

	if (runs_simulation())
	{
		queue_get(_simulation_queue_bookkeeping)->dispose();
		queue_get(_simulation_queue)->dispose();
	}
	else
	{
		update_queue_reset();
	}

	m_watcher = NULL;
	m_distributed_world = NULL;
	m_world_type = _simulation_world_type_none;

	return;
}

void c_simulation_world::update(
	void)
{
	ASSERT(exists());

	if (is_authority())
	{
		if (!is_connected() && !is_joining() && !is_dead())
		{
			update_authority_join_initiate();
		}
		if (m_world_state==_simulation_world_state_joining)
		{
			update_authority_join_progress();
		}
		if (m_world_state==_simulation_world_state_active)
		{
			update_authority_active();
		}
		if (m_world_state==_simulation_world_state_handoff)
		{
			update_authority_handoff();
		}

		update_player_activation();
	}
	else
	{
		if (!is_connected() && !is_joining() && !is_dead())
		{
			update_client_join_initiate();
		}
		if (is_joining())
		{
			update_client_join_progress();
		}
		if (!is_active() && !is_dead())
		{
			update_client_failure();
		}
		if (is_connected() || is_joining())
		{
			update_client_disconnection();
		}

		send_player_acknowledgements(false);
	}

	if (m_world_type == _simulation_world_type_synchronous_client && m_time_immediate_update)
	{
		ASSERT(m_world_state==_simulation_world_state_joining);
	}
	else
	{
		ASSERT(time_running()==is_active());
	}

	s_simulation_world_view_iterator iterator;
	iterator_begin(&iterator, (uint32)NONE);

	c_simulation_view* view;
	while (iterator_next(&iterator, &view))
	{
		ASSERT(view);
		view->update();
	}

	return;
}

void c_simulation_world::process_input(
	uint32 user_action_mask,
	player_action const* user_actions)
{
	ASSERT(exists());

	if (runs_simulation())
	{
		for (int32 user_index = 0; user_index<k_number_of_users; ++user_index)
		{
			if (TEST_BIT(user_action_mask, user_index))
			{
				c_simulation_player* local_user = find_player_by_machine(&m_local_machine_identifier, user_index);
				if (local_user)
				{
					local_user->handle_local_input(&user_actions[user_index]);
				}
			}
		}

	}
	else if (m_world_type==_simulation_world_type_synchronous_client)
	{
		c_simulation_view* view = get_authority_view();
		
		ASSERT(view);

		view->dispatch_synchronous_actions(user_action_mask, user_actions);
	}
	else
	{
		if (is_playback())
		{
			// do nothing?
		}
	}

	return;
}

void c_simulation_world::build_player_actions(
	struct simulation_update* update)
{
	INVOKE_TYPE(0x1DBE3F, 0x0, void(__thiscall*)(c_simulation_world*, struct simulation_update*), this, update);
	return;
}

void c_simulation_world::build_update(
	struct simulation_update* update)
{
	if (runs_simulation())
	{
		update->update_number = get_next_update_number();
		update->verify_game_time = get_time();

		random_seed_allow_use();
		update->verify_random_seed = get_random_seed();
		random_seed_disallow_use();

		simulation_build_machine_update(&update->machine_update_valid, &update->machine_update);
		simulation_build_player_updates(&update->player_update_count, NUMBEROF(update->player_updates), update->player_updates);
		update->simulation_in_progress = simulation_in_progress();

		if (update->simulation_in_progress)
		{
			build_player_actions(update);

			if (is_distributed())
			{
				// Do nothing?
			}

			if (is_authority() && m_gamestate_flushed)
			{
				update->flush_gamestate = true;
				m_gamestate_flushed = false;
			}
		}

		update->bookkeeping_simulation_queue.initialize();
		update->game_simulation_queue.initialize();

		attach_simulation_queues_to_update(update);

		uint8 data[0x20000];
		c_bitstream temporary_stream(data, sizeof(data));

		temporary_stream.begin_writing(k_bitstream_default_alignment);
		simulation_update_encode(&temporary_stream, update);
		temporary_stream.finish_writing(NULL);

		update->bookkeeping_simulation_queue.dispose();
		update->game_simulation_queue.dispose();

		temporary_stream.begin_reading();
		bool decode_success = simulation_update_decode(&temporary_stream, update);

		ASSERT(!temporary_stream.error_occurred());
		temporary_stream.finish_reading();

		ASSERT(decode_success);
	}
	else
	{
		update_queue_retrieve_update(update);
	}
	return;
}

void c_simulation_world::destroy_update(
	struct simulation_update* update)
{
	ASSERT(update);

	update->bookkeeping_simulation_queue.dispose();
	update->game_simulation_queue.dispose();

	return;
}

void c_simulation_world::process_pending_updates(
	void)
{
	ASSERT(exists());

	if (is_distributed() && is_authority())
	{
		ASSERT(m_distributed_world);

		m_distributed_world->m_entity_database.process_pending_updates();
	}

	return;
}

void c_simulation_world::distribute_update(
	const struct simulation_update* update)
{
	ASSERT(update);
	ASSERT(exists());
	ASSERT(is_authority());

	if (is_synchronous())
	{
		synchronous_authority_dispatch_update(update);
	}
	else if (is_distributed())
	{
		distributed_authority_dispatch_player_actions(update->player_action_mask, update->player_actions);
		distributed_authority_dispatch_actor_control(update->unit_control_mask, update->unit_control);
	}

	return;
}

void c_simulation_world::advance_update(
	const struct simulation_update* update)
{
	ASSERT(update);
	m_next_update_number = update->update_number+1;
	
	return;
}

void c_simulation_world::go_out_of_sync(
	void)
{
	ASSERT(exists());
	ASSERT(!runs_simulation());
	ASSERT(m_time_running);
	
	// TODO: add debug hs global check here 
	//if ()
	{
		m_out_of_sync = true;
	}

	return;
}

void c_simulation_world::attach_to_map(
	void)
{
	ASSERT(!m_attached_to_map);
	ASSERT(m_view_count==0);

	m_attached_to_map = true;

	return;
}

void c_simulation_world::detach_from_map(
	void)
{
	ASSERT(m_attached_to_map);

	remove_all_views();
	m_attached_to_map = false;

	return;
}

void c_simulation_world::time_start(
	int32 next_update_number)
{
	ASSERT(exists());
	ASSERT(!m_time_running);

	ASSERT(m_local_machine_index != NONE);
	ASSERT(next_update_number>=0);

	m_next_update_number = next_update_number;
	
	if (!runs_simulation())
	{
		update_queue_start(next_update_number);
	}
	m_time_running = true;

	return;
}

void c_simulation_world::time_stop(
	void)
{
	ASSERT(exists());
	ASSERT(m_time_running);
	
	m_time_running = false;

	if (!runs_simulation())
	{
		update_queue_stop();
	}

	return;
}

int32 c_simulation_world::time_get_available(
	bool* match_remote_time)
{
	int32 result = 0;

	ASSERT(exists());
	ASSERT(match_remote_time);

	*match_remote_time = false;
	
	if (m_time_running)
	{
		result = LONG_MAX;
		switch (m_world_type)
		{
		case _simulation_world_type_local:
		case _simulation_world_type_distributed_authority:
		case _simulation_world_type_distributed_client:
			break;
		case _simulation_world_type_synchronous_authority:
			result = synchronous_authority_get_maximum_updates();
			break;
		case _simulation_world_type_synchronous_client:
			result = update_queue_get_available_updates();
			*match_remote_time = true;
			break;
		default:
			unreachable();
		}
	}

	return result;
}

void c_simulation_world::time_set_immediate_update(
	bool time_immediate_update)
{
	ASSERT(exists());
	m_time_immediate_update = time_immediate_update;

	if (m_time_immediate_update)
	{
		ASSERT(m_world_type==_simulation_world_type_synchronous_client);

		bool match_remote_time;
		while (time_get_available(&match_remote_time))
		{
			game_tick();
		}
	}

	return;
}

void c_simulation_world::get_machine_identifier(
	s_machine_identifier* identifier) const
{
	ASSERT(identifier);
	ASSERT(m_local_machine_identifier_valid);
	
	*identifier = m_local_machine_identifier;

	return;
}

void c_simulation_world::set_machine_identifier(
	s_machine_identifier const* identifier)
{
	ASSERT(identifier);
	
	m_local_machine_identifier = *identifier;
	m_local_machine_identifier_valid = 1;

	return;
}

int32 c_simulation_world::get_machine_index(
	void) const
{
	return m_local_machine_index;
}

void c_simulation_world::set_machine_index(
	int32 machine_index)
{
	ASSERT(m_local_machine_identifier_valid);
	ASSERT(machine_index>=0 && machine_index<k_network_maximum_machines_per_session);
	
	m_local_machine_index = machine_index;

	return;
}

int32 c_simulation_world::get_view_count(
	void) const
{
	return m_view_count;
}

void c_simulation_world::remove_all_views(
	void)
{
	ASSERT(exists());

	s_simulation_world_view_iterator iterator;
	iterator_begin(&iterator, (uint32)NONE);
	
	c_simulation_view* view;
	while(iterator_next(&iterator, &view))
	{
		ASSERT(view);
		ASSERT(view->exists());
		ASSERT(view->get_world()==this);

		simulation_remove_view_from_world(view);
	}

	ASSERT(m_view_count==0);


	for (int32 view_index= 0; view_index<NUMBEROF(m_views); ++view_index)
	{
		ASSERT(m_views[view_index]==NULL);
	}

	return;
}

void c_simulation_world::iterator_begin(
	s_simulation_world_view_iterator* iterator,
	uint32 view_type_mask)
{
	ASSERT(iterator);
	ASSERT(view_type_mask == NONE || (view_type_mask != 0 && VALID_BITS(view_type_mask, k_simulation_view_type_count)));

	iterator->view_type_mask = view_type_mask;
	iterator->next_world_view_index = 0;
	return;
}

bool c_simulation_world::iterator_next(
	s_simulation_world_view_iterator* iterator,
	c_simulation_view** view) const
{
	bool result = false;

	ASSERT(iterator);
	ASSERT(view);

	while (iterator->next_world_view_index<NUMBEROF(m_views))
	{
		c_simulation_view* current_view = m_views[iterator->next_world_view_index++];
		if (current_view && TEST_BIT(iterator->view_type_mask, current_view->view_type()))
		{
			*view = current_view;
			result = true;
			break;
		}
	}

	return result;
}

c_simulation_view* c_simulation_world::get_authority_view(
	void)
{
	c_simulation_view* view = NULL;

	ASSERT(exists());

	s_simulation_world_view_iterator iterator;
	iterator_begin(&iterator, FLAG(_simulation_world_type_local) | FLAG(_simulation_world_type_synchronous_client));

	if (iterator_next(&iterator, &view))
	{
		ASSERT(view != NULL);
	}

	return view;
}

c_simulation_view* c_simulation_world::get_client_view_by_machine_index(
	int32 remote_machine_index)
{
	c_simulation_view* view = NULL;

	ASSERT(exists());

	s_simulation_world_view_iterator iterator;
	iterator_begin(&iterator, FLAG(_simulation_world_type_synchronous_authority) | FLAG(_simulation_world_type_distributed_authority));

	c_simulation_view* test_view;
	while (iterator_next(&iterator, &test_view))
	{
		view = test_view;

		ASSERT(test_view != NULL);

		if (view->get_machine_index() == remote_machine_index)
		{
			break;
		}
	}

	return view;
}

c_simulation_view* c_simulation_world::get_view_by_channel(
	int32 network_channel_index)
{
	c_simulation_view* test_view;
	s_simulation_world_view_iterator iterator;

	c_simulation_view* view = NULL;

	ASSERT(exists());

	iterator_begin(&iterator, (uint32)NONE);

	while (iterator_next(&iterator, &test_view))
	{
		ASSERT(test_view != NULL);

		if (test_view->get_channel_index() == network_channel_index)
		{
			view = test_view;
			break;
		}
	}

	return view;
}

int32 c_simulation_world::get_machine_index_by_identifier(
	struct s_machine_identifier const* remote_machine_identifier) const
{
	ASSERT(exists());
	ASSERT(m_watcher);

	return m_watcher->get_machine_index_by_identifier(remote_machine_identifier);
}

void c_simulation_world::disconnect(
	void)
{
	if (is_connected() || is_joining() || m_view_count > 0)
	{
		event(
			_event_message,
			"simulation:world: disconnected (state %s, %d views)",
			get_state_string(m_world_state),
			m_view_count
		);
	}

	if (is_connected() || is_joining())
	{
		change_state_disconnected();
	}

	return;
}

bool c_simulation_world::claim_authority_gameworld(
	void)
{
	return INVOKE_TYPE(0x1DE3D0, 0x1C5890, bool(__thiscall*)(c_simulation_world*), this);
}

void c_simulation_world::handle_view_establishment(
	const c_simulation_view* view,
	bool established)
{
	if (established && view == get_authority_view())
	{
		ASSERT(!is_authority());
		simulation_reset();
		send_player_acknowledgements(true);
	}

	return;
}

void c_simulation_world::handle_view_activation(
	const c_simulation_view* view,
	bool active)
{
	if (active && view == get_authority_view() && is_joining())
	{
		update_client_join_progress();
	}

	return;
}

void c_simulation_world::change_state_internal(
	e_simulation_world_state new_state)
{
	ASSERT(exists());
	ASSERT(new_state > _simulation_world_type_none && new_state < k_simulation_world_state_count);
	ASSERT(new_state != m_world_state);

	if (!is_local())
	{
		event(
			_event_message,
			"simulation:world: state %s -> %s",
			get_state_string(m_world_state),
			get_state_string(new_state)
		);
	}

	if (new_state == _simulation_world_state_active)
	{
		ASSERT(time_running());
	}
	else
	{
		time_set_immediate_update(false);
		if (time_running())
		{
			time_stop();
		}
	}

	if (m_world_state == _simulation_world_state_joining)
	{
		if (is_authority())
		{
			m_joining_total_wait_msec += network_time_since(m_world_state_data.disconnected.disconnected_timestamp);
		}

		if (new_state != _simulation_world_state_active)
		{
			if (synchronous_catchup_in_progress())
			{
				synchronous_gamestate_clear();
			}

			++m_unsuccessful_join_attempts;
			event(
				_event_message,
				"simulation:world: recording an unsuccessful join (now %d failures)",
				m_unsuccessful_join_attempts
			);
		}
	}
	else if (m_world_state == _simulation_world_state_active)
	{
		m_last_active_timestamp = network_time_get();
	}

	ASSERT(!synchronous_gamestate_write_in_progress());

	m_world_state = new_state;

	return;
}

void c_simulation_world::change_state_joining(
	uint32 joining_client_machine_mask)
{
	ASSERT(exists());
	ASSERT((!is_connected() && !is_dead()) || is_active());

	change_state_internal(_simulation_world_state_joining);
	m_world_state_data.disconnected.disconnected_timestamp = network_time_get();
	m_world_state_data.joining.join_client_machine_mask = joining_client_machine_mask;

	return;
}

void c_simulation_world::change_state_active(
	void)
{
	ASSERT(exists());
	ASSERT(is_joining());

	if (time_running())
	{
		ASSERT(m_world_type == _simulation_world_type_synchronous_client);
		ASSERT(m_time_immediate_update);

		time_set_immediate_update(false);
	}
	else
	{
		time_start(m_next_update_number);
	}

	change_state_internal(_simulation_world_state_active);
	m_world_state_data.disconnected.disconnected_timestamp = 0;

	return;
}

void c_simulation_world::change_state_disconnected(
	void)
{
	ASSERT(exists());
	ASSERT(m_world_state != _simulation_world_state_dead);

	if (m_world_state != _simulation_world_state_disconnected)
	{
		change_state_internal(_simulation_world_state_disconnected);
		m_world_state_data.disconnected.disconnected_timestamp = network_time_get();
	}

	return;
}

void c_simulation_world::change_state_dead(
	void)
{
	ASSERT(exists());

	if (!is_dead())
	{
		disconnect();
		change_state_internal(_simulation_world_state_dead);
	}

	return;
}

void c_simulation_world::change_state_handoff(
	void)
{
	ASSERT(exists());
	ASSERT(is_authority());

	if (is_active())
	{
		change_state_internal(_simulation_world_state_handoff);
	}
	else
	{
		change_state_leaving();
	}

	return;
}

void c_simulation_world::change_state_leaving(
	void)
{
	ASSERT(exists());

	if ((is_connected() || is_joining()) &&
		m_world_state != _simulation_world_state_leaving)
	{
		change_state_internal(_simulation_world_state_leaving);
	}

	return;
}

void c_simulation_world::create_player(
	datum player_index)
{
	event(_event_verbose, "simulation:players: create player 0x%08X", player_index);
	typedef void(__thiscall* create_player_t)(c_simulation_world*, datum);
	INVOKE_TYPE(0x1DC05C, 0x1C3511, create_player_t, this, player_index);
	return;
}

void c_simulation_world::delete_player(
	datum player_index)
{
	typedef void(__thiscall* delete_player_t)(c_simulation_world*, datum);
	INVOKE_TYPE(0x1DC124, 0x1C35D8, delete_player_t, this, player_index);
	return;
}

bool c_simulation_world::player_is_in_game(
	int32 player_index,
	struct s_player_identifier const* player_identifier) const
{
	int32 player_absolute_index = DATUM_INDEX_TO_ABSOLUTE_INDEX(player_index);
	bool player_in_game = false;

	ASSERT(exists());

	if (VALID_INDEX(player_absolute_index, NUMBEROF(m_players)))
	{
		c_simulation_player const* player = &m_players[player_absolute_index];
		
		if (player->exists())
		{
			s_player_identifier world_player_identifier;

			player->get_identifier(&world_player_identifier);

			if (!csmemcmp(player_identifier, &world_player_identifier, sizeof(*player_identifier)))
			{
				if (m_watcher->get_player_is_in_game(player_absolute_index, player_identifier))
				{
					player_in_game = true;
				}
			}
		}
	}

	return player_in_game;
}

int32 c_simulation_world::synchronous_authority_get_maximum_updates(
	void)
{
	c_simulation_view* lowest_view = NULL;
	c_simulation_view* view_comparison = NULL;
	int32 lowest_update_num = INT_MAX;
	int32 result = INT_MAX;

	s_simulation_world_view_iterator iterator;
	iterator_begin(&iterator, FLAG(_simulation_world_type_synchronous_authority));

	// Iterate through every view and find the view that's fallen the most behind in updates
	c_simulation_view* view;
	while (iterator_next(&iterator, &view))
	{
		ASSERT(view);
		if (view->active())
		{
			const int32 update_num = view->synchronous_client_get_acknowledged_update_number();
			if (update_num < lowest_update_num)
			{
				lowest_view = view;
				lowest_update_num = update_num;
			}
		}
	}

	if (lowest_view)
	{
		const int32 updates_to_send = get_next_update_number() - 1 - lowest_update_num;
		const int32 new_maximum = k_simulation_world_maximum_synchronous_updates - updates_to_send;
		if (updates_to_send < 0)
		{
			DISPLAY_ASSERT("simulation world believes its clients are all in the future!");
		}

		if (new_maximum > 0)
		{
			result = new_maximum;
		}
		else
		{
			view_comparison = lowest_view;
		}
	}

	/* WIP Halo 3 Code
	if (!view_comparison)
	{
		iterator_begin(&iterator, FLAG(_simulation_world_type_synchronous_authority));
		while (iterator_next(&iterator, &view))
		{
			if (view->observer_channel_backlogged(38))
			{
				view_comparison = view;
				view_comparison->observer_channel_set_waiting_on_backlog(38);
				break;
			}
		}
	}
	*/

	result = view_comparison != NULL ? 0 : result;

	iterator_begin(&iterator, FLAG(_simulation_world_type_synchronous_authority));
	while (iterator_next(&iterator, &view))
	{
		const bool views_match = view_comparison == view;
		ASSERT(view);
		view->synchronous_client_block(views_match);
	}

	return result;
}

void c_simulation_world::synchronous_authority_dispatch_update(
	struct simulation_update const* update)
{
	ASSERT(exists());
	ASSERT(is_authority());

	s_simulation_world_view_iterator iterator;
	iterator_begin(&iterator, FLAG(_simulation_world_type_synchronous_authority));

	c_simulation_view* view;
	while (iterator_next(&iterator, &view))
	{
		ASSERT(view);
		view->dispatch_synchronous_update(update);
	}

	return;
}

bool c_simulation_world::handle_synchronous_update(
	const struct simulation_update* update)
{
	bool result = false;

	ASSERT(exists());

	ASSERT(m_world_type==_simulation_world_type_synchronous_client);
	ASSERT(m_time_running);

	ASSERT(update);

	const int32 next_expected_update_number = update_queue_get_next_expected_update_number();
	if (synchronous_gamestate_write_in_progress())
	{
		event(_event_error, "simulation:world: OUT OF SYNC: server update arrived while gamestate transfer was incomplete");
		go_out_of_sync();
	}
	else if (update->update_number < next_expected_update_number)
	{
		event(
			_event_warning,
			"simulation:world: synchronous-update discarded (expected #%ld, got old #%ld)",
			next_expected_update_number,
			update->update_number
		);
	}
	else if (update->update_number != next_expected_update_number)
	{
		event(
			_event_error,
			"simulation:world: OUT OF SYNC: missed a server update (expected #%ld, got #%ld)",
			next_expected_update_number,
			update->update_number
		);
		go_out_of_sync();
	}
	else if (is_active() && m_time_immediate_update)
	{
		event(
			_event_error,
			"simulation:world: OUT OF SYNC: server update arrived while world was unable to process it (state %d)",
			get_state()
		);
		go_out_of_sync();
	}
	else if (!update_queue_handle_server_update(update))
	{
		event(
			_event_error,
			"simulation:world: synchronous-update #%ld couldn't be inserted into update queue",
			update->update_number
		);
		simulation_fatal_error();
	}
	else
	{
		result = true;
		if (m_time_immediate_update)
		{
			bool time_available;
			event(
				_event_message,
				"simulation:world: processing immediate updates (%d at update #%d time #%d)",
				time_get_available(&time_available),
				get_next_update_number(),
				game_time_get()
			);

			while (game_in_progress() && !simulation_aborted() && m_out_of_sync && time_get_available(&time_available) <= 0)
			{
				game_tick();
			}
		}
	}

	return result;
}

void c_simulation_world::distributed_authority_dispatch_player_actions(
	uint32 player_valid_mask,
	const player_action* player_actions)
{
	INVOKE_TYPE(0x1DC5B3, 0x0, void(__thiscall*)(c_simulation_world*, uint32, const player_action*), this, player_valid_mask, player_actions);
	return;
}

void c_simulation_world::distributed_authority_dispatch_actor_control(
	uint32 actor_valid_mask,
	const unit_control_data* actor_control)
{
	INVOKE_TYPE(0x1DC761, 0x0, void(__thiscall*)(c_simulation_world*, uint32, const unit_control_data*), this, actor_valid_mask, actor_control);
	return;
}

void c_simulation_world::update_authority_join_initiate(
	void)
{
	ASSERT(exists());
	ASSERT(is_authority());
	ASSERT(!is_connected() && !is_joining() && !is_dead());

	if (is_local())
	{
		change_state_joining(0);
		change_state_active();
	}
	else
	{
		change_state_joining(m_watcher->get_machine_valid_mask());
	}

	return;
}

void c_simulation_world::update_authority_join_progress(
	void)
{
	int32 machine_index;
	uint32 join_blocking_machine_mask;

	uint32 machine_valid_mask = m_watcher->get_machine_valid_mask();
	uint32 join_client_machine_mask = 0;
	uint32 join_connected_machine_mask = 0;
	uint32 join_established_machine_mask = 0;
	uint32 join_waiting_machine_mask = 0;
	uint32 join_in_progress_machine_mask = 0;
	uint32 join_complete_machine_mask = 0;
	int32 join_wait_time_msec = network_time_since(m_world_state_data.disconnected.disconnected_timestamp);

	ASSERT(exists());
	ASSERT(is_authority());
	ASSERT(!is_local());
	ASSERT(m_world_state==_simulation_world_state_joining);

	for (machine_index = 0; machine_index < NUMBEROF(m_views); ++machine_index)
	{
		if (TEST_BIT(machine_valid_mask, machine_index) && machine_index!=get_machine_index())
		{
			c_simulation_view* view = get_client_view_by_machine_index(machine_index);
			SET_BIT(join_client_machine_mask, machine_index, true);

			if (view)
			{
				if (!view->is_dead(NULL))
				{
					SET_BIT(join_connected_machine_mask, machine_index, true);

					ASSERT(!view->active());
					
					if (view->established())
					{
						SET_BIT(join_established_machine_mask, machine_index, true);
						
						if (TEST_BIT(m_world_state_data.joining.join_client_machine_mask, machine_index))
						{
							if (update_joining_view(view))
							{
								SET_BIT(join_complete_machine_mask, machine_index, true);
							}
							else
							{
								SET_BIT(join_in_progress_machine_mask, machine_index, true);
							}
						}
						else
						{
							SET_BIT(join_waiting_machine_mask, machine_index, true);
						}
					}
					else
					{
						update_establishing_view(view);
					}
				}
			}
		}
	}

	join_blocking_machine_mask = 
		join_client_machine_mask &
		m_world_state_data.joining.join_client_machine_mask &
		~join_complete_machine_mask;

	
	if (!join_blocking_machine_mask || join_wait_time_msec >= global_network_configuration_get()->client_join_timeout)
	{
		event(
			_event_message,
			"simulation:world: update_authority_join: JOIN-%s, clients total/conn/est/wait/join/complete"
			" 0x%04X/0x%04X/0x%04X/0x%04X/0x%04X/0x%04X join-client/block 0x%04X/0x%04X after %dms",
			join_waiting_machine_mask ? "TIMEOUT" : "COMPLETE",
			join_client_machine_mask,
			join_connected_machine_mask,
			join_established_machine_mask,
			join_waiting_machine_mask,
			join_in_progress_machine_mask,
			join_complete_machine_mask,
			m_world_state_data.joining.join_client_machine_mask,
			join_blocking_machine_mask,
			join_wait_time_msec
		);

		change_state_active();
	}

	return;
}

void c_simulation_world::update_authority_active(
	void)
{
	INVOKE_TYPE(0x1DDDCE, 0x0, void(__thiscall*)(c_simulation_world*), this);

	return;
}

void c_simulation_world::update_authority_handoff(
	void)
{
	ASSERT(exists());
	ASSERT(is_authority());
	ASSERT(m_world_state==_simulation_world_state_handoff);

	s_simulation_world_view_iterator iterator;
	iterator_begin(&iterator, (uint32)NONE);

	c_simulation_view* view;
	while (iterator_next(&iterator, &view))
	{
		ASSERT(view);

		if (!view->is_dead(NULL) && view->get_view_establishment_mode() > _simulation_view_establishment_mode_established)
		{
			event(
				_event_message,
				"simulation:world: update_authority_handoff: view %s being paused for handoff (mode %d -> %d)",
				view->get_view_description(),
				view->get_view_establishment_mode(),
				_simulation_view_establishment_mode_established
			);

			view->set_view_establishment(
				_simulation_view_establishment_mode_established,
				view->get_view_establishment_identifier());
		}
	}

	return;
}

void c_simulation_world::update_client_join_initiate(
	void)
{
	c_simulation_view* view = get_authority_view();
	
	ASSERT(exists());
	ASSERT(!is_authority());
	ASSERT(!is_connected() && !is_joining() && !is_dead());

	if (view && view->get_channel_index()!=NONE && !view->is_dead(NULL))
	{
		event(
			_event_message,
			"simulation:world: client join initiated over remote authority view %s (mode %d -> %d)",
			view->get_view_description(),
			view->get_view_establishment_mode(),
			_simulation_view_establishment_mode_connected
		);

		view->set_view_establishment(_simulation_view_establishment_mode_connected, NONE);
		change_state_joining(0);
	}

	return;
}

void c_simulation_world::update_client_join_progress(
	void)
{
	c_simulation_view* authority_view = get_authority_view();
	bool should_disconnect = false;

	ASSERT(exists());
	ASSERT(!is_authority());
	ASSERT(is_joining());

	if (!authority_view)
	{
		event(_event_message, "simulation:world: client join aborted, remote authority view has been deleted");
		should_disconnect = true;
	}
	else
	{
		if (authority_view->active())
		{
			event(_event_message, "simulation:world: client join complete, going active");
			change_state_active();
		}
		else
		{
			s_network_configuration* g_network_configuration = global_network_configuration_get();

			const int32 time_since_disconnect = network_time_since(m_world_state_data.disconnected.disconnected_timestamp);
			if (time_since_disconnect > g_network_configuration->client_join_timeout)
			{
				event(
					_event_warning,
					"simulation:world: client join timeout, aborting after %d>%dmsec",
					time_since_disconnect,
					g_network_configuration->client_join_timeout
				);
				should_disconnect = true;
			}
		}
	}

	if (should_disconnect)
	{
		disconnect();
	}

	return;
}

void c_simulation_world::update_client_failure(
	void)
{
	bool simulation_failed = false;

	int32 time_since_last_active = network_time_since(m_last_active_timestamp);
	
	ASSERT(exists());
	ASSERT(!is_authority());
	ASSERT(!is_active());

	s_network_configuration* g_network_configuration=  global_network_configuration_get();
	if (m_unsuccessful_join_attempts < g_network_configuration->max_join_attempts)
	{
		if (time_since_last_active < g_network_configuration->client_active_timeout)
		{
		}
		else
		{
			event(
				_event_warning,
				"simulation:world: client not yet active after %d > %d msec (%d join failures), simulation has failed, world is dying",
				time_since_last_active,
				g_network_configuration->client_active_timeout,
				m_unsuccessful_join_attempts
			);
			simulation_failed = true;
		}
	}
	else
	{
		event(
			_event_warning,
			"simulation:world: client activation failed %d times after %d msec, simulation has failed, world is dying",
			m_unsuccessful_join_attempts,
			time_since_last_active
		);
		simulation_failed = true;
	}


	if (simulation_failed)
	{
		change_state_dead();
	}

	return;
}

void c_simulation_world::update_client_disconnection(
	void)
{
	bool disconnected = false;

	ASSERT(exists());
	ASSERT(is_connected() || is_joining());

	c_simulation_view const* view = get_authority_view();
	if (view)
	{
		if (!view->established() && !is_joining())
		{
			disconnected = true;
		}
	}
	else
	{
		disconnected = true;
	}

	if (disconnected)
	{
		disconnect();
	}

	return;
}

void c_simulation_world::gamestate_flush(
	void)
{
	ASSERT(exists());
	ASSERT(m_world_type==_simulation_world_type_synchronous_client);

	game_state_call_before_save_procs(0);
	game_state_call_after_save_procs(0);

	return;
}

c_simulation_queue* c_simulation_world::queue_get(
	e_simulation_queue_type type) const
{
	return &g_simulation_queues[type];
}

void c_simulation_world::simulation_queue_allocate(
	e_event_queue_type type,
	int32 data_size,
	s_simulation_queue_element** out_allocated_elem)
{
	ASSERT(type != _simulation_queue_element_type_none);
	ASSERT(data_size > 0);
	ASSERT(out_allocated_elem != NULL);

	*out_allocated_elem = NULL;
	if (TEST_FLAG(FLAG(type), _simulation_queue_element_type_bookkeeping))
	{
		// player event, player update, gamestate clear
		queue_get(_simulation_queue_bookkeeping)->allocate(data_size, out_allocated_elem);
	}
	else
	{
		bool sim_queue_restrict_allocations = false;
		c_simulation_queue* simulation_queue = queue_get(_simulation_queue);

		if (!TEST_FLAG(FLAG(type), _simulation_queue_element_important_update))
		{
			real32 allocated_percentage;
			real32 allocated_in_bytes_percentage;
			simulation_queue->get_allocation_status(&allocated_percentage, &allocated_in_bytes_percentage);

			// if we allocated more than 90% of the buffer
			// skip some updates to aleviate some of the stress on the queue
			// especially if the game froze for multiple seconds
			// and allow the allocation for important updates only
			// entity deletion, entity promotion, and global game events
			if (allocated_percentage > 90.f / 100.f
				|| allocated_in_bytes_percentage > 90.f / 100.f)
			{
				sim_queue_restrict_allocations = true;
			}
		}

		// event, creation, update, entity_deletion, entity_promotion, game_global_event
		if (!sim_queue_restrict_allocations)
		{
			simulation_queue->allocate(data_size, out_allocated_elem);
		}
	}

	if (*out_allocated_elem)
	{
		(*out_allocated_elem)->type = type;
	}

	return;
}

void c_simulation_world::simulation_queue_free(
	s_simulation_queue_element* element)
{
	if (TEST_FLAG(FLAG(element->type), _simulation_queue_element_type_bookkeeping))
	{
		// player event, player update, gamestate clear
		queue_get(_simulation_queue_bookkeeping)->deallocate(element);
	}
	else
	{
		queue_get(_simulation_queue)->deallocate(element);
	}

	return;
}

void c_simulation_world::simulation_queue_enqueue(
	s_simulation_queue_element* element)
{
	ASSERT(element);
	ASSERT(element->type != _simulation_queue_element_type_none);
	ASSERT(element->data_size > 0);

	if (runs_simulation())
	{
		if (TEST_FLAG(FLAG(element->type), _simulation_queue_element_type_bookkeeping))
		{
			// player event, player update, gamestate clear
			queue_get(_simulation_queue_bookkeeping)->enqueue(element);
		}
		else
		{
			// event, creation, update, entity_deletion, entity_promotion, game_global_event
			queue_get(_simulation_queue)->enqueue(element);
		}
	}
	
	return;
}

void c_simulation_world::apply_simulation_queue(
	const c_simulation_queue* simulation_queue)
{
	ASSERT(simulation_queue != NULL);

	if (simulation_queue->queued_count() > 0)
	{
		s_simulation_queue_element* element = simulation_queue->get_first_element();
		int32 update_count = 0;
		int32 total_size = 0;

		while (element != NULL)
		{
			switch (element->type)
			{
			case _simulation_queue_element_type_event:
				simulation_queue_event_apply(element);
				break;
			case _simulation_queue_element_type_entity_creation:
				simulation_queue_entity_creation_apply(element);
				break;
			case _simulation_queue_element_type_entity_update:
				simulation_queue_entity_update_apply(element);
				break;
			case _simulation_queue_element_type_entity_deletion:
				simulation_queue_entity_deletion_apply(element);
				break;
			case _simulation_queue_element_type_entity_promotion:
				simulation_queue_entity_promotion_apply(element);
				break;
			case _simulation_queue_element_type_game_global_event:
				simulation_queue_game_global_event_apply(element);
				break;
			case _simulation_queue_element_type_player_event:
				simulation_queue_player_event_apply(element);
				break;
			case _simulation_queue_element_type_player_update_event:
				simulation_queue_player_update_apply(element);
				break;
			case _simulation_queue_element_type_gamestates_clear:
				break;
			case _simulation_queue_element_type_sandbox_event:
				ASSERT(false);
				break;
			default:
				event(
					_event_error,
					"networking:simulation:world: apply_simulation_queue() unknown/invalid element type %d",
					element->type
				);
				break;
			}

			++update_count;
			total_size += simulation_queue->get_element_size_in_bytes(element);
			element = simulation_queue->get_next_element(element);
		}

		if (update_count != simulation_queue->queued_count())
		{
			event(
				_event_error,
				"networking:simulation:world: simulation queue from simulation update count mismatch [%d != %d]",
				update_count,
				simulation_queue->queued_count()
			);
		}

		if (total_size != simulation_queue->queued_size_in_bytes())
		{
			event(
				_event_error,
				"networking:simulation:world: simulation queue from simulation update size mismatch [%d != %d]",
				total_size,
				simulation_queue->queued_size_in_bytes()
			);
		}
	}
	return;
}

bool c_simulation_world::simulation_queues_empty(
	void) const
{
	return queue_get(_simulation_queue_bookkeeping)->queued_count() == 0 && queue_get(_simulation_queue)->queued_count() == 0;
}

void c_simulation_world::delete_all_players(
	void)
{
	for (int32 player_index = 0; player_index<NUMBEROF(m_players); ++player_index)
	{
		if (m_players[player_index].exists())
		{
			m_players[player_index].destroy();
		}
	}
	return;
}

void c_simulation_world::delete_all_actors(
	void)
{
	for (uint32 i = 0; i < NUMBEROF(m_actors); i++)
	{
		c_simulation_actor* actor = &m_actors[i];
		if (actor->m_actor_index != NONE)
		{
			actor->destroy();
		}
	}
	return;
}

void c_simulation_world::update_queue_reset(
	void)
{
	//INVOKE_TYPE(0x1DCDC3, 0x1C4277, void(__thiscall*)(c_simulation_world*), this);
	
	ASSERT(exists());
	ASSERT(!m_time_running);
	ASSERT(exists());
	ASSERT(!runs_simulation());
	
	while (m_update_queue_head)
	{
		//network_heap_verify_block(m_update_queue_head);
		s_simulation_queued_update* next= m_update_queue_head->next_node;
		network_heap_free_block(m_update_queue_head);
		m_update_queue_head = next;
	}

	m_update_queue_head = NULL;
	m_update_queue_tail = NULL;
	m_update_queue_length = 0;
	m_update_queue_next_update_number_to_dequeue = 0;
	m_update_queue_latest_update_number_received = NONE;

	return;
}

c_simulation_player* c_simulation_world::find_player_by_machine(
	s_machine_identifier const* machine_identifier,
	int32 user_index)
{
	s_machine_identifier machine_identifiers[k_network_maximum_machines_per_session];

	int32 machine_index = NONE;
	c_simulation_player* player = NULL;

	ASSERT(machine_identifier);
	ASSERT(user_index>=0 && user_index<k_number_of_users);
	
	uint32 machine_valid_mask;
	players_get_machines(&machine_valid_mask, machine_identifiers);

	for (int32 machine_num = 0; machine_num <NUMBEROF(machine_identifiers); ++machine_num)
	{
		if (TEST_BIT(machine_valid_mask, machine_num) && 
			!csmemcmp(machine_identifier, &machine_identifiers[machine_num], sizeof(*machine_identifier)))
		{
			machine_index = machine_num;
			break;
		}
	}

	if (machine_index!=NONE)
	{
		for (int32 player_index = 0; player_index<NUMBEROF(m_players); ++player_index)
		{
			if (m_players[player_index].exists())
			{
				int32 player_datum_index = m_players[player_index].get_player_index();
				struct player_datum const* player_datum = player_get(player_datum_index);

				if (player_datum->machine_index==machine_index &&
					player_datum->user_index==user_index)
				{
					ASSERT(!TEST_BIT(player_datum->flags, _player_left_game_bit));

					player = &m_players[player_index];
					
					ASSERT(player->exists());
					
					break;
				}
			}
		}
	}

	return player;
}


uint32 c_simulation_world::get_acknowledged_player_mask(
	void) const
{
	return INVOKE_TYPE(0x1DCA76, 0x0, uint32(__thiscall*)(c_simulation_world const*), this);
}

void c_simulation_world::send_player_acknowledgements_not_during_simulation_reset_in_progress(bool a1)
{
	if (!simulation_reset_in_progress())
	{
		send_player_acknowledgements(a1);
	}
}

void c_simulation_world::queues_initialize(void)
{
	for (int32 i = 0; i < k_simulation_queue_count; i++)
	{
		queue_get((e_simulation_queue_type)i)->initialize();
	}
}

bool c_simulation_world::synchronous_catchup_in_progress(
	void) const
{
	return m_world_type && !is_authority() && !is_distributed() && m_synchronous_gamestate_write_progress!=NONE;
}

void c_simulation_world::update_queue_retrieve_update(
	struct simulation_update* update)
{
	//INVOKE_TYPE(0x1DCE7C, 0x0, void(__thiscall*)(c_simulation_world *, struct simulation_update*), this, update);
	//return;
	ASSERT(update);
	ASSERT(exists());
	ASSERT(m_time_running);
	ASSERT(!runs_simulation());
	
	
	ASSERT(m_update_queue_next_update_number_to_dequeue <= m_update_queue_latest_update_number_received);
	ASSERT(m_update_queue_length > 0);
	ASSERT(m_update_queue_head != NULL);

	s_simulation_queued_update* update_node = m_update_queue_head;
	
	//network_heap_verify_block(update_node);
	ASSERT(update_node->update.update_number == m_update_queue_next_update_number_to_dequeue);

	csmemcpy(update, update_node, sizeof(*update));

	const bool last_node = m_update_queue_tail == update_node;
	m_update_queue_head = update_node->next_node;
	if (last_node)
	{
		m_update_queue_tail = NULL;
	}

	network_heap_free_block(update_node);
	
	const int32 new_length= --m_update_queue_length;
	++m_update_queue_next_update_number_to_dequeue;
	if (new_length>0)
	{
		ASSERT(m_update_queue_head!=NULL);

		ASSERT(m_update_queue_head->update.update_number==m_update_queue_next_update_number_to_dequeue);
	}
	return;
}

const char* c_simulation_world::get_state_string(
	int32 world_state)
{
	const char* result;

	switch (world_state)
	{
	case _simulation_world_state_none:
		result = "none";
		break;
	case _simulation_world_state_dead:
		result = "dead";
		break;
	case _simulation_world_state_disconnected:
		result = "disconnected";
		break;
	case _simulation_world_state_joining:
		result = "joining";
		break;
	case _simulation_world_state_active:
		result = "active";
		break;
	case _simulation_world_state_handoff:
		result = "handoff";
		break;
	case _simulation_world_state_leaving:
		result = "leaving";
		break;
	default:
		result = "<unknown>";
		break;
	}

	return result;
}

void c_simulation_world::send_player_acknowledgements(
	bool force_acknowledgement)
{
	INVOKE_TYPE(0x1DD777, 0x1C4C37, void(__thiscall*)(c_simulation_world*, bool), this, force_acknowledgement);
	return;
}

/* private code*/

c_simulation_world::e_join_progress c_simulation_world::update_joining_view(
	c_simulation_view* view)
{
	return INVOKE_TYPE(
		0x1DD4BB,
		0x0,
		c_simulation_world::e_join_progress(__thiscall*)(c_simulation_world*, c_simulation_view *view),
		this,
		view
	);
}

void c_simulation_world::update_establishing_view(
	c_simulation_view* view)
{
	ASSERT(exists());
	ASSERT(is_authority());
	ASSERT(view);
	ASSERT(!view->established());

	if (view->get_view_establishment_mode()!=_simulation_view_establishment_mode_established)
	{
		if (view->ready_to_establish())
		{
			int32 new_establishment_identifier = m_next_view_establishment_identifier;
			m_next_view_establishment_identifier++;

			event(
				_event_message,
				"simulation:world: simulation connected, go established - advancing remote client view %s (mode %d -> %d, new identifier %d)",
				view->get_view_description(),
				view->get_view_establishment_mode(),
				_simulation_view_establishment_mode_established,
				m_next_view_establishment_identifier
			);

			view->set_view_establishment(_simulation_view_establishment_mode_established, new_establishment_identifier);
		}
		else if (view->get_view_establishment_mode()!=_simulation_view_establishment_mode_connected)
		{
			event(
				_event_message,
				"simulation:world: view ready to connect, advancing remote client view %s (mode %d -> %d)",
				view->get_view_description(),
				view->get_view_establishment_mode(),
				_simulation_view_establishment_mode_connected
			);

			view->set_view_establishment(_simulation_view_establishment_mode_connected, NONE);
		}
	}

	return;
}


void c_simulation_world::verify_player_activation(
	void) const
{
	if (is_authority())
	{
		uint32 player_acknowledged_mask = get_acknowledged_player_mask();
		for (int32 player_index= 0; player_index <NUMBEROF(m_players); ++player_index)
		{
			c_simulation_player const* player = &m_players[player_index];

			if (player->exists() && player->active())
			{
				s_player_identifier player_identifier;

				ASSERT(TEST_BIT(player_acknowledged_mask, player_index));

				player->get_identifier(&player_identifier);

				ASSERT(m_watcher->get_player_is_in_game(player_index, &player_identifier));
			}
		}
	}

	return;
}

void c_simulation_world::update_player_activation(
	void)
{
	uint32 player_acknowledged_mask = get_acknowledged_player_mask();

	for (int32 player_index = 0; player_index<NUMBEROF(m_players); ++player_index)
	{
		c_simulation_player* player = &m_players[player_index];
		if (player->exists() && !player->active())
		{
			s_player_identifier player_identifier;
			player->get_identifier(&player_identifier);

			if (m_watcher->get_player_is_in_game(player_index, &player_identifier))
			{
				if (TEST_BIT(player_acknowledged_mask, player_index) && !player->pending_deletion())
				{
					event(_event_message, "simulation:world: player %d going active", player_index);
					player->set_active(true);
				}
			}
		}
	}

	verify_player_activation();

	return;
}

bool c_simulation_world::synchronous_gamestate_write_in_progress(
	void) const
{
	return exists() && !is_authority() && !is_distributed() && m_synchronous_gamestate_write_progress!=NONE;
}

void c_simulation_world::synchronous_gamestate_clear(
	void)
{
	ASSERT(exists());
	ASSERT(m_world_type==_simulation_world_type_synchronous_client);
	ASSERT(synchronous_gamestate_write_in_progress());

	ASSERT(m_synchronous_gamestate_write_progress!=NONE);
	ASSERT(m_synchronous_gamestate_write_buffer!=NULL);

	texture_cache_free(m_synchronous_gamestate_write_buffer);
	m_synchronous_gamestate_write_buffer = NULL;
	m_synchronous_gamestate_write_progress = NONE;

	return;
}

void c_simulation_world::update_queue_start(
	int32 next_update_number)
{
	ASSERT(exists());
	ASSERT(!m_time_running);
	ASSERT(!runs_simulation());

	update_queue_reset();

	ASSERT(m_update_queue_length==0);

	m_update_queue_latest_update_number_received = next_update_number-1;
	m_update_queue_next_update_number_to_dequeue = next_update_number;

	event(
		_event_message,
		"simulation:world: update queue started at #%d (expected: #%d)",
		m_update_queue_latest_update_number_received,
		m_update_queue_next_update_number_to_dequeue
	);

	return;
}

void c_simulation_world::update_queue_stop(
	void)
{
	ASSERT(exists());
	ASSERT(m_time_running);
	ASSERT(!runs_simulation());

	update_queue_reset();

	return;
}

bool c_simulation_world::update_queue_handle_server_update(
	const struct simulation_update* update)
{
	bool success = false;

	ASSERT(update);

	ASSERT(exists());
	ASSERT(m_time_running);
	ASSERT(!runs_simulation());
	ASSERT(update->update_number==update_queue_get_next_expected_update_number());
	ASSERT(m_update_queue_tail==NULL || (update->update_number==m_update_queue_tail->update.update_number+1));

	struct s_simulation_queued_update* update_storage = (struct s_simulation_queued_update*)network_heap_allocate_block(sizeof(*update_storage));
	if (update_storage)
	{
		if (m_update_queue_tail)
		{
			//network_heap_verify_block(m_update_queue_tail);
			m_update_queue_tail->next_node = update_storage;
		}
		else
		{
			m_update_queue_head = update_storage;
		}
		
		update_storage->next_node = NULL;
		++m_update_queue_length;
		m_update_queue_tail= update_storage;

		event(
			_event_verbose,
			"simulation:world: update queue received #%d (previously received: #%d)",
			update->update_number,
			m_update_queue_next_update_number_to_dequeue
		);

		update_storage->update = *update;
		m_update_queue_latest_update_number_received = update->update_number;
		success = true;
	}
	else
	{
#ifdef EVENTS_ENABLED
		char heapbuf[1024];
		event(
			_event_error,
			"simulation:world: OUT OF MEMORY allocating stored update [#%d] (queue [#%d]/[#%d] length [%d]) [%s]",
			update->update_number,
			m_update_queue_next_update_number_to_dequeue,
			m_update_queue_latest_update_number_received,
			m_update_queue_length,
			network_heap_describe(heapbuf, sizeof(heapbuf))
		);
#endif
	}

	return success;
}

int32 c_simulation_world::update_queue_get_available_updates(
	void) const
{
	ASSERT(exists());
	ASSERT(m_time_running);
	ASSERT(!runs_simulation());

	const int32 available_updates = m_update_queue_latest_update_number_received - m_update_queue_next_update_number_to_dequeue + 1;
	ASSERT(m_update_queue_length==available_updates);

	return available_updates;
}

int32 c_simulation_world::update_queue_get_next_expected_update_number(
	void) const
{
	return m_update_queue_latest_update_number_received+1;
}

void c_simulation_world::attach_simulation_queues_to_update(
	struct simulation_update* update)
{
	ASSERT(update);

	c_simulation_queue* bookkeeping_simulation_queue = queue_get(_simulation_queue_bookkeeping);

	if (bookkeeping_simulation_queue->queued_count() > 0)
	{
		ASSERT(bookkeeping_simulation_queue->queued_size_in_bytes() > 0);
		update->bookkeeping_simulation_queue.transfer_elements(bookkeeping_simulation_queue);
	}
	else
	{
		ASSERT(bookkeeping_simulation_queue->queued_size_in_bytes() == 0);
	}

	c_simulation_queue* game_simulation_queue = queue_get(_simulation_queue);

	if (game_simulation_queue->queued_count() > 0)
	{
		ASSERT(game_simulation_queue->queued_size_in_bytes() > 0);
		update->game_simulation_queue.transfer_elements(game_simulation_queue);
	}
	else
	{
		ASSERT(game_simulation_queue->queued_size_in_bytes() == 0);
	}

	return;
}
