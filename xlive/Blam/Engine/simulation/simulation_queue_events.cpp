#include "stdafx.h"
#include "simulation_queue_events.h"

#include "simulation.h"
#include "simulation_gamestate_entities.h"
#include "simulation_type_collection.h"
#include "simulation_world.h"

#include "memory/bitstream.h"
#include "networking/network_event.h"

/* structures */

struct s_simulation_queue_events_apply
{
	e_simulation_event_type event_type;
	int32 reference_count;
	int32 gamestate_indices[k_entity_reference_indices_count_max];
	uint8 data[k_simulation_event_maximum_payload_size];
	int32 data_size;
};

/* */

static bool encode_event_to_buffer(
	uint8* encode_buffer,
	int32 encode_buffer_size,
	int32* out_encoded_size,
	e_simulation_event_type event_type,
	int32 reference_count,
	const int32* entity_references,
	int32 payload_size,
	void* payload
)
{
	c_bitstream stream(encode_buffer, encode_buffer_size);

	ASSERT(VALID_INDEX(event_type, k_simulation_event_type_count));

	stream.begin_writing(k_bitstream_default_alignment);
	stream.write_integer("event-type", event_type, 5);
	stream.write_integer("reference-count", reference_count, 2);

	for (int32 i = 0; i < reference_count; i++)
	{
		stream.write_bool("entity-index-exists", entity_references[i] != NONE);
		if (entity_references[i] != NONE)
		{
			simulation_entity_index_encode(&stream, entity_references[i]);
		}
	}

	c_simulation_type_collection* sim_type_collection = simulation_get_type_collection();
	c_simulation_event_definition* sim_event_definition = sim_type_collection->get_event_definition(event_type);
	// write the event data to the stream
	sim_event_definition->encode(payload_size, payload, &stream);

	*out_encoded_size = stream.get_space_used_in_bytes();
	
	const bool result = !stream.error_occurred();
	stream.finish_writing(NULL);

	return result;
}

static bool decode_event_from_buffer(
	int32 encoded_size,
	uint8* encoded_data,
	s_simulation_queue_events_apply* decoded_event_data)
{
	bool result = false;

	c_bitstream stream(encoded_data, encoded_size);
	stream.begin_reading();

	decoded_event_data->event_type = (e_simulation_event_type)stream.read_integer("event-type", 5);
	decoded_event_data->reference_count = stream.read_integer("reference-count", 2);

	for (int32 i = 0; i<decoded_event_data->reference_count; ++i)
	{
		if (stream.read_bool("entity-index-exists"))
		{
			simulation_entity_index_decode(&stream, &decoded_event_data->gamestate_indices[i]);
		}
		else
		{
			decoded_event_data->gamestate_indices[i] = NONE;
		}
	}

	c_simulation_type_collection* sim_type_collection = simulation_get_type_collection();
	c_simulation_event_definition* sim_event_def = sim_type_collection->get_event_definition(decoded_event_data->event_type);

	decoded_event_data->data_size = sim_event_def->payload_size();
	if (VALID_INDEX(decoded_event_data->event_type, k_simulation_event_type_count))
	{
		if (decoded_event_data->reference_count <= NUMBEROF(decoded_event_data->gamestate_indices))
		{
			if (decoded_event_data->data_size <= k_simulation_event_maximum_payload_size)
			{
				if (decoded_event_data->data_size > 0 &&
					sim_event_def->decode(decoded_event_data->data_size, decoded_event_data->data, &stream))
				{
					// Success
				}
				else
				{
					event(
						_event_warning,
						"networking:simulation:queue:events: failed to decode event payload type %d",
						decoded_event_data->event_type
					);
				}

				result = !stream.error_occurred();
			}
			else
			{
				event(
					_event_error,
					"networking:simulation:queue:events: invalid event payload size %d",
					decoded_event_data->data_size
				);
			}
		}
		else
		{
			event(
				_event_error,
				"networking:simulation:queue:events: crazy reference count during decode %d",
				decoded_event_data->reference_count
			);
		}
	}
	else
	{
		event(
			_event_error,
			"networking:simulation:queue:events: failed to decode event (bad event type %d)",
			decoded_event_data->event_type
		);
	}

	stream.finish_reading();

	return result;
}

void simulation_queue_event_insert(
	e_simulation_event_type event_type,
	uint32 reference_count,
	const int32* entity_reference_indices, 
	int32 payload_size,
	void* payload)
{
	ASSERT(VALID_INDEX(event_type, k_simulation_event_type_count));

	int32 event_buffer_encoded_size_bytes;
	uint8 encode_buffer[k_simulation_event_maximum_payload_size];
	s_simulation_queue_element* element = NULL;

	//c_simulation_type_collection* simulation_type_collection = simulation_get_type_collection();
	//c_simulation_event_definition* sim_event_def = simulation_type_collection->get_event_definition(type);

	// skip postprocess, not available in h2
	// call postprocess from event def
	// end

	if (encode_event_to_buffer(
		encode_buffer,
		sizeof(encode_buffer),
		&event_buffer_encoded_size_bytes,
		event_type,
		k_entity_reference_indices_count_max,
		entity_reference_indices,
		payload_size,
		payload))
	{
		simulation_get_world()->simulation_queue_allocate(_simulation_queue_element_type_event, event_buffer_encoded_size_bytes, &element);
		if (element)
		{
			// copy the data to the buffer
			csmemcpy(element->data, encode_buffer, event_buffer_encoded_size_bytes);

			// copy it to the queue
			simulation_get_world()->simulation_queue_enqueue(element);
		}
		else
		{
			event(
				_event_fatal,
				"networking:simulation:queue: failed to allocate element for event %d/%d",
				event_type,
				event_buffer_encoded_size_bytes
			);
		}
	}
	else
	{
		event(
			_event_error,
			"networking:simulation:queue: failed to encode event for simulation queue type %d payload size %d",
			event_type,
			payload_size
		);
	}

	return;
}

void simulation_queue_event_apply(const s_simulation_queue_element* update)
{
	s_simulation_queue_events_apply sim_apply_data;
	csmemset(&sim_apply_data, 0, sizeof(sim_apply_data));

	// apply the decoded event to sim
	if (decode_event_from_buffer(update->data_size, update->data, &sim_apply_data))
	{
		c_simulation_type_collection* sim_type_collection = simulation_get_type_collection();
		c_simulation_event_definition* sim_event_def = sim_type_collection->get_event_definition(sim_apply_data.event_type);

		// ### FIXME !!! this actually requires entity references, instead of object references
		sim_event_def->perform(
			sim_apply_data.reference_count,
			sim_apply_data.gamestate_indices,
			sim_apply_data.data_size,
			sim_apply_data.data);
	}
	else
	{
		event(_event_error, "networking:simulation:queue: failed to decode event for simulation queue");
	}
	
	return;
}
