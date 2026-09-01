#include "stdafx.h"
#include "replication_entity_manager.h"

#include "replication_entity_manager_view.h"
#include "networking/network_event.h"
#include "simulation/simulation_entity_database.h"

CLASS_HOOK_DECLARE_LABEL(c_replication_entity_manager__write_creation_to_packet, c_replication_entity_manager::write_creation_to_packet);
static __declspec(naked) void jmp_c_replication_entity_manager__write_creation_to_packet(void)
{
	CLASS_HOOK_JMP(c_replication_entity_manager__write_creation_to_packet, c_replication_entity_manager::write_creation_to_packet);
}

/* public code */

void replication_entity_manager_apply_patches(
	void)
{
	PatchCall(Memory::GetAddress(0x1D2677, 0x0), jmp_c_replication_entity_manager__write_creation_to_packet);
	PatchCall(Memory::GetAddress(0x1D27ED, 0x0), jmp_c_replication_entity_manager__write_creation_to_packet);

	return;
}

void c_replication_entity_manager::initialize(
	void)
{
	m_client = NULL;
	m_view_mask = 0;

	for (uint32 i = 0; i < NUMBEROF(m_views); i++)
	{
		m_views[i] = NULL;
	}

	reset();

	return;
}

void c_replication_entity_manager::destroy(
	void)
{
	ASSERT(m_view_mask==0);

	for (int32 view_index= 0; view_index <NUMBEROF(m_views); ++view_index)
	{
		ASSERT(m_views[view_index]==NULL);
	}

	return;
}

void c_replication_entity_manager::reset(
	void)
{
	for (uint32 i = 0; i < NUMBEROF(m_views); i++)
	{
		if (m_views[i] != NULL)
		{
			m_views[i]->reset();
		}
	}

	csmemset(m_entity_data, 0, sizeof(m_entity_data));
	m_entity_creation_start_position = 0;

	return;
}



void c_replication_entity_manager::attach_client(
	class c_simulation_entity_database* client)
{
	ASSERT(client);
	ASSERT(m_client==NULL);
	ASSERT(m_view_mask==0);

	m_client = client;

	return;
}

void c_replication_entity_manager::detach_client(
	c_simulation_entity_database* client)
{
	ASSERT(client==m_client);
	m_client = NULL;

	return;
}

int32 c_replication_entity_manager::create_local_entity(
	void)
{
	int32 new_entity_index = NONE;

	ASSERT(m_client != NULL);

	int32 new_absolute_index = preallocate_entity();

	if (new_absolute_index!=NONE)
	{
		new_entity_index = create_local_entity_internal(new_absolute_index);

		event(_event_status, "replication:entity: local entity created %lx", new_entity_index);

	}
	else
	{
		event(_event_error, "replication:entity: unable to create local entity, table is full");
	}

	return new_entity_index;
}

void c_replication_entity_manager::delete_local_entity(
	int32 entity_index)
{
	s_replication_entity_data* entity= get_entity(entity_index);
	
	ASSERT(TEST_BIT(entity->flags, _replication_entity_local_flag));
	ASSERT(TEST_BIT(entity->flags, _replication_entity_allocated_flag));
	ASSERT(!TEST_BIT(entity->flags, _replication_entity_marked_for_deletion_flag));
	ASSERT(entity->deletion_mask==0);

	SET_BIT(entity->flags, _replication_entity_marked_for_deletion_flag, true);

	event(_event_status, "networking:replication:entity: local entity marked for deletion %lx", entity_index);
	
	ASSERT(m_client);

	m_client->notify_mark_entity_for_deletion(entity_index /*, false*/);

	for (int32 view_index= 0; view_index<NUMBEROF(m_views); ++view_index)
	{
		if (TEST_BIT(m_view_mask, view_index))
		{
			ASSERT(m_views[view_index]!=NULL);

			m_views[view_index]->mark_entity_for_deletion(entity_index);
		}
	}

	if (entity->deletion_mask==0)
	{
		delete_entity_internal(entity_index);
	}

	return;
}

void c_replication_entity_manager::set_entity_dirty(
	int32 entity_index,
	uint32 update_mask)
{
	s_replication_entity_data const* entity = get_entity(entity_index);
	
	ASSERT(update_mask!=0);
	ASSERT(TEST_BIT(entity->flags, _replication_entity_local_flag));
	ASSERT(!TEST_BIT(entity->flags, _replication_entity_marked_for_deletion_flag));

	for (int32 view_index= 0; view_index<NUMBEROF(m_views); ++view_index)
	{
		if (m_views[view_index])
		{
			m_views[view_index]->set_entity_dirty(entity_index, update_mask);
		}
	}

	return;
}

/* private code */

int32 c_replication_entity_manager::preallocate_entity(
	void)
{
	int32 new_entity_index = NONE;

	for (int32 entity_index = m_entity_creation_start_position; entity_index<NUMBEROF(m_entity_data); ++entity_index)
	{
		if (!TEST_BIT(m_entity_data[entity_index].flags, _replication_entity_allocated_flag))
		{
			new_entity_index = entity_index;
			break;
		}
	}
	
	if (new_entity_index == NONE)
	{
		for (int32 entity_index = 0; entity_index < m_entity_creation_start_position; ++entity_index)
		{
			if (!TEST_BIT(m_entity_data[entity_index].flags, _replication_entity_allocated_flag))
			{
				new_entity_index = entity_index;
				break;
			}
		}
	}

	if (new_entity_index != NONE)
	{
		m_entity_data[new_entity_index].flags = FLAG(_replication_entity_allocated_flag);
		m_entity_creation_start_position = (new_entity_index+1) % NUMBEROF(m_entity_data);
	}

	return new_entity_index;
}

bool c_replication_entity_manager::write_creation_to_packet(
	int32 entity_index,
	uint32 update_mask,
	void const* telemetry_data,
	class c_bitstream* packet,
	int32 must_leave_space_bits,
	uint32* out_update_mask)
{
	s_replication_entity_data* entity = get_entity(entity_index);

	ASSERT(m_client != NULL);
	ASSERT(TEST_BIT(entity->flags, _replication_entity_allocated_flag));

	return m_client->write_creation_to_packet(entity_index, update_mask, telemetry_data, packet, must_leave_space_bits, out_update_mask);
}

int32 c_replication_entity_manager::create_local_entity_internal(
	int32 absolute_index)
{
	int32 entity_index;

	ASSERT(absolute_index>=0 && absolute_index<NUMBEROF(m_entity_data));
	ASSERT(m_entity_data[absolute_index].flags==FLAG(_replication_entity_allocated_flag));

	SET_BIT(m_entity_data[absolute_index].flags, _replication_entity_local_flag, true);
	
	s_replication_entity_data* entity = &m_entity_data[absolute_index];

	entity->field_4 = (uint32)NONE;
	entity->seed = (entity->seed+1) % 16;

	entity_index = ENTITY_INDEX_NEW(absolute_index, entity->seed);

	for (int32 view_index = 0; view_index < NUMBEROF(m_views); ++view_index)
	{
		if (m_views[view_index])
		{
			m_views[view_index]->create_entity(entity_index);
		}
	}

	return entity_index;
}


void c_replication_entity_manager::delete_entity_internal(
	int32 entity_index)
{
	s_replication_entity_data* entity = get_entity(entity_index);

	ASSERT(m_client!=NULL);
	ASSERT(TEST_BIT(entity->flags, _replication_entity_allocated_flag));
	ASSERT(TEST_BIT(entity->flags, _replication_entity_marked_for_deletion_flag));
	ASSERT(entity->deletion_mask==0);

	m_client->notify_delete_entity(entity_index);
	SET_BIT(entity->flags, _replication_entity_allocated_flag, false);

	event(_event_status, "networking:replication:entity: entity deleted %lx", entity_index);

	return;
}
