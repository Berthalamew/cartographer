#pragma once

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

uint8* __cdecl network_heap_allocate_block(uint32 size);

void __cdecl network_heap_free_block(void* block);

char* network_heap_describe(char* buffer, int32 size);
