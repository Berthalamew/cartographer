#include "stdafx.h"
#include "network_memory.h"

#include "memory/rockall_heap_manager.h"

#include "networking/delivery/network_channel.h"
#include "networking/delivery/network_link.h"
#include "networking/messages/network_message_gateway.h"
#include "networking/messages/network_message_handler.h"
#include "networking/messages/network_message_type_collection.h"
#include "networking/session/network_observer.h"
#include "networking/session/network_session_manager.h"
#include "networking/session/network_text_chat_manager.h"

/* typedefs */

typedef uint8* (__cdecl* t_network_heap_allocate_block)(uint32 size);
typedef void(__cdecl* t_network_heap_free_block)(void* block);

/* structures */

struct s_network_shared_memory_globals
{
	int32 current_configuration;
	int32 physical_memory_size;
	void* physical_memory_low_address;
	void* physical_memory_high_address;
	int32 maximum_channel_count;
	c_network_channel* channels;
	void* connection_array;
	void* queue_array;
	int32 channel_count;
	bool distributed_simulation_available;
	c_simulation_distributed_world* simulation_distributed_world;
	struct data_array* simulation_view_data_array;
	struct data_array* simulation_distributed_view_data_array;
	int32 heap_size;
	void* heap;
	void* heap_buffer;
};

/* prototypes */

static s_network_shared_memory_globals* network_shared_memory_globals_get(void);

CLASS_HOOK_DECLARE_LABEL(c_network_heap__dispose, c_network_heap::dispose);
static __declspec(naked) void jmp_c_network_heap__discard(void)
{
	CLASS_HOOK_JMP(c_network_heap__dispose, c_network_heap::dispose);
}

/* globals */

static t_network_heap_allocate_block p_network_heap_allocate_block;
static t_network_heap_free_block p_network_heap_free_block;

static s_network_heap_stats g_network_heap_allocations;

/* public code */

void network_memory_apply_patches(void)
{
	// hook the heap allocator globally, to get a picture of the network heap usage
	DETOUR_ATTACH(p_network_heap_allocate_block, Memory::GetAddress<t_network_heap_allocate_block>(0x1AC939, 0x1ACB07), network_heap_allocate_block);
	DETOUR_ATTACH(p_network_heap_free_block, Memory::GetAddress<t_network_heap_free_block>(0x1AC94A, 0x1ACB18), network_heap_free_block);
	PatchCall(Memory::GetAddress(0x1AD292, 0x1AD460), jmp_c_network_heap__discard);

	// increase network_shared_memory_globals.maximum_channel_count for campaign
	WriteValue<uint32>(Memory::GetAddress(0x1ACCDB + 1), k_network_maximum_observers_for_campaign);	
	return;
}

c_network_heap* network_get_heap(void)
{
	return *Memory::GetAddress<c_network_heap**>(0x4FADE0, 0x525298);
}

int32 c_network_heap::get_block_size(const void* block) const
{
	int32 size;
	if (!rockall_frontend->Details(block, &size))
	{
		size = -1;
	}

	return size;
}

void c_network_heap::dispose(void)
{
	g_network_heap_allocations.allocations = 0;
	g_network_heap_allocations.allocations_in_bytes = 0;
	return INVOKE_TYPE(0x381574, 0x32CCAE, void(__thiscall*)(c_network_heap*), this);
}


void network_memory_verify(
	void)
{
	// TODO: implement
	return;
}

s_network_heap_stats* network_heap_get_description(void)
{
	return &g_network_heap_allocations;
}

c_network_channel* network_memory_get_channel(
	int32 channel_index)
{
	c_network_channel* network_channels = *Memory::GetAddress<c_network_channel**>(0x4FADBC, 0x525274);
	return &network_channels[channel_index];
}

bool __cdecl network_memory_base_initialize(
	c_network_link** link,
	c_network_message_type_collection** message_types,
	c_network_message_gateway** message_gateway,
	c_network_message_handler** message_handler,
	c_network_observer** observer,
	c_network_session** sessions,
	c_network_session_manager** session_manager,
	c_network_text_chat_manager** text_chat_manager)
{
	INVOKE(0x1AC71B, 0x1AC8E9, network_memory_base_initialize,
		link,
		message_types,
		message_gateway,
		message_handler,
		observer,
		sessions,
		session_manager,
		text_chat_manager);

	*message_types = &g_network_message_types_mem;
	return true;
}

bool __cdecl network_memory_simulation_initialize(
	c_simulation_world** world,
	c_simulation_watcher** watcher,
	c_simulation_type_collection** type_collection)
{
	return INVOKE(0x1AC76F, 0x1AC93D, network_memory_simulation_initialize, world, watcher, type_collection);
}

uint8* __cdecl network_heap_allocate_block(uint32 size)
{
	//uint8* block = INVOKE(0x1AC939, 0x1ACB07, network_heap_allocate_block, size);
	uint8* block = p_network_heap_allocate_block(size);

	if (block)
	{
		g_network_heap_allocations.allocations++;
		g_network_heap_allocations.allocations_in_bytes += network_get_heap()->get_block_size(block);
	}

	return block;
}


void __cdecl network_heap_free_block(void* block)
{
	int32 block_size = network_get_heap()->get_block_size(block);

	if (block_size > 0)
	{
		g_network_heap_allocations.allocations--;
		g_network_heap_allocations.allocations_in_bytes -= block_size;
	}

	return p_network_heap_free_block(block);
	// return INVOKE(0x1AC94A, 0x1ACB18, network_heap_free_block, block);
}

c_simulation_distributed_world* network_allocate_simulation_distributed_world(
	void)
{
	s_network_shared_memory_globals* network_shared_memory_globals = network_shared_memory_globals_get();

	ASSERT(network_shared_memory_globals->simulation_distributed_world!=NULL);

	return network_shared_memory_globals->simulation_distributed_world;
}

char* network_heap_describe(char* buffer, int32 size)
{
	return "TODO: implement network_heap_describe";
}


/* private code */

static s_network_shared_memory_globals* network_shared_memory_globals_get(
	void)
{
	return Memory::GetAddress<s_network_shared_memory_globals*>(0x4FADA8, 0x0);
}
