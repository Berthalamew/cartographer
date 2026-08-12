#pragma once
#include "data_reference.h"

/* structures */

struct tag_reference
{
	tag_group group;
	datum index;
};
ASSERT_STRUCT_SIZE(tag_reference, 8);

/* public code */

inline void* tag_data_get_address(
	data_reference const* data)
{
	ASSERT(data);

	void* result = NULL;
	
	if (data->data != NONE)
	{
		result = (void*)(data->data + *Memory::GetAddress<uintptr_t*>(0x482290, 0x4A6438));
	}
	
	return result;
}

inline void* tag_block_get_address(
	s_tag_block const* block)
{
	ASSERT(block);

	void* result = NULL;

	if (block->data != NONE)
	{
		result = (void*)(block->data + *Memory::GetAddress<uintptr_t*>(0x482290, 0x4A6438));
	}

	return result;
}
