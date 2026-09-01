#include "stdafx.h"
#include "simulation_entity_definition.h"

#include "memory/bitstream.h"
#include "simulation_type_collection.h"

/* public code */

bool c_entity_update_encode_helper::make_room_for_update(
	class c_bitstream* bitstream,
	int32 required_leave_space_bits,
	int32 update_component_first_index,
	int32 update_component_count,
	uint32 update_mask)
{
	ASSERT(bitstream);
	ASSERT(required_leave_space_bits>=0);
	ASSERT(update_component_first_index>=0);
	ASSERT(update_component_count>0);
	ASSERT(update_component_first_index+update_component_count<=k_simulation_entity_maximum_update_flag_count);
	ASSERT((update_mask & ~(MASK(update_component_count) << update_component_first_index))==0);

	ASSERT(m_bitstream==NULL);

	m_update_component_first_index = update_component_first_index;
	m_required_leave_space_bits = required_leave_space_bits;
	m_bitstream = bitstream;
	m_update_component_count = update_component_count;
	m_update_mask = update_mask;
	m_current_leave_space_bits = update_component_count+required_leave_space_bits;
	m_update_considered_mask = 0;
	m_update_written_mask = 0;
	m_update_skipped_mask = 0;
	m_update_overflowed_mask = 0;
	m_current_update_component_index = NONE;
	m_current_update_component_name = NULL;
	m_able_to_write_update = bitstream->get_space_left_in_bits() >= m_current_leave_space_bits;

	return m_able_to_write_update;
}

void c_entity_update_encode_helper::finish_update(
	uint32* update_mask_written)
{
	ASSERT(m_bitstream!=NULL);
	ASSERT(m_current_update_component_index==NONE);
	ASSERT(m_able_to_write_update);

	ASSERT(update_mask_written);

	ASSERT(m_current_leave_space_bits>=m_required_leave_space_bits);
	ASSERT(m_bitstream->get_space_left_in_bits()>=m_current_leave_space_bits);


	// Ensure that the written skipped and overflowed masks are in sync with the current update mask
	ASSERT((m_update_written_mask|m_update_skipped_mask|m_update_overflowed_mask)==m_update_mask);



	ASSERT((*update_mask_written & m_update_written_mask) == 0);
	ASSERT((*update_mask_written & (MASK(m_update_component_count) << m_update_component_first_index)) == 0);

	*update_mask_written |= m_update_written_mask;

	return;
}

bool c_entity_update_encode_helper::write_component_header(
	int32 update_component_index,
	char const* update_component_name)
{
	bool write_component = false;

	ASSERT(m_bitstream!=NULL);
	ASSERT(m_current_update_component_index==NONE);

	ASSERT(update_component_index>=m_update_component_first_index);
	ASSERT(update_component_index<m_update_component_first_index+m_update_component_count);



	
	
	
	ASSERT(m_bitstream->get_space_left_in_bits()>=m_current_leave_space_bits);

	// Set passed component name and index
	m_current_update_component_name = update_component_name;
	m_current_update_component_index = update_component_index;

	ASSERT(!TEST_BIT(m_update_considered_mask, m_current_update_component_index));

	SET_BIT(m_update_considered_mask, m_current_update_component_index, true);

	m_bitstream->push_position();

	if (TEST_BIT(m_update_mask, m_current_update_component_index))
	{
		m_bitstream->write_bool(m_current_update_component_name, true);
		SET_BIT(m_update_written_mask, m_current_update_component_index, true);
		write_component = true;
	}

	return write_component;
}

void c_entity_update_encode_helper::skip_component(
	void)
{
	ASSERT(m_bitstream!=NULL);
	ASSERT(m_current_update_component_index>=m_update_component_first_index);
	ASSERT(m_current_update_component_index<m_update_component_first_index+m_update_component_count);


	ASSERT(TEST_BIT(m_update_written_mask, m_current_update_component_index));
	ASSERT(!TEST_BIT(m_update_skipped_mask, m_current_update_component_index));

	SET_BIT(m_update_skipped_mask, m_current_update_component_index, true);

	return;
}

void c_entity_update_encode_helper::finish_component(
	void)
{
	bool undo_component = false;

	ASSERT(m_bitstream!=NULL);
	ASSERT(m_current_update_component_index>=m_update_component_first_index);
	ASSERT(m_current_update_component_index<m_update_component_first_index+m_update_component_count);

	if (TEST_BIT(m_update_written_mask, m_current_update_component_index))
	{
		if (TEST_BIT(m_update_skipped_mask, m_current_update_component_index))
		{
			undo_component = true;
		}
		else if (m_bitstream->get_space_left_in_bits()<m_current_leave_space_bits)
		{
			SET_BIT(m_update_overflowed_mask, m_current_update_component_index, true);
			undo_component = true;
		}
	}

	if (undo_component)
	{
		SET_BIT(m_update_written_mask, m_current_update_component_index, false);
		m_bitstream->pop_position(true);
	}
	else
	{
		m_bitstream->pop_position(false);
	}

	if (!TEST_BIT(m_update_written_mask, m_current_update_component_index))
	{
		m_bitstream->write_bool(m_current_update_component_name, false);
	}

	ASSERT(m_current_leave_space_bits>=m_required_leave_space_bits);


	ASSERT(m_bitstream->get_space_left_in_bits()>=m_current_leave_space_bits);

	m_current_update_component_name = NULL;
	m_current_update_component_index = NONE;

	return;
}
