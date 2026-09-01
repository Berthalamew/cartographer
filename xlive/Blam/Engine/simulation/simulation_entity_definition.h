#pragma once

/* structures */

struct s_entity_update_data
{
	uint32 update_mask;
	uint32 update_timestamp;
	struct s_simulation_view_telemetry_data const* telemetry;
};

/* classes */

class c_entity_update_encode_helper
{
public:
	c_entity_update_encode_helper(void) : m_bitstream(NULL)
	{
		return;
	}

	~c_entity_update_encode_helper(void)
	{
		return;
	}

	bool make_room_for_update(class c_bitstream* bitstream, int32 required_leave_space_bits, int32 update_component_first_index, int32 update_component_count, uint32 update_mask);
	void finish_update(uint32* update_mask_written);
	bool write_component_header(int32 update_component_index, char const* update_component_name);
	void skip_component(void);
	void finish_component(void);

private:
	class c_bitstream *m_bitstream;
	int32 m_update_component_first_index;
	int32 m_update_component_count;
	uint32 m_update_mask;
	uint32 m_update_considered_mask;
	uint32 m_update_written_mask;
	uint32 m_update_skipped_mask;
	uint32 m_update_overflowed_mask;
	bool m_able_to_write_update;
	int32 m_current_update_component_index;
	const char* m_current_update_component_name;
	int32 m_required_leave_space_bits;
	int32 m_current_leave_space_bits;
};
