#pragma once
#include "networking/network_constants.h"

class c_replication_event_manager
{
public:
	void initialize(class c_replication_entity_manager* entity_manager);
	void destroy(void);

	void reset(void);
	
	e_network_read_result read_incoming_event(
		int32 event_type,
		int32 const* entity_reference_indices,
		int32 maximum_block_count,
		int32* block_count,
		struct s_replication_allocation_block* blocks,
		class c_bitstream* packet);

	void register_client(
		class c_simulation_event_handler* client)
	{
		ASSERT(client!=NULL);
		ASSERT(m_client==NULL);

		m_client = client;

		return;
	}

	void deregister_client(
		class c_simulation_event_handler const* client)
	{
		ASSERT(client!=NULL);
		ASSERT(m_client==client);
		
		m_client = NULL;

		return;
	}

private:
	class c_replication_entity_manager* m_entity_manager;
	class c_simulation_event_handler* m_client;
	uint32 m_view_mask;
	class c_replication_event_manager_view* m_views[k_network_maximum_views_per_simulation];
	int32 m_outgoing_event_count;
	class c_replication_outgoing_event* m_outgoing_event_list;
};
ASSERT_STRUCT_SIZE(c_replication_event_manager, 84);
