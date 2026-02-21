#include "stdafx.h"
#include "simulation_queue.h"

#include "simulation_queue_global_events.h"

#include "memory/bitstream.h"
#include "networking/network_event.h"
#include "networking/network_memory.h"

void c_simulation_queue::allocate(int32 size, s_simulation_queue_element** element_out)
{
	ASSERT(size > 0);
	ASSERT(element_out != NULL);

	*element_out = NULL;
	if (initialized())
	{
		uint32 required_data_size = sizeof(s_simulation_queue_element) + size;

		if (allocated_count() + 1 < k_simulation_queue_count_max)
		{
			if (allocated_size_in_bytes() + required_data_size < k_simulation_queue_size_max)
			{
				if (size < k_simulation_queue_element_data_size_max)
				{
					if (allocated_new_encoded_size_bytes(size) < k_simulation_queue_max_encoded_size)
					{
						uint8* net_heap_block = network_heap_allocate_block(required_data_size);
						if (net_heap_block)
						{
							csmemset(net_heap_block, 0, required_data_size);
							s_simulation_queue_element* allocated_elem = (s_simulation_queue_element*)net_heap_block;
							allocated_elem->type = _simulation_queue_element_type_none;
							allocated_elem->data = net_heap_block + sizeof(s_simulation_queue_element);
							allocated_elem->data_size = size;
							allocated_elem->next = NULL;

							++m_allocated_count;
							m_allocated_size += required_data_size;
							*element_out = allocated_elem;
						}
						else
						{
#ifdef EVENTS_ENABLED
							char description[1024];
							csstrncpy(description, "UNKNOWN", NUMBEROF(description));
							network_heap_describe(description, NUMBEROF(description));
							event(
								_event_fatal,
								"networking:simulation_queue: OUT OF MEMORY requesting allocation for %d bytes [%s]",
								size,
								description
							);
#endif
						}
					}
					else
					{
						event(
							_event_fatal,
							"networking:simulation_queue: can't allocate element, would exceed encoded size maximum [0x%08X/0x%8X]",
							allocated_new_encoded_size_bytes(size),
							k_simulation_queue_max_encoded_size
						);
					}
				}
				else
				{
					event(
						_event_fatal,
						"networking:simulation_queue: can't allocate element, would exceed element size maximum [0x%08X/0x%8X]",
						size,
						k_simulation_queue_element_data_size_max
					);
				}
			}
			else
			{
				event(
					_event_fatal,
					"networking:simulation_queue: can't allocate element, would exceed size maximum [0x%08X/0x%8X]",
					size,
					131072
				);
			}
		}
		else
		{
			event(
				_event_fatal,
				"networking:simulation_queue: can't allocate element, would exceed count maximum [max 0x%8X]",
				k_simulation_queue_count_max
			);
		}
	}
}

// transefrs data from the source into ours
void c_simulation_queue::transfer_elements(c_simulation_queue* simulation_queue)
{
	bool cutoff = false;

	ASSERT(simulation_queue);
	ASSERT(queued_count() == 0);
	ASSERT(queued_size_in_bytes() == 0);
	//ASSERT(simulation_queue->queued_count() > 0);
	//ASSERT(simulation_queue->queued_size_in_bytes() > 0);

	const int32 count = simulation_queue->queued_count();
	for (int32 i = 0; i < count; i++)
	{
		s_simulation_queue_element* element = NULL;
		simulation_queue->deque(&element);
		--simulation_queue->m_allocated_count;
		simulation_queue->m_allocated_size -= get_element_size_in_bytes(element);
		++m_allocated_count;
		m_allocated_size += get_element_size_in_bytes(element);
		enqueue(element);

		// if the element type is a global event
		// check if we need to cut the update here
		if (element->type == _simulation_queue_element_type_game_global_event
			&& simulation_queue_game_global_event_requires_cutoff(element))
		{
			// cut the queue here, keep the rest for the next update
			cutoff = true;
			break;
		}
	}

	if (!cutoff)
	{
		ASSERT(simulation_queue->queued_count() == 0);
		ASSERT(simulation_queue->queued_size_in_bytes() == 0);
	}

	//ASSERT(queued_count() > 0);
	//ASSERT(queued_size_in_bytes() > 0);
	ASSERT(simulation_queue->queued_count() >= 0);
	ASSERT(simulation_queue->queued_size_in_bytes() >= 0);
	ASSERT(simulation_queue->allocated_count() >= 0);
	ASSERT(simulation_queue->allocated_size_in_bytes() >= 0);

	if (simulation_queue->queued_count() == 0)
	{
		ASSERT(simulation_queue->queued_size_in_bytes() == 0);
	}

	m_initialized = true;

	return;
}

void c_simulation_queue::deallocate(s_simulation_queue_element* element)
{
	ASSERT(element);

	if (initialized())
	{
		ASSERT(element->data);
		ASSERT(element->data_size > 0 && element->data_size < k_simulation_queue_element_data_size_max);
		ASSERT(element->next == NULL);

		m_allocated_size -= get_element_size_in_bytes(element);
		--m_allocated_count;

		ASSERT(allocated_size_in_bytes() >= 0);
		ASSERT(allocated_count() >= 0);

		//network_heap_verify_block(element);
		network_heap_free_block(element);
	}

	return;
}

void c_simulation_queue::enqueue(s_simulation_queue_element* element)
{
	ASSERT(element);
	ASSERT(element->data);
	ASSERT(element->data_size > 0);
	ASSERT(element->next == NULL);
	ASSERT(m_queued_count >= 0);
	ASSERT(queued_count() < k_simulation_queue_count_max);
	ASSERT(queued_size_in_bytes() < k_simulation_queue_size_max);

	if (initialized())
	{

		if (m_tail)
		{
			//network_heap_verify_block(element);
			m_tail->next = element;
		}
		else
		{
			ASSERT(m_head == NULL);
			ASSERT(m_queued_count == 0);

			m_head = element;
		}

		m_tail = element;
		++m_queued_count;
		m_size += get_element_size_in_bytes(element);

		ASSERT(queued_count() < k_simulation_queue_count_max);
		ASSERT(queued_size_in_bytes() < k_simulation_queue_size_max);
		ASSERT(queued_encoded_size_in_bytes() < k_simulation_queue_max_encoded_size);
	}
	return;
}

void c_simulation_queue::deque(s_simulation_queue_element** element_out)
{
	ASSERT(element_out);

	*element_out = NULL;
	if (initialized())
	{
		if (m_head)
		{
			ASSERT(m_queued_count > 0);
			ASSERT(m_size > 0);

			*element_out = m_head;
			m_head = m_head->next;

			--m_queued_count;
			m_size -= get_element_size_in_bytes(*element_out);
			
			(*element_out)->next = NULL;
		}

		if (m_head == NULL)
		{
			m_tail = NULL;
		}
	}
	return;
}

void c_simulation_queue::clear()
{
	if (initialized())
	{
		while (get_head() != NULL)
		{
			s_simulation_queue_element* element_to_dequeue = NULL;
			deque(&element_to_dequeue);
			ASSERT(element_to_dequeue);
			deallocate(element_to_dequeue);
			// ### TODO clear allocated but not queued too??
		}

		ASSERT(m_queued_count == 0);
		ASSERT(m_allocated_count == 0);
		ASSERT(m_size == 0);
	}
	return;
}

void c_simulation_queue::dispose()
{
	if (initialized())
	{
		clear();
		m_initialized = false;
	}
}

bool c_simulation_queue::decode(c_bitstream* bitstream)
{
	bool result = true;

	ASSERT(bitstream);
	//ASSERT(!m_initialized);	FIXME

	initialize();
	
	const int32 count = bitstream->read_integer("queue-count", 12);
	const int32 size = bitstream->read_integer("queue-size", 17);
	
	if (VALID_INDEX(count, k_simulation_queue_count_max))
	{
		if (VALID_INDEX(size, k_simulation_queue_size_max))
		{
			for (int32 i = 0; i < count; ++i)
			{
				const e_event_queue_type element_type = (e_event_queue_type)bitstream->read_integer("type", k_simulation_queue_type_encoded_size_in_bits);
				const int32 element_size = bitstream->read_integer("size", 10);

				s_simulation_queue_element* element = NULL;
				if (VALID_INDEX(element_type, k_simulation_queue_element_type_count))
				{
					allocate(element_size, &element);
					if (element)
					{
						element->type = element_type;
						bitstream->read_raw_data("data", element->data, SIZEOF_BITS(element_size));
						enqueue(element);
					}
					else
					{
						event(
							_event_fatal,
							"networking:simulation:queue: failed to allocate element %d/%d while decoding for queue type %d size %d",
							i,
							count,
							element_type,
							element_size
						);
						result = false;
					}
				}
				else
				{
					event(
						_event_error,
						"networking:simulation:queue: bad element type %d during decode (size %d)",
						element_type,
						element_size
					);
					result = false;
				}

				// If error stop iterating through elements
				if (!result)
				{
					break;
				}
			}

			// Verify counts match
			if (count != m_queued_count)
			{
				event(
					_event_error,
					"networking:simulation:queue: queue count mismatch during decode %d!=%d",
					count,
					m_queued_count
				);
				result = false;
			}

			// Verify sizes match
			if (size != m_size)
			{
				event(
					_event_error,
					"networking:simulation:queue: size mismatch during decode %d!=%d",
					size,
					m_size
				);
				result = false;
			}
		}
		else
		{
			event(_event_error, "networking:simulation:queue: bad queue size %d during decode", size);
			result = false;
		}
	}
	else
	{
		event(_event_error, "networking:simulation:queue: bad queue count %d during decode", count);
		result = false;
	}


	return result;
}

void c_simulation_queue::encode(c_bitstream* bitstream)
{
	ASSERT(bitstream);
	ASSERT(m_initialized);
	ASSERT(m_allocated_count == m_queued_count);

	bitstream->write_integer("queue-count", m_queued_count, 12);
	bitstream->write_integer("queue-count", m_size, 17);

	s_simulation_queue_element* element = NULL;
	if (m_initialized && m_queued_count>0)
	{
		ASSERT(m_head);

		//network_heap_verify_block(m_head);
		element = m_head;
	}

	for (int32 element_index = 0; element; ++element_index)
	{
		ASSERT(element_index < m_queued_count);
		ASSERT(element->type!=_simulation_queue_element_type_none);
		ASSERT(VALID_INDEX(element->type, k_simulation_queue_element_type_count));
		ASSERT(element->data_size > 0);

		bitstream->write_integer("type", element->type, k_simulation_queue_type_encoded_size_in_bits);
		bitstream->write_integer("size", element->data_size, 10);
		bitstream->write_raw_data("data", element->data, SIZEOF_BITS(element->data_size));

		s_simulation_queue_element* next = NULL;
		if (m_initialized)
		{
			ASSERT(queued_count() > 0);
			next = element->next;
			if (element->next)
			{
				//network_heap_verify_block(element->next);
			}
		}
		element = next;
	}

	return;
}
