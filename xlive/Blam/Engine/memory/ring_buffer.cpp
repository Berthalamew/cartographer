#include "stdafx.h"
#include "ring_buffer.h"

/* public code */

void c_ring_stream::detach(
	void)
{
	ASSERT(attached());
	start_offset = 0;
	storage = 0;

	return;
}

int32 c_ring_stream::add_block(
	int32 block_data_size,
	void const* block_data)
{
	return INVOKE_TYPE(0x381507, 0x0, int32(__thiscall*)(c_ring_stream*, int32, void const*), this, block_data_size, block_data);
}

void c_ring_stream::remove_block(
	int32 block_data_size,
	void* out_block_data)
{
	return INVOKE_TYPE(0x381546, 0x0, void(__thiscall*)(c_ring_stream*, uint32, void*), this, block_data_size, out_block_data);
}
