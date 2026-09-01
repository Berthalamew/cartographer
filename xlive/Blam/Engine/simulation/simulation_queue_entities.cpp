#include "stdafx.h"
#include "simulation_queue_entities.h"

#include "simulation.h"
#include "simulation_gamestate_entities.h"
#include "simulation_type_collection.h"
#include "simulation_world.h"

#include "game/game.h"
#include "memory/bitstream.h"
#include "networking/network_event.h"
#include "objects/objects.h"

/* structures */

struct s_simulation_queue_decoded_creation_data
{
	int32 entity_index;
	e_simulation_entity_type entity_type;
	datum gamestate_index;
	uint32 initial_update_mask;
	uint32 creation_data_size;
	uint8 creation_data[k_simulation_entity_maximum_creation_data_size];
	uint32 state_data_size;
	uint8 state_data[k_simulation_entity_maximum_state_data_size];
};

struct s_simulation_queue_decoded_update_data
{
	int32 entity_index;
	e_simulation_entity_type entity_type;
	datum gamestate_index;
	uint32 update_mask;
	uint32 state_data_size;
	uint8 state_data[k_simulation_entity_maximum_state_data_size];
};

/* public code */


bool simulation_queue_entity_creation_allocate(
	s_simulation_queue_entity_data* entity_data,
	uint32 initial_update_mask,
	s_simulation_queue_element** simulation_queue_element_out,
	int32* gamestate_index_out)
{
	bool success = false;
	s_simulation_queue_element* simulation_queue_element = NULL;

	ASSERT(game_is_distributed());
	ASSERT(!game_is_playback());

	int32 gamestate_index = simulation_gamestate_entity_create();

	if (gamestate_index==NONE)
	{
		event(
			_event_error,
			"networking:simulation:queue:entities: failed to create gamestate index for entity creation (entity type %d)",
			entity_data->entity_type
		);
	}
	else
	{
		int32 entity_creation_encoded_size_bytes;
		uint8 entity_creation_buffer[k_simulation_queue_element_data_size_max];

		/* NOTE missing pre - gameworld processing, unused in h2 so skip(even in H3 mainly unused)
		c_simulation_type_collection* type_collection= simulation_get_type_collection();
		c_simulation_entity_definition* entity_definition= type_collection->get_entity_definition(entity_data->entity_type);
		
		entity_definition->prepare_creation_data_for_gameworld(entity_data->creation_data_size, entity_data->creation_data);
		entity_definition->prepare_state_data_for_gameworld(initial_update_mask, entity_data->state_data_size, entity_data->state_data);
		*/

		if (!encode_simulation_queue_creation_to_buffer(
				entity_creation_buffer,
				sizeof(entity_creation_buffer),
				gamestate_index,
				entity_data,
				initial_update_mask,
				&entity_creation_encoded_size_bytes))
		{
			event(
				_event_error,
				"networking:simulation:queue:entities: failed to encode entity creation to buffer type %d",
				entity_data->entity_type
			);
		}
		else
		{
			c_simulation_world* world = simulation_get_world();

			world->simulation_queue_allocate(_simulation_queue_element_type_entity_creation, entity_creation_encoded_size_bytes, &simulation_queue_element);

			if (simulation_queue_element)
			{
				// copy the encode_buffer to the buffer, enqueuing done later for entities
				csmemcpy(simulation_queue_element->data, entity_creation_buffer, entity_creation_encoded_size_bytes);
				success = true;
			}
			else if (game_time_initialized() && (game_time_get_paused() || game_time_get_speed()==0.f))
			{
				event(
					_event_warning,
					"networking:simulation:queue:entities: failed to allocate element for entity %d/%d (creation)",
					entity_data->entity_type,
					entity_creation_encoded_size_bytes
				);
			}
			else
			{
				event(
					_event_fatal,
					"networking:simulation:queue:entities: failed to allocate element for entity %d/%d (creation)",
					entity_data->entity_type,
					entity_creation_encoded_size_bytes
				);
			}
		}
	}

	if (success)
	{
		*simulation_queue_element_out = simulation_queue_element;
		*gamestate_index_out = gamestate_index;
	}
	else
	{
		if (gamestate_index != NONE)
		{
			simulation_gamestate_entity_delete(gamestate_index);
		}

		if (simulation_queue_element)
		{
			c_simulation_world* world = simulation_get_world();

			world->simulation_queue_free(simulation_queue_element);
		}
	}

	return success;
}

void simulation_queue_entity_creation_insert(
	s_simulation_queue_element* simulation_queue_element)
{
	c_simulation_world* world= simulation_get_world();

	ASSERT(simulation_queue_element->type == _simulation_queue_element_type_entity_creation);

	world->simulation_queue_enqueue(simulation_queue_element);

	return;
}

void simulation_queue_entity_creation_apply(
	const s_simulation_queue_element* element)
{
	ASSERT(element);
	ASSERT(element->type == _simulation_queue_element_type_entity_creation);

	if (game_is_distributed() && !game_is_playback())
	{
		s_simulation_queue_decoded_creation_data decoded_creation_data;
		
		csmemset(&decoded_creation_data, 0, sizeof(decoded_creation_data));

		if (decode_simulation_queue_creation_from_buffer(element->data, element->data_size, &decoded_creation_data))
		{
			c_simulation_type_collection* type_collection = simulation_get_type_collection();
			c_simulation_entity_definition* entity_definition = type_collection->get_entity_definition(decoded_creation_data.entity_type);
			bool success = true;

			ASSERT(decoded_creation_data.gamestate_index != NONE);

			if (game_is_playback())
			{
				int32 created_gamestate_index = simulation_gamestate_entity_create_at_index(decoded_creation_data.gamestate_index);

				ASSERT(created_gamestate_index != NONE);
				ASSERT(created_gamestate_index == decoded_creation_data.gamestate_index);

				if (created_gamestate_index != decoded_creation_data.gamestate_index ||
					created_gamestate_index == NONE)
				{
					event(
						_event_error,
						"networking:simulation:queue:entities: failed to create gamestate index (during playback) for entity type %d",
						decoded_creation_data.entity_type
					);

					success = false;
				}
			}
			
			if (success)
			{
				success = entity_definition->create_game_entity(
					decoded_creation_data.gamestate_index,
					decoded_creation_data.creation_data_size,
					decoded_creation_data.creation_data,
					decoded_creation_data.initial_update_mask,
					decoded_creation_data.state_data_size,
					decoded_creation_data.state_data
				);

				if (!success)
				{
					event(
						_event_warning,
						"networking:simulation:queue:entities: failed to create game entity for entity type %d",
						decoded_creation_data.entity_type
					);
				}
			}
		}
		else
		{
			event(_event_error, "networking:simulation:queue: failed to decode creation");
		}
	}

	return;
}

bool simulation_queue_entity_update_allocate(
	s_simulation_queue_entity_data* entity_data,
	int32 gamestate_index,
	uint32 update_mask,
	s_simulation_queue_element** simulation_queue_element_out)
{
	bool success = false;
	s_simulation_queue_element* simulation_queue_element = NULL;

	ASSERT(game_is_distributed());
	ASSERT(!game_is_playback());
	ASSERT(entity_data->entity_index != NONE);
	ASSERT(gamestate_index != NONE);
	ASSERT(simulation_gamestate_index_valid(gamestate_index));

	if (entity_data->entity_index == NONE)
	{
		event(_event_error, "networking:simulation:queue:entities: attempting to enqueue update with bad entity index (type %d)", entity_data->entity_type);
	}
	else if (gamestate_index == NONE)
	{
		event(_event_error, "networking:simulation:queue:entities: attempting to enqueue update with bad gamestate index (type %d)", entity_data->entity_type);
	}
	else
	{
		uint8 entity_update_buffer[k_simulation_queue_element_data_size_max];
		int32 entity_update_encoded_size_bytes;

		/* TODO: finish this
		c_simulation_type_collection* type_collection = simulation_get_type_collection();
		c_simulation_entity_definition* entity_definition = type_collection->get_entity_definition(entity_data->entity_type);

		entity_definition->prepare_state_data_for_gameworld(update_mask, entity_data->state_data_size, entity_data->state_data);
		*/

		if (encode_simulation_queue_update_to_buffer(
				entity_update_buffer,
				sizeof(entity_update_buffer),
				entity_data,
				gamestate_index,
				update_mask,
				&entity_update_encoded_size_bytes))
		{
			c_simulation_world* world = simulation_get_world();

			world->simulation_queue_allocate(_simulation_queue_element_type_entity_update, entity_update_encoded_size_bytes, &simulation_queue_element);
			
			if (simulation_queue_element)
			{
				csmemcpy(simulation_queue_element->data, entity_update_buffer, entity_update_encoded_size_bytes);
				success = true;
			}
			else if (game_time_initialized() && (game_time_get_paused() || game_time_get_speed()==0.f))
			{
				event(
					_event_warning,
					"networking:simulation:queue:entities: failed to allocate element for entity %d/%d (update)",
					entity_data->entity_type,
					entity_update_encoded_size_bytes
				);
			}
			else
			{
				event(
					_event_fatal,
					"networking:simulation:queue:entities: failed to allocate element for entity %d/%d (update)",
					entity_data->entity_type,
					entity_update_encoded_size_bytes
				);
			}
		}
		else
		{
			event(
				_event_error,
				"networking:simulation:queue failed to encode update type %d",
				entity_data->entity_type
			);
		}
	}

	if (success)
	{
		*simulation_queue_element_out = simulation_queue_element;
	}
	else if (simulation_queue_element)
	{
		c_simulation_world* world = simulation_get_world();

		world->simulation_queue_free(simulation_queue_element);
	}

	return success;
}


void simulation_queue_entity_update_insert(
	s_simulation_queue_element* simulation_queue_element)
{
	c_simulation_world* world = simulation_get_world();

	ASSERT(simulation_queue_element->type == _simulation_queue_element_type_entity_update);

	world->simulation_queue_enqueue(simulation_queue_element);
	
	return;
}

void simulation_queue_entity_update_apply(
	const s_simulation_queue_element* element)
{
	ASSERT(element);
	ASSERT(element->type == _simulation_queue_element_type_entity_update);

	if (game_is_distributed())
	{
		s_simulation_queue_decoded_update_data decoded_update_data;

		csmemset(&decoded_update_data, 0, sizeof(decoded_update_data));

		if (decode_simulation_queue_update_from_buffer(element->data, element->data_size, &decoded_update_data))
		{
			c_simulation_type_collection* type_collection = simulation_get_type_collection();
			c_simulation_entity_definition* entity_definition = type_collection->get_entity_definition(decoded_update_data.entity_type);

			if (!entity_definition->update_game_entity(
					decoded_update_data.gamestate_index,
					decoded_update_data.update_mask,
					decoded_update_data.state_data_size,
					decoded_update_data.state_data))
			{
				event(
					_event_warning,
					"networking:simulation:queue:entities: failed to apply update to game entity (type %d 0x%8X)",
					decoded_update_data.entity_type,
					decoded_update_data.gamestate_index
				);
			}
		}
		else
		{
			event(_event_error, "networking:simulation:queue: failed to decode update");
		}
	}

	return;
}

void simulation_queue_entity_deletion_insert(
	s_simulation_entity* entity,
	bool force_cleanup_after_deletion)
{
	uint8 scratch_buffer[512];

	bool success = false;
	int32 encoded_size_bytes = 0;
	c_bitstream bitstream(scratch_buffer, sizeof(scratch_buffer));

	ASSERT(entity);
	ASSERT(game_is_distributed());
	ASSERT(!game_is_playback());
	ASSERT(entity->entity_index != NONE);
	ASSERT(entity->gamestate_index != NONE);
	ASSERT(simulation_gamestate_index_valid(entity->gamestate_index));

	bitstream.begin_writing(k_bitstream_default_alignment);
	simulation_queue_entity_encode_header(&bitstream, entity->entity_type, NONE);
	bitstream.write_bool("force-cleanup", force_cleanup_after_deletion);

	encoded_size_bytes = bitstream.get_space_used_in_bytes();

	if (!bitstream.error_occurred())
	{
		c_simulation_world* world = simulation_get_world();
		s_simulation_queue_element* element = NULL;

		world->simulation_queue_allocate(_simulation_queue_element_type_entity_deletion, encoded_size_bytes, &element);
		if (element)
		{
			csmemcpy(element->data, scratch_buffer, encoded_size_bytes);
			world->simulation_queue_enqueue(element);
			success = true;
		}
		else
		{
			event(
				_event_fatal,
				"networking:simulation:queue: failed to allocate element for entity %d/%d (deletion)",
				entity->entity_type,
				encoded_size_bytes
			);
		}
	}
	else
	{
		event(
			_event_error,
			"networking:simulation:queue: failed to encode deletion for type %d",
			entity->entity_type
		);
	}

	bitstream.finish_writing(NULL);

	return;
}

void simulation_queue_entity_deletion_apply(
	const s_simulation_queue_element* element)
{
	int32 gamestate_index = NONE;
	e_simulation_entity_type entity_type = k_simulation_entity_type_none;
	c_bitstream entity_bitstream(element->data, element->data_size);

	ASSERT(element);
	ASSERT(element->type == _simulation_queue_element_type_entity_deletion);
	ASSERT(game_is_distributed());

	entity_bitstream.begin_reading();

	if (simulation_queue_entity_decode_header(&entity_bitstream, &entity_type, &gamestate_index))
	{
		bool force_cleanup = entity_bitstream.read_bool("force-cleanup");

		c_simulation_type_collection* type_collection = simulation_get_type_collection();
		c_simulation_entity_definition* entity_definition = type_collection->get_entity_definition(entity_type);
		int32 force_cleanup_object_index = NONE;

		if (force_cleanup)
		{
			if (entity_definition->entity_type_is_gameworld_object())
			{
				int32 object_index = simulation_gamestate_entity_get_object_index(gamestate_index);
				
				if (object_index!=NONE)
				{
					force_cleanup_object_index = object_index;
				}
			}
		}

		if (!entity_definition->delete_game_entity(gamestate_index))
		{
			event(
				_event_warning,
				"networking:simulation:queue:entities: failed to delete game entity gamestate 0x%8X",
				gamestate_index
			);
		}

		if (force_cleanup_object_index!=NONE)
		{
			event(
				_event_message,
				"networking:simulation:queue:entities: force cleanup on object 0x%08X entity type %d gamestate 0x%08X [object count %d]",
				force_cleanup_object_index,
				entity_type,
				gamestate_index,
				object_header_data_get()->count
			);

			object_delete_immediately(force_cleanup_object_index);
		}

		simulation_gamestate_entity_delete(gamestate_index);
	}
	else
	{
		event(_event_error, "networking:simulation:queue:entities: failed to decode header for deletion");
	}

	entity_bitstream.finish_reading();

	return;
}

void simulation_queue_entity_promotion_insert(
	s_simulation_entity* entity)
{
	int32 encoded_size_bytes;
	uint8 scratch_buffer[512];
	c_bitstream bitstream(scratch_buffer, sizeof(scratch_buffer));

	ASSERT(entity);
	ASSERT(game_is_distributed());
	ASSERT(!game_is_playback());
	ASSERT(simulation_gamestate_index_valid(entity->gamestate_index));

	bitstream.begin_writing(k_bitstream_default_alignment);
	
	simulation_queue_entity_encode_header(&bitstream, entity->entity_type, NONE);
	
	bitstream.finish_writing(NULL);

	encoded_size_bytes = bitstream.get_space_used_in_bytes();

	if (bitstream.error_occurred())
	{
		event(_event_error, "networking:simulation:queue: failed to encode promotion for entity type %d", entity->entity_type);
	}
	else
	{
		c_simulation_world* world = simulation_get_world();
		s_simulation_queue_element* element = NULL;

		world->simulation_queue_allocate(_simulation_queue_element_type_entity_promotion, encoded_size_bytes, &element);

		if (element)
		{
			csmemcpy(element->data, scratch_buffer, encoded_size_bytes);
			world->simulation_queue_enqueue(element);
		}
		else if (game_time_initialized() && (game_time_get_paused() || game_time_get_speed() == 0.f))
		{
			event(_event_warning, "networking:simulation:queue: failed to allocate element for entity %d/%d (promotion)", entity->entity_type, encoded_size_bytes);
		}
		else
		{
			event(_event_fatal, "networking:simulation:queue: failed to allocate element for entity %d/%d (promotion)", entity->entity_type, encoded_size_bytes);
		}
	}

	return;
}

void simulation_queue_entity_promotion_apply(
	const s_simulation_queue_element* element)
{
	ASSERT(element);
	ASSERT(element->type == _simulation_queue_element_type_entity_promotion);

	if (game_is_distributed())
	{
		int32 gamestate_index = NONE;
		e_simulation_entity_type entity_type = k_simulation_entity_type_none;

		c_bitstream entity_bitstream(element->data, element->data_size);
		
		entity_bitstream.begin_reading();

		if (simulation_queue_entity_decode_header(&entity_bitstream, &entity_type, &gamestate_index))
		{
			if (simulation_gamestate_index_valid(gamestate_index))
			{
				c_simulation_type_collection* type_collection = simulation_get_type_collection();
				c_simulation_entity_definition* entity_definition = type_collection->get_entity_definition(entity_type);

				if (!entity_definition->promote_game_entity_to_authority(gamestate_index))
				{
					event(
						_event_error,
						"networking:simulation:queue:entities: failed to promote game entity type %d 0x%8X",
						entity_type,
						gamestate_index
					);

					if (game_is_playback())
					{
						// Do nothing?
					}
				}
			}
			else
			{
				event(
					_event_warning,
					"networking:simulation:queue:entities: can't promote gamestate 0x%8X (type %d) as the gamestate is no longer valid",
					gamestate_index,
					entity_type
				);
			}

		}
		else
		{
			event(_event_error, "networking:simulation:queue:entities: failed to decode entity promotion header");
		}

		entity_bitstream.finish_reading();
	}

	return;
}

void simulation_queue_entity_encode_header(
	c_bitstream* bitstream,
	e_simulation_entity_type type,
	int32 gamestate_index)
{
	ASSERT(bitstream);
	ASSERT(gamestate_index != NONE);

	bitstream->write_integer("entity-type", type, 5);
	simulation_gamestate_index_encode(bitstream, gamestate_index);

	return;
}

bool simulation_queue_entity_decode_header(
	c_bitstream* bitstream,
	e_simulation_entity_type* entity_type,
	int32* gamestate_index)
{
	bool success = false;

	ASSERT(bitstream);
	ASSERT(entity_type);
	ASSERT(gamestate_index);

	*entity_type = (e_simulation_entity_type)bitstream->read_integer("entity-type", 5);
	simulation_gamestate_index_decode(bitstream, gamestate_index);
	
	if (VALID_INDEX(*entity_type, k_simulation_entity_count))
	{
		if (*gamestate_index==NONE)
		{
			event(_event_error, "networking:simulation:queue:entities: decoded a bad gamestate index %d", *gamestate_index);
		}
		else if (bitstream->error_occurred())
		{
			event(_event_error, "networking:simulation:queue:entities: header decode failed with bitstream error");
		}
		else
		{
			success = true;
		}
	}
	else
	{
		event(
			_event_error,
			"networking:simulation:queue:entities: decoded an out of range entity type %d",
			*entity_type
		);
	}

	return success;
}

bool encode_simulation_queue_creation_to_buffer(
	uint8* buffer,
	int32 buffer_size,
	int32 gamestate_index,
	s_simulation_queue_entity_data const* entity_data,
	uint32 initial_update_mask,
	int32* encoded_size_out)
{
	bool success;
	c_bitstream entity_creation_bitstream(buffer, buffer_size);
	c_simulation_type_collection* type_collection = simulation_get_type_collection();
	c_simulation_entity_definition* entity_definition = type_collection->get_entity_definition(entity_data->entity_type);

	ASSERT(gamestate_index != NONE);

	entity_creation_bitstream.begin_writing(k_bitstream_default_alignment);
	
	simulation_queue_entity_encode_header(&entity_creation_bitstream, entity_data->entity_type, gamestate_index);
	entity_creation_bitstream.write_integer("initial-update-mask", initial_update_mask, SIZEOF_BITS(initial_update_mask));

	// write the actual encode_buffer
	entity_definition->entity_creation_encode(entity_data->creation_data_size, entity_data->creation_data, NULL, &entity_creation_bitstream, false);

	if (initial_update_mask!=0)
	{
		uint32 update_mask_written = 0;

		entity_definition->entity_update_encode(true, initial_update_mask, &update_mask_written, entity_data->state_data_size, entity_data->state_data, NULL, &entity_creation_bitstream, 0, false);
	}

	*encoded_size_out = entity_creation_bitstream.get_space_used_in_bytes();

	success = !entity_creation_bitstream.error_occurred();
	entity_creation_bitstream.finish_writing(NULL);
	
	return success;
}

bool decode_simulation_queue_creation_from_buffer(
	uint8* buffer,
	int32 buffer_size,
	s_simulation_queue_decoded_creation_data* decoded_creation_data)
{
	bool success = false;
	c_simulation_type_collection* type_collection = simulation_get_type_collection();
	c_bitstream entity_creation_bitstream(buffer, buffer_size);
	
	entity_creation_bitstream.begin_reading();

	if (simulation_queue_entity_decode_header(&entity_creation_bitstream, &decoded_creation_data->entity_type, &decoded_creation_data->gamestate_index))
	{
		c_simulation_entity_definition* entity_definition;

		decoded_creation_data->initial_update_mask = entity_creation_bitstream.read_integer("initial-update-mask", SIZEOF_BITS(decoded_creation_data->initial_update_mask));
		entity_definition = type_collection->get_entity_definition(decoded_creation_data->entity_type);

		decoded_creation_data->creation_data_size = entity_definition->creation_data_size();
		decoded_creation_data->state_data_size = entity_definition->state_data_size();

		if (decoded_creation_data->creation_data_size <= k_simulation_entity_maximum_creation_data_size)
		{
			if (decoded_creation_data->state_data_size <= k_simulation_entity_maximum_state_data_size)
			{
				success = entity_definition->entity_creation_decode(
					decoded_creation_data->creation_data_size,
					decoded_creation_data->creation_data,
					&entity_creation_bitstream,
					false);

				if (success)
				{
					if (decoded_creation_data->initial_update_mask)
					{
						uint32 update_mask = 0;

						entity_definition->build_baseline_state_data(
							decoded_creation_data->creation_data_size,
							decoded_creation_data->creation_data,
							decoded_creation_data->state_data_size,
							decoded_creation_data->state_data
						);
						
						entity_definition->entity_update_decode(
							true,
							&update_mask,
							decoded_creation_data->state_data_size,
							decoded_creation_data->state_data,
							&entity_creation_bitstream,
							false
						);
					}

					success = !entity_creation_bitstream.error_occurred();
					entity_creation_bitstream.finish_reading();
				}
				else
				{
					event(_event_warning, "networking:simulation:queue:entities: failed to decode entity creation");
				}
			}
			else
			{
				event(
					_event_error,
					"networking:simulation:queue:entities: decoded invalid update data payload size %d for entity creation",
					decoded_creation_data->state_data_size
				);
			}
		}
		else
		{
			event(
				_event_error,
				"networking:simulation:queue:entities: decoded invalid creation payload size %d for entity creation",
				decoded_creation_data->creation_data_size
			);
		}

	}
	else
	{
		event(_event_error, "networking:simulation:queue:entities: failed to decode header for creation");
	}

	if (success && decoded_creation_data->gamestate_index == NONE)
	{
		event(_event_error, "networking:simulation:queue:entities: failed to decode creation (bad gamestate index) (type %d)", decoded_creation_data->gamestate_index);
		success = false;
	}

	return success;
}


bool encode_simulation_queue_update_to_buffer(
	uint8* buffer, 
	int32 buffer_size, 
	s_simulation_queue_entity_data const* entity_data,
	int32 gamestate_index,
	uint32 update_mask, 
	int32* encoded_size_out)
{
	bool success;
	c_bitstream entity_update_bitstream(buffer, buffer_size);
	
	entity_update_bitstream.begin_writing(k_bitstream_default_alignment);

	ASSERT(gamestate_index!=NONE);

	simulation_queue_entity_encode_header(&entity_update_bitstream, entity_data->entity_type, gamestate_index);

	entity_update_bitstream.write_integer("update-mask", update_mask, SIZEOF_BITS(update_mask));

	// Encode the update buffer
	{
		uint32 update_mask_written = 0;
		c_simulation_type_collection* type_collection = simulation_get_type_collection();
		c_simulation_entity_definition* entity_definition = type_collection->get_entity_definition(entity_data->entity_type);

		entity_definition->entity_update_encode(false, update_mask, &update_mask_written, entity_data->state_data_size, entity_data->state_data, NULL, &entity_update_bitstream, 0, false);
	}

	success = !entity_update_bitstream.error_occurred();
	
	*encoded_size_out = entity_update_bitstream.get_space_used_in_bytes();
	
	entity_update_bitstream.finish_writing(NULL);

	return success;
}

bool decode_simulation_queue_update_from_buffer(
	uint8* buffer,
	int32 buffer_size,
	s_simulation_queue_decoded_update_data* decoded_update_data)
{
	bool success = false;
	c_bitstream entity_update_bitstream(buffer, buffer_size);
	
	entity_update_bitstream.begin_reading();

	if (simulation_queue_entity_decode_header(&entity_update_bitstream, &decoded_update_data->entity_type, &decoded_update_data->gamestate_index))
	{
		uint8 creation_data_scratch[1024];
		c_simulation_type_collection* type_collection = simulation_get_type_collection();
		c_simulation_entity_definition* entity_definition = type_collection->get_entity_definition(decoded_update_data->entity_type);

		csmemset(creation_data_scratch, 0, sizeof(creation_data_scratch));

		decoded_update_data->update_mask = entity_update_bitstream.read_integer("update-mask", SIZEOF_BITS(decoded_update_data->update_mask));

		if (entity_definition->creation_data_size() <= k_simulation_entity_maximum_state_data_size)
		{
			uint32 update_mask_decoded = 0;

			entity_definition->build_creation_data(
				decoded_update_data->gamestate_index,
				entity_definition->creation_data_size(),
				creation_data_scratch
			);
			entity_definition->build_baseline_state_data(
				entity_definition->creation_data_size(),
				creation_data_scratch,
				decoded_update_data->state_data_size,
				decoded_update_data->state_data
			);
			success = entity_definition->entity_update_decode(
				false,
				&update_mask_decoded,
				decoded_update_data->state_data_size,
				decoded_update_data->state_data,
				&entity_update_bitstream,
				false
			);
			
			if (success)
			{
				success = !entity_update_bitstream.error_occurred();
				entity_update_bitstream.finish_reading();
			}
			else
			{
				event(
					_event_error,
					"networking:simulation:queue:entities: failed to decode entity update"
				);
			}
		}
		else
		{
			event(
				_event_error,
				"networking:simulation:queue:entities: creation data size for update exceeds scratch size?"
			);
		}
	}
	else
	{
		event(_event_error, "networking:simulation:queue:entities: failed to decode header for update");
	}

	return success;
}
