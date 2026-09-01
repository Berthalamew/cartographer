#include "stdafx.h"
#include "replication_entity_manager_view.h"

#include "replication_entity.h"
#include "replication_entity_manager.h"

#include "memory/bitstream.h"
#include "networking/network_event.h"

/* public code */

bool c_replication_entity_manager_view::has_data_to_transmit(
	void)
{
	bool result = false;

	if (m_replicating)
	{
		result =
			m_statistics.creations_pending != 0 ||
			m_statistics.updates_pending != 0 ||
			m_statistics.deletions_pending != 0;
	}

	return result;
}

bool c_replication_entity_manager_view::build_outgoing_requests(
	void const* telemetry_data,
	int32 maximum_number_of_requests,
	s_replication_incoming_request* requests)
{
	return INVOKE_TYPE(0x1D180C, 0x1D6A98, bool(__thiscall*)(c_replication_entity_manager_view*, void const*, int32, s_replication_incoming_request*),
		this, telemetry_data, maximum_number_of_requests, requests);
}

int32 c_replication_entity_manager_view::terminator_required_bits(
	void)
{
	// todo: determine if its a bits_required for a specific structure or a constant
	return 3;
}

void c_replication_entity_manager_view::write_to_packet(
	void* request_identifier,
	int32 request_type,
	void const* telemetry_data,
	int32 packet_sequence_number,
	c_bitstream* packet,
	int32 must_leave_space_bits)
{
	INVOKE_TYPE(0x1D2C24, 0x1D7EB4, void(__thiscall*)(c_replication_entity_manager_view*, void*, int32, void const*, int32, c_bitstream*, int32),
		this, request_identifier, request_type, telemetry_data, packet_sequence_number, packet, must_leave_space_bits);
	return;
}

void c_replication_entity_manager_view::write_terminator_to_packet(
	c_bitstream* packet)
{
	packet->write_integer("code", 0, 3);
	m_outgoing_packet = nullptr;

	return;
}

e_network_read_result c_replication_entity_manager_view::read_from_packet(
	int32 packet_sequence_number,
	c_bitstream* packet,
	int32 maximum_number_of_requests,
	s_replication_incoming_request* requests,
	int32* out_number_of_requests)
{
	return INVOKE_TYPE(0x1D1C46, 0x1D6ED2, e_network_read_result(__thiscall*)(c_replication_entity_manager_view*, c_bitstream*, int32, void*, int32*),
		this, packet, maximum_number_of_requests, requests, out_number_of_requests);
}

void c_replication_entity_manager_view::process_incoming_request(
	void* request)
{
	INVOKE_TYPE(0x1D29BA, 0x1D7C46, void(__thiscall*)(c_replication_entity_manager_view*, void*),
		this, request);
	return;
}

void c_replication_entity_manager_view::notify_packet_acknowledged(
	void)
{
	// empty function
	return;
}

void c_replication_entity_manager_view::mark_packet_delivered(
	bool delivered)
{
	INVOKE_TYPE(0x1D236B, 0x1D75F7, void(__thiscall*)(c_replication_entity_manager_view*, bool), this, delivered);
	return;
}

void c_replication_entity_manager_view::initialize(
	int32 view_index,
	class c_replication_entity_manager* entity_manager)
{
	ASSERT(!m_initialized);
	ASSERT(view_index>=0 && view_index<k_short_bits);

	event(_event_status, "networking:replication:entity:[%d] entity view allocated", view_index);

	m_entity_manager = entity_manager;
	m_view_index = view_index;
	m_view_mask = FLAG(view_index);
	m_packet_list = NULL;
	m_outgoing_packet = NULL;

	for (uint32 i = 0; i<k_replication_entity_manager_view_max_entities; ++i)
	{
		s_replication_entity_view_data* data = &m_entity_data[i];

		data->entity_index = NONE;
		data->update_mask = 0;
		data->state = 0;
		data->next_index = NONE;
		data->update_timestamp = 0;
		data->flags = 0;
	}

	m_current_absolute_index_position = NONE;
	m_statistics.creations_pending = 0;
	m_statistics.creations_sent = 0;
	m_statistics.deletions_pending = 0;
	m_statistics.deletions_sent = 0;
	m_statistics.updates_pending = 0;
	m_statistics.updates_sent = 0;
	m_replicating = false;
	m_fatal_error = false;
	m_initialized = true;

	return;
}

void c_replication_entity_manager_view::reset(
	void)
{
	INVOKE_TYPE(0x1D1718, 0x1D69A4, void(__thiscall*)(c_replication_entity_manager_view*), this);
	return;
}

void c_replication_entity_manager_view::create_entity(
	int32 entity_index)
{
	INVOKE_TYPE(0x1D258A, 0x1D7816, void(__thiscall*)(c_replication_entity_manager_view*, int32), this, entity_index);
	return;
}

void c_replication_entity_manager_view::set_entity_dirty(
	int32 entity_index,
	uint32 update_mask)
{
	s_replication_entity_view_data* view_entity = get_entity(entity_index);
	s_replication_entity_data const* entity = m_entity_manager->get_entity(entity_index);

	if (view_entity->state == 3 && !TEST_BIT(entity->deletion_mask, m_view_index) && view_entity->update_mask==0)
	{
		ASSERT(m_statistics.updates_pending <= k_replication_entity_table_length);
	}

	view_entity->update_mask |= update_mask;

	return;
}

void c_replication_entity_manager_view::mark_entity_for_deletion(
	int32 entity_index)
{
	INVOKE_TYPE(0x1D2599, 0x0, void(__thiscall*)(c_replication_entity_manager_view*, int32), this, entity_index);

	return;
}

bool c_replication_entity_manager_view::entity_is_active(
	int32 entity_index) const
{
	int32 absolute_index = ENTITY_INDEX_TO_ABSOLUTE_INDEX(entity_index);
	bool active = false;

	if (VALID_INDEX(absolute_index, NUMBEROF(m_entity_data)) &&
		m_entity_data[absolute_index].entity_index == entity_index)
	{
		ASSERT(m_entity_manager);
		ASSERT(TEST_BIT(m_entity_manager->m_entity_data[absolute_index].flags, _replication_entity_allocated_flag));
		ASSERT(m_entity_manager->m_entity_data[absolute_index].seed==ENTITY_INDEX_TO_SEED(entity_index));

		active =
			!m_entity_manager->is_entity_local(entity_index) ||
			m_entity_data[absolute_index].state == 3 &&
			!m_entity_manager->is_entity_being_deleted(entity_index);
	}

	return active;
}

void c_replication_entity_manager_view::stop_replication(
	void)
{
	ASSERT(m_replicating);
	m_replicating = false;

	return;
}


/* private code */

/*
bool c_replication_entity_manager_view::write_creation_to_packet(
	int32 request_identifier,
	void const* telemetry_data,
	class c_bitstream* packet,
	int32 must_leave_space_bits)
{

	uint32 update_mask_written;
	bool write_success;
	int32 entity_index;
	s_replication_entity_data const* entity;
	int32 absolute_index;

	bool wrote_creation = false;

	ASSERT(packet);
	ASSERT(m_outgoing_packet);
	ASSERT(absolute_index>=0 && absolute_index<k_replication_entity_table_length);



	ASSERT(entity_index != NONE);
	ASSERT(m_entity_data[absolute_index].state==_replication_entity_view_state_ready);
	ASSERT(entity->deletion_mask==0);

	return wrote_creation;
}
*/

s_replication_entity_view_data* c_replication_entity_manager_view::get_entity(int32 entity_index)
{
	int32 absolute_index = ENTITY_INDEX_TO_ABSOLUTE_INDEX(entity_index);

	ASSERT(m_entity_data[absolute_index].entity_index==entity_index);
	ASSERT(TEST_BIT(m_entity_manager->m_entity_data[absolute_index].flags, _replication_entity_allocated_flag));
	ASSERT(m_entity_manager->m_entity_data[absolute_index].seed == ENTITY_INDEX_TO_SEED(entity_index));

	return &m_entity_data[absolute_index];
}
