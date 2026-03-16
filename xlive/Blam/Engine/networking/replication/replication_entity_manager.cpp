#include "stdafx.h"
#include "replication_entity_manager.h"

/* public code */

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
