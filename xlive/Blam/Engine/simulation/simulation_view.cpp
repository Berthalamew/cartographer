#include "stdafx.h"
#include "simulation_view.h"

#include "simulation.h"
#include "simulation_world.h"

#include "cache/pc_texture_cache.h"
#include "networking/network_configuration.h"
#include "networking/network_event.h"
#include "networking/network_memory.h"
#include "networking/network_time.h"
#include "networking/delivery/network_link.h"
#include "networking/messages/network_messages.h"
#include "networking/messages/network_messages_simulation.h"
#include "networking/messages/network_messages_simulation_synchronous.h"
#include "networking/session/network_observer.h"

/* globals */

const char* g_simulation_view_reason_strings[k_simulation_view_reason_count]
{
	"none",
	"disconnected",
	"out-of-sync",
	"failed-to-join",
	"blocking",
	"catchup-fail",
	"ended",
	"mode-error",
	"player-error",
	"replication-entity",
	"replication-event",
	"replication-game-results",
	"no-longer-authority"
};

/* public code */

char const* c_simulation_view::get_view_description(
	void) const
{
#ifdef SIMULATION_VIEW_DEBUG
	csprintf(
		m_view_description,
		NUMBEROF(m_view_description),
		"v%d/m%d/%s",
		m_world_view_index,
		m_remote_machine_index,
		m_remote_machine_name);
	return m_view_description;
#else
	return "FIXME";
#endif
}

char const* c_simulation_view::get_death_reason_string(
	uint32 death_reason) const
{
	const char* result = "<unknown>";

	if (VALID_INDEX(death_reason, k_simulation_view_reason_count))
	{
		result = g_simulation_view_reason_strings[death_reason];
	}

	return result;
}

bool c_simulation_view::observer_channel_game_results_backlogged(
	void)
{
	bool backlogged;

	if (m_observer_channel_index!=NONE)
	{
		ASSERT(m_observer!=NULL);

		backlogged = m_observer->observer_channel_backlogged(_network_observer_owner_simulation, m_observer_channel_index, _network_message_type_game_results);
	}
	else
	{
		ASSERT(is_dead(NULL));

		backlogged = true;
	}

	return backlogged;
}

void c_simulation_view::observer_channel_set_waiting_on_backlog(
	e_network_message_type message_type)
{
	if (m_observer_channel_index!=NONE)
	{
		ASSERT(m_observer!=NULL);

		m_observer->observer_channel_set_waiting_on_backlog(_network_observer_owner_simulation, m_observer_channel_index, message_type);
	}

	return;
}

void c_simulation_view::send_message(
	e_network_message_type message_type,
	int32 message_size,
	const void* message_payload)
{
	if (m_observer_channel_index!=NONE)
	{
		ASSERT(m_observer!=NULL);
		
		m_observer->send_message(_network_observer_owner_simulation, m_observer_channel_index, false, message_type, message_size, message_payload);
	}
	else
	{
		ASSERT(is_dead(NULL));
	}

	return;
}

void c_simulation_view::kill_view(
	e_simulation_view_reason death_reason)
{
	if (is_dead(NULL))
	{
		event(
			_event_message,
			"simulation:view: view %s dying (%s) in mode %d/%d",
			get_view_description(),
			get_death_reason_string(death_reason),
			m_view_establishment_mode,
			m_view_establishment_identifier
		);

		set_view_establishment(_simulation_view_establishment_mode_none, NONE);
		m_view_death_reason = death_reason;
	}

	return;
}

void c_simulation_view::set_view_establishment(
	e_simulation_view_establishment_mode establishment_mode,
	int32 establishment_identifier)
{
	// Param validation
	ASSERT(exists());
	ASSERT(establishment_mode>=0 && establishment_mode<k_simulation_view_establishment_mode_count);
	
	// Class validation
	ASSERT(m_world!=NULL);
	ASSERT(m_channel_index != NONE);


	bool valid_mode;

	if (is_dead(NULL))
	{
		ASSERT(establishment_mode==_simulation_view_establishment_mode_none);
	}
	
	if (establishment_mode>=_simulation_view_establishment_mode_established)
	{
		if (establishment_mode!=_simulation_view_establishment_mode_established)
		{
			valid_mode =
				establishment_identifier==m_view_establishment_identifier &&
				establishment_mode==m_view_establishment_mode+1;
		}
		else
		{
			valid_mode = establishment_identifier>=0;
		}
	}
	else
	{
		valid_mode = establishment_identifier==NONE;
	}

	if (!valid_mode)
	{
		event(
			_event_error,
			"simulation:view: view %s mode logic error: not uniformly ascending while above established (%d/%d -> %d/%d)",
			get_view_description(),
			m_view_establishment_mode,
			m_view_establishment_identifier,
			establishment_mode,
			establishment_identifier
		);
		kill_view(_simulation_view_reason_mode_error);
	}
	else
	{
		if (m_view_establishment_mode == establishment_mode &&
			m_view_establishment_identifier == establishment_identifier)
		{
			event(
				_event_status,
				"simulation:view: view %s suppressing duplicate mode %d/%d",
				get_view_description(),
				m_view_establishment_mode,
				m_view_establishment_identifier
			);
		}
		else
		{
			event(
				_event_status,
				"simulation:view: view %s mode change %d/%d -> %d/%d",
				get_view_description(),
				m_view_establishment_mode,
				m_view_establishment_identifier,
				establishment_mode,
				establishment_identifier
			);

			m_view_establishment_mode = establishment_mode;
			m_view_establishment_identifier = establishment_identifier;
			
			s_network_message_view_establishment message;
			csmemset(&message, 0, sizeof(message));

			message.establishment_mode = m_view_establishment_mode;
			message.establishment_identifier = m_view_establishment_identifier;
			send_message(_network_message_type_view_establishment, sizeof(message), &message);
			
			update_view_activation_state();
		}
	}

	return;
}

void c_simulation_view::update(
	void)
{
	//INVOKE_TYPE(0x1DF78A, 0x0, void(__thiscall*)(c_simulation_view*), this);
	//return;

	ASSERT(exists());
	ASSERT(m_world!=NULL);

	if (!is_dead(NULL) && is_distributed())
	{
		ASSERT(m_distributed_view);
		
		if (m_distributed_view->m_entity_view.has_fatal_error())
		{
			event(_event_error, "simulation:view: fatal entity replication error raised, dying");
		}
		else if (m_distributed_view->m_event_view.has_fatal_error())
		{
			event(_event_error, "simulation:view: fatal event replication error raised, dying");
		}
		else if (m_distributed_view->m_game_results_replicator.has_fatal_error())
		{
			event(_event_error, "simulation:view: fatal game results replication error raised, dying");
		}
	}

	if (!is_dead(NULL) && is_distributed())
	{
		ASSERT(m_distributed_view);
		m_distributed_view->m_game_results_replicator.update();
	}

	return;
}

uint32 c_simulation_view::get_acknowledged_player_mask(
	void) const
{
	ASSERT(exists());
	ASSERT(is_client_view());
	ASSERT(established());

	return m_simulation_player_acknowledged_mask;
}

bool c_simulation_view::handle_synchronous_update(
	struct simulation_update const* update)
{
	ASSERT(exists());
	ASSERT(m_view_type==_simulation_view_type_synchronous_to_remote_authority);
	ASSERT(m_world!=NULL);
	ASSERT(update);

	return m_world->time_running() ? m_world->handle_synchronous_update(update) : false;
}

void c_simulation_view::dispatch_synchronous_actions(
	uint32 valid_user_mask,
	struct player_action const* user_actions)
{
	ASSERT(exists());
	ASSERT(m_view_type==_simulation_view_type_synchronous_to_remote_authority);
	ASSERT(m_world!=NULL);

	if (active())
	{
		s_network_message_synchronous_actions message;
		csmemset(&message, 0, sizeof(message));
		
		message.action_number = m_synchronous_catchup_stream_items;
		m_synchronous_catchup_stream_items = message.action_number+1;

		message.client_update_number = m_world->get_next_update_number()-1;
		message.go_out_of_sync = m_world->is_out_of_sync();
		message.valid_user_mask = valid_user_mask;

		if (TEST_BIT(valid_user_mask, 0))
		{
			message.actions[0] = user_actions[0];
		}

		if (TEST_BIT(valid_user_mask, 1))
		{
			message.actions[1] = user_actions[1];
		}

		if (TEST_BIT(valid_user_mask, 2))
		{
			message.actions[2] = user_actions[2];
		}

		if (TEST_BIT(valid_user_mask, 3))
		{
			message.actions[3] = user_actions[3];
		}

		send_message(_network_message_type_synchronous_actions, sizeof(message), &message);
	}

	return;
}

void c_simulation_view::dispatch_synchronous_update(
	struct simulation_update const* update)
{

	ASSERT(exists());
	ASSERT(m_view_type==_simulation_view_type_synchronous_to_remote_client);
	ASSERT(m_world!=NULL);
	ASSERT(update);

	if (m_view_establishment_mode == _simulation_view_establishment_mode_active)
	{
		s_network_message_synchronous_update message;
		csmemset(&message, 0, sizeof(message));
		message.update = *update;
		
		send_message(_network_message_type_synchronous_update, sizeof(message), &message);
	}
	else if (synchronous_catchup_in_progress())
	{
		if (synchronous_catchup_complete(update))
		{
			synchronous_catchup_send_data();
		}
		else
		{
			event(
				_event_error,
				"simulation:view: view [%s] synchronous-catchup failed to submit update [#%d]",
				get_view_description(),
				update->update_number
			);

			kill_view(_simulation_view_reason_synchronous_catchup_fail);
		}
	}
	else
	{
		event(
			_event_verbose,
			"simulation:view: view %s not yet in game, skipping synchronous-update [#%d]",
			get_view_description(),
			update->update_number
		);
	}

	return;
}

uint32 c_simulation_view::synchronous_client_get_acknowledged_update_number(
	void) const
{
	ASSERT(exists());
	ASSERT(m_view_type==_simulation_view_type_synchronous_to_remote_client);

	return m_synchronous_acknowledged_update_number;
}

void c_simulation_view::synchronous_client_block(
	bool block)
{
	ASSERT(exists());
	ASSERT(m_view_type==_simulation_view_type_synchronous_to_remote_client);

	if (block && !m_synchronous_client_blocked)
	{
		m_synchronous_client_block_timestamp = network_time_get();
	}
	m_synchronous_client_blocked = block;

	if (block && network_time_since(m_synchronous_client_block_timestamp) >= 2000)
	{
		event(
			_event_message,
			"simulation:view: view [%s] has blocked for [%dms] and is now dead at update/time [#%d]/[#%d]",
			get_view_description(),
			network_time_since(m_synchronous_client_block_timestamp),
			m_world->get_next_update_number(),
			m_world->get_time()
		);
		kill_view(_simulation_view_reason_synchronous_block);
	}

	return;
}

bool c_simulation_view::synchronous_catchup_in_progress(
	void) const
{
	ASSERT(exists());
	ASSERT(m_world!=NULL);
	ASSERT(m_view_type==_simulation_view_type_synchronous_to_remote_client);

	return m_synchronous_catchup_buffer!=NULL;
}

void c_simulation_view::update_view_activation_state(
	void)
{
	//INVOKE_TYPE(0x1DEF6D, 0x0, void(__thiscall*)(c_simulation_view*), this);

	bool simulation_established = false;
	bool simulation_active = false;

	if (m_world && m_channel_index != NONE && m_view_establishment_identifier==m_remote_establishment_identifier)
	{
		simulation_established=
			m_view_establishment_mode >= _simulation_view_establishment_mode_established &&
			m_remote_establishment_mode >= _simulation_view_establishment_mode_established;
		simulation_active= 
			m_view_establishment_mode >= _simulation_view_establishment_mode_active &&
			m_remote_establishment_mode >= _simulation_view_establishment_mode_established;
	}

	ASSERT(!(simulation_active && !simulation_established));

	if (simulation_established != established())
	{
		event(
			_event_message,
			"simulation:view: view %s simulation is now %s",
			get_view_description(),
			simulation_established ? "ESTABLISHED" : "NON-ESTABLISHED"
		);

		m_channel_interface.set_established(simulation_established);
		
		if (!simulation_established)
		{
			if (is_client_view())
			{
				if (m_view_type == _simulation_view_type_synchronous_to_remote_client)
				{
					m_synchronous_received_action_number = NONE;
					m_synchronous_acknowledged_update_number = NONE;
					if (synchronous_catchup_in_progress())
					{
						synchronous_catchup_terminate();
					}
				}
				else if (m_view_type == _simulation_view_type_distributed_to_remote_client)
				{
					distributed_join_abort();
				}
			}
			else if (m_view_type == _simulation_view_type_synchronous_to_remote_authority)
			{
				m_synchronous_catchup_stream_items = 0;
			}
		}

		if (is_distributed())
		{
			m_distributed_view->m_game_results_replicator.handle_view_establishment(simulation_established);
		}

		ASSERT(m_world);
		m_world->handle_view_establishment(this, simulation_established);
	}

	if (m_simulation_active != simulation_active)
	{
		event(
			_event_message,
			"simulation:view: view %s simulation is now %s",
			get_view_description(),
			simulation_active ? "ACTIVE" : "INACTIVE"
		);

		m_simulation_active = simulation_active;

		ASSERT(m_world);
		m_world->handle_view_activation(this, m_simulation_active);
	}

	return;
}

void c_simulation_view::synchronous_catchup_terminate(
	void)
{
	if (synchronous_catchup_in_progress())
	{
		m_synchronous_catchup_stream.detach();

		ASSERT(m_synchronous_catchup_buffer!=NULL);

		texture_cache_free(m_synchronous_catchup_buffer);
		m_synchronous_catchup_buffer = NULL;
		m_synchronous_catchup_buffer_size = 0;
	}

	return;
}

void c_simulation_view::synchronous_catchup_send_data(
	void)
{
	bool success = true;

	ASSERT(synchronous_catchup_in_progress());

	while (
		success &&
		m_channel_index!=NONE &&
		established() &&
		m_synchronous_catchup_stream_items &&
		network_memory_get_channel(m_channel_index)->get_message_space_available() >= k_network_link_maximum_game_data_size)
	{
		s_network_message_synchronous_catchup message_base;
		uint8 gamestate_update_buffer[1024];

		success = false;

		// Remove base message and gamestate update (if it exists) from the stream and process
		m_synchronous_catchup_stream.remove_block(sizeof(message_base), &message_base);
		if (message_base.next_update_number>0)
		{
			m_synchronous_catchup_stream.remove_block(message_base.next_update_number, gamestate_update_buffer);
		}

		switch (message_base.type)
		{
		case _synchronous_gamestate_message_initiate_join:
		{
			success = true;
			s_network_message_synchronous_join message;
			csmemset(&message, 0, sizeof(message));
			message.next_update_number = message_base.next_update_number;

			send_message(_network_message_type_synchronous_join, sizeof(message), &message);
			--m_synchronous_catchup_stream_items;
			break;
		}
		case _synchronous_gamestate_message_gamestate_finish:
		{
			success = true;
			uint8 message_buffer[k_unsigned_short_max];
			s_network_message_synchronous_gamestate* message = (s_network_message_synchronous_gamestate*)&message_buffer;
			void* gamestate_data = &message_buffer[sizeof(*message)];

			csmemset(message, 0, sizeof(*message));
			
			message->gamestate_offset = message_base.next_update_number;
			message->gamestate_size = message_base.update_length;
			ASSERT(message->gamestate_size>0 && sizeof(*message)+message->gamestate_size<=sizeof(message_buffer));

			csmemcpy(gamestate_data, gamestate_update_buffer, message_base.update_length);
			
			send_message(_network_message_type_synchronous_gamestate, message_base.update_length+sizeof(*message), &message_buffer[12]);
			--m_synchronous_catchup_stream_items;
			break;

		}
		case _synchronous_gamestate_message_catchup:
		{
			success = true;
			struct simulation_update update;
			csmemset(&update, 0, sizeof(update));

			if (simulation_update_read_from_buffer(&update, message_base.next_update_number, gamestate_update_buffer))
			{
				send_message(_network_message_type_synchronous_update, sizeof(update), &update);
				--m_synchronous_catchup_stream_items;
			}
			else
			{
				success = false;
			}

			break;
		}
		default:
			unreachable();
			break;
		}

		if (!success)
		{
			event(
				_event_error,
				"simulation:view: view [%s] synchronous-catchup failed to send item (type %d size [%d]-bytes)",
				get_view_description(),
				message_base.type,
				message_base.next_update_number
			);
		}
		break;
	}

	return;
}

bool c_simulation_view::synchronous_catchup_complete(
	struct simulation_update const* update)
{
	int32 size;
	uint8 buffer[k_unsigned_short_max];

	bool result = false;
	
	ASSERT(synchronous_catchup_in_progress());

	if (simulation_update_write_to_buffer(update, sizeof(buffer), buffer, &size))
	{
		s_network_message_synchronous_catchup message;
		csmemset(&message, 0, sizeof(message));
		message.next_update_number = update->update_number;
		message.type = _synchronous_gamestate_message_catchup;
		message.update_length = (int16)size;

		if (m_synchronous_catchup_stream.add_block(sizeof(message), &message)!=NONE  &&
			m_synchronous_catchup_stream.add_block(size, buffer)!=NONE)
		{
			++m_synchronous_catchup_stream_items;
			result = true;
		}
	}

	return result;
}

void c_simulation_view::distributed_join_abort(
	void)
{
	ASSERT(is_distributed());
	ASSERT(m_distributed_view);

	if (m_distributed_view->m_entity_view.is_replicating())
	{
		event(_event_message, "simulation:view: view %s distributed-join terminated", get_view_description());
		m_distributed_view->m_entity_view.stop_replication();
	}

	return;
}

void c_game_results_replicator::handle_view_establishment(
	bool simulation_established)
{
	ASSERT(m_view);
	ASSERT(m_view->is_distributed());

	if (simulation_established)
	{
		ASSERT(!m_sending_updates);
		ASSERT(!m_receiving_updates);

		if (m_view->is_client_view())
		{
			start_sending_updates();
		}
		else
		{
			start_receiving_updates();
		}
	}
	else
	{
		if (m_sending_updates)
		{
			stop_sending_updates();
		}
		if (m_receiving_updates)
		{
			stop_receiving_updates();
		}

		ASSERT(!m_sending_updates);
		ASSERT(!m_receiving_updates);
	}

	return;
}

void c_game_results_replicator::update(
	void)
{
	if (m_view->is_distributed() && m_view->established())
	{
		if (m_view->is_client_view())
		{
			ASSERT(m_sending_updates);
			ASSERT(!game_results_get_game_updating());
		}
		else
		{
			ASSERT(m_receiving_updates);
			ASSERT(!game_results_get_game_recording());
			ASSERT(game_results_get_game_updating());
		}

		s_network_configuration* g_network_configuration = global_network_configuration_get();

		if (
			m_sending_updates &&
			game_results_get_game_recording() &&
			(
				m_update_timestamp==NONE ||
				!m_game_results_incremental.initialized &&
				game_results_get_game_finalized() ||
				network_time_since(m_update_timestamp)>g_network_configuration->game_results_update_interval_msec)
		)
		{
			if (m_view->observer_channel_game_results_backlogged())
			{
				m_view->observer_channel_set_waiting_on_backlog(_network_message_type_game_results);
			}
			else
			{
				send_game_results_update();
			}
		}
	}

	return;
}

void c_game_results_replicator::start_sending_updates(
	void)
{
	event(_event_status, "simulation:view: starting to send game result updates");
	ASSERT(!m_sending_updates);
	csmemset(&m_game_results_incremental, 0, sizeof(m_game_results_incremental));
	m_update_timestamp = NONE;
	m_update_number = 0;
	m_sending_updates = true;

	return;
}

void c_game_results_replicator::stop_sending_updates(
	void)
{
	event(_event_message, "simulation:view: stopping the sending of game result updates");
	ASSERT(m_sending_updates);

	m_sending_updates = false;

	return;
}

void c_game_results_replicator::start_receiving_updates(
	void)
{
	event(_event_message, "simulation:view: starting to receive game result updates");
	ASSERT(!m_receiving_updates);
	
	csmemset(&m_game_results_incremental, 0, sizeof(m_game_results_incremental));
	m_update_number = 0;
	game_results_start_updating();
	m_receiving_updates = true;

	return;
}

void c_game_results_replicator::stop_receiving_updates(
	void)
{
	event(_event_message, "simulation:view: stoping the reception of game result updates");
	ASSERT(m_receiving_updates);

	m_receiving_updates = false;

	return;
}

void c_game_results_replicator::send_game_results_update(
	void)
{
	s_game_results_incremental results;
	s_network_message_distributed_game_results message;

	ASSERT(game_results_get_game_recording());
	ASSERT(m_sending_updates);
	ASSERT(m_view->is_distributed());
	ASSERT(m_view->is_client_view());

	game_results_populate_incremental_update(&results);

	if (csmemcmp(&results, &m_game_results_incremental, sizeof(results)))
	{
		if (m_game_results_incremental.initialized)
		{
			event(
				_event_error,
				"simulation:view: send_game_results_update: view %s attempting to send game result update when results are finalized",
				m_view->get_view_description()
			);
		}
		else
		{
			event(
				_event_message,
				"simulation:view: send_game_results_update: view %s sending game result incremental update %d",
				m_view->get_view_description(),
				m_update_number
			);

			csmemset(&message, 0, sizeof(message));
			message.establishment_identifier = m_view->get_view_establishment_identifier();
			message.update_number = m_update_number;

			game_results_calculate_incremental_update(&m_game_results_incremental, &results, &message.incremental_update);

			m_view->send_message(_network_message_type_game_results, sizeof(message), &message);
			csmemcpy(&m_game_results_incremental, &results, sizeof(m_game_results_incremental));
			++m_update_number;
		}
	}

	return;
}
