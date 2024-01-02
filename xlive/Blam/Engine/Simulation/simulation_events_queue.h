#pragma once

#include "Simulation/simulation.h"
#include "Networking/memory/networking_memory.h"

#define k_simulation_queue_count_max 4096
#define k_simulation_payload_size_max 1024
#define k_simulation_queue_avg_payload_size 32
#define k_simulation_queue_size_max (k_simulation_queue_count_max * k_simulation_queue_avg_payload_size)

#define k_simulation_queue_type_encoded_size_in_bits 4
#define k_simulation_queue_payload_encoded_size_in_bits 10
#define k_simulation_queue_header_encoded_size_in_bits (k_simulation_queue_type_encoded_size_in_bits + k_simulation_queue_payload_encoded_size_in_bits)

#define k_simulation_queue_max_encoded_size 59392

#define k_entity_reference_indices_count_max 2

enum e_event_queue_type : int16
{
	_simulation_queue_element_type_none,
	_simulation_queue_element_type_event = 1,
	_simulation_queue_element_type_entity_creation = 2,
	_simulation_queue_element_type_entity_update = 3,
	_simulation_queue_element_type_entity_deletion = 4,
	_simulation_queue_element_type_entity_promotion = 5,
	_simulation_queue_element_type_game_global_event = 6,
	_simulation_queue_element_type_player_event = 7,
	_simulation_queue_element_type_player_update_event = 8,
	_simulation_queue_element_type_gamestates_clear = 9,

	// probably unused, forge related
	_simulation_queue_element_type_sandbox_event = 10,

	k_simulation_queue_element_type_count,

	_simulation_queue_element_type_1 = FLAG(_simulation_queue_element_type_player_event) | FLAG(_simulation_queue_element_type_player_update_event) | FLAG(_simulation_queue_element_type_gamestates_clear),
	_simulation_queue_element_type_2 = FLAG(_simulation_queue_element_type_entity_deletion) | FLAG(_simulation_queue_element_type_entity_promotion) | FLAG(_simulation_queue_element_type_game_global_event),
	_simulation_queue_element_type_3 = (int16)0xffff & (int16)~(_simulation_queue_element_type_1 | _simulation_queue_element_type_2),
};

struct s_simulation_queue_element
{
	e_event_queue_type type;
	s_simulation_queue_element* next;
	uint8* data;
	uint32 data_size;
};

class c_simulation_queue
{
	bool	m_initialized;
	int32	m_allocated_count;
	int32	m_allocated_size_in_bytes;

	int32	m_queued_count;
	int32	m_queued_size;

	s_simulation_queue_element* m_head;
	s_simulation_queue_element* m_tail;

public:
	c_simulation_queue()
	{
		initialize();
	}

	~c_simulation_queue()
	{
		dispose(this);
	}

	void initialize()
	{
		m_allocated_count = 0;
		m_allocated_size_in_bytes = 0;
		m_queued_count = 0;
		m_queued_size = 0;
		m_head = NULL;
		m_tail = NULL;
		m_initialized = true;
	}

	bool initialized() const
	{
		return m_initialized;
	}

	const s_simulation_queue_element* get_head() const
	{
		if (initialized())
		{
			return m_head;
		}
		return NULL;
	}

	const s_simulation_queue_element* get_first_element() const
	{
		if (initialized())
		{
			return get_head();
		}
		return NULL;
	}

	const s_simulation_queue_element* get_next_element(const s_simulation_queue_element* element) const
	{
		if (initialized())
		{
			return element->next;
		}
		return NULL;
	}

	int32 allocated_count() const
	{
		if (initialized())
		{
			return m_allocated_count;
		}

		return 0;
	}

	int32 allocated_size_in_bytes() const
	{
		if (initialized())
		{
			return m_allocated_size_in_bytes;
		}

		return 0;
	}

	int32 get_element_size_in_bytes(s_simulation_queue_element* element) const
	{
		return element->data_size + sizeof(s_simulation_queue_element);
	}

	int32 allocated_encoded_size_bytes() const
	{
		// ### FIXME figure out + 36 in bits
		int32 size = (k_simulation_queue_header_encoded_size_in_bits * allocated_count() + 36) / 8;
		return size + allocated_size_in_bytes() - (sizeof(s_simulation_queue_element) * allocated_count());
	}

	int32 allocated_new_encoded_size_bytes(int32 size) const
	{
		return size + 2 + allocated_encoded_size_bytes();
	}

	int32 queued_count() const
	{
		return m_queued_count;
	}

	void allocate(int32 data_size, s_simulation_queue_element** out_allocated_elem);
	void deallocate(s_simulation_queue_element* element);

	void enqueue(s_simulation_queue_element* element);
	void dequeue(s_simulation_queue_element** out_deq_elem);

	void clear();

	void dispose(c_simulation_queue* to_dispose);
};

inline void c_simulation_queue::allocate(int32 data_size, s_simulation_queue_element** out_allocated_elem)
{
	*out_allocated_elem = NULL;

	if (initialized())
	{
		uint32 required_data_size = sizeof(s_simulation_queue_element) + data_size;

		if (allocated_count() + 1 > k_simulation_queue_count_max)
		{
			if (allocated_size_in_bytes() + required_data_size > k_simulation_queue_size_max)
			{
				if (data_size < k_simulation_payload_size_max)
				{
					if (allocated_new_encoded_size_bytes(data_size) > k_simulation_queue_max_encoded_size)
					{
						uint8* net_heap_block = network_heap_allocate_block(required_data_size);
						if (net_heap_block)
						{
							csmemset(net_heap_block, 0, required_data_size);
							s_simulation_queue_element* queue_elem = (s_simulation_queue_element*)net_heap_block;
							queue_elem->type = _simulation_queue_element_type_none;
							queue_elem->data = net_heap_block;
							queue_elem->data_size = data_size;
							queue_elem->next = NULL;

							m_allocated_count++;
							m_allocated_size_in_bytes += required_data_size;
							*out_allocated_elem = queue_elem;
						}
						else
						{
							// DEBUG
						}
					}
				}
			}
		}
	}
}

inline void c_simulation_queue::deallocate(s_simulation_queue_element* element)
{
	if (initialized())
	{
		int32 actual_data_size = element->data_size + sizeof(s_simulation_queue_element);

		m_allocated_size_in_bytes -= actual_data_size;
		m_allocated_count--;
		network_heap_free_block((uint8*)element);
	}
}

inline void c_simulation_queue::enqueue(s_simulation_queue_element* element)
{
	if (initialized())
	{
		if (m_tail)
		{
			m_tail->next = element;
		}
		else
		{
			m_head = element;
		}

		m_tail = element;
		m_queued_count++;
		m_queued_size += element->data_size;
	}
}

inline void c_simulation_queue::dequeue(s_simulation_queue_element** out_deq_elem)
{
	*out_deq_elem = NULL;
	if (initialized())
	{
		m_queued_count--;
		m_queued_size -= m_head->data_size + sizeof(s_simulation_queue_element);
		*out_deq_elem = m_head;
		m_head = m_head->next;
		(*out_deq_elem)->next = NULL;
	}
}

inline void c_simulation_queue::clear()
{
	if (initialized())
	{
		while (m_head)
		{
			s_simulation_queue_element* element_to_deque = NULL;
			dequeue(&element_to_deque);
			deallocate(element_to_deque);
		}
	}
}

inline void c_simulation_queue::dispose(c_simulation_queue* to_dispose)
{
	if (to_dispose->initialized())
	{
		to_dispose->clear();
		to_dispose = NULL;
	}
}