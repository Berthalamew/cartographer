#pragma once

/* enums */

enum e_network_memory_block : int16
{
	_network_memory_block_message_outgoing_fragment = 0,
	_network_memory_block_message_incoming_fragment,
	_network_memory_block_replication_entity_status_record,
	_network_memory_block_replication_entity_update_record,
	_network_memory_block_replication_entity_packet_record,
	_network_memory_block_replication_event_header,
	_network_memory_block_replication_event_payload,
	_network_memory_block_replication_event_record,
	_network_memory_block_replication_event_packet_record,
	_network_memory_block_replication_control_motion,
	_network_memory_block_replication_control_prediction,
	_network_memory_block_simulation_synchronous_client_update,
	_network_memory_block_simulation_entity_creation,
	_network_memory_block_simulation_entity_state,
	_network_memory_block_simulation_event,
	_network_memory_block_logic_session_array,
	_network_memory_block_logic_unsuitable_session_array,
	_network_memory_block_join_request,

	// Added block typs
	_network_memory_block_forward_simulation_queue_element,
	_network_memory_block_forward_gamestate_element,

	k_network_memory_block_count,
};

/* structures */

struct s_network_heap_stats
{
	int32 allocations;
	int32 allocations_in_bytes;
};

/* classes */

class c_network_heap
{
public:
	class c_fixed_memory_rockall_frontend* rockall_frontend;
	int32 get_block_size(const void* block) const;

	void dispose();
};

/* prototypes */

void network_memory_apply_patches(void);

c_network_heap* network_get_heap(void);

void network_memory_verify(void);

s_network_heap_stats* network_heap_get_description(void);

class c_network_channel* network_memory_get_channel(int32 channel_index);

bool __cdecl network_memory_base_initialize(
	class c_network_link** link,
	class c_network_message_type_collection** message_types,
	class c_network_message_gateway** message_gateway,
	class c_network_message_handler** message_handler,
	class c_network_observer** observer,
	class c_network_session** sessions,
	class c_network_session_manager** session_manager,
	class c_network_text_chat_manager** text_chat_manager);

bool __cdecl network_memory_simulation_initialize(
	class c_simulation_world** world,
	class c_simulation_watcher** watcher,
	class c_simulation_type_collection** type_collection);

uint8* __cdecl network_heap_allocate_block(uint32 size);

void __cdecl network_heap_free_block(void* block);


class c_simulation_distributed_world* network_allocate_simulation_distributed_world(void);

char* network_heap_describe(char* buffer, int32 size);

