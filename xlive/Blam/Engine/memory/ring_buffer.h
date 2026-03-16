#pragma once

/* classes */

class c_ring_buffer
{

private:
	int32 ring_size;
	uint8* storage;
	int32 start_offset;
	int32 data_size;
};

class c_ring_stream
{
public:
	void detach(void);
	int32 add_block(int32 block_data_size, void const* block_data);
	void remove_block(int32 block_data_size, void* out_block_data);

	bool attached(
		void) const
	{
		return start_offset!=NULL;
	}

private:
	int32 ring_size;
	uint8* storage;
	int32 start_offset;
	int32 data_size;
};
