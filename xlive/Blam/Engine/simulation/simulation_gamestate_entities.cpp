#include "stdafx.h"
#include "simulation_gamestate_entities.h"

#include "game/game.h"
#include "main/main_game.h"
#include "memory/bitstream.h"
#include "memory/data.h"
#include "memory/rockall_heap_manager.h"
#include "networking/network_event.h"
#include "objects/object_constants.h"
#include "objects/object_types.h"
#include "objects/objects.h"

/* macros */

#define simulation_gamestate_entity_get(index) ((struct s_simulation_gamestate_entity*)datum_get(g_simulation_gamestate_entity_data, (index)))
#define simulation_gamestate_entity_try_and_get(index) ((struct s_simulation_gamestate_entity*)datum_try_and_get(g_simulation_gamestate_entity_data, (index)))

/* globals */

static data_array* g_simulation_gamestate_entity_data = NULL;

/* public code */

void simulation_gamestate_entities_initialize(
	void)
{
	g_simulation_gamestate_entity_data = DATA_NEW("sim. gamestate entities", k_maximum_objects_per_map, sizeof(s_simulation_gamestate_entity), 0, normal_allocation_global_get());
	ASSERT(g_simulation_gamestate_entity_data);

	return;
}

void simulation_gamestate_entities_dispose(
	void)
{
	if (g_simulation_gamestate_entity_data)
	{
		DATA_DISPOSE(g_simulation_gamestate_entity_data);
		g_simulation_gamestate_entity_data = NULL;
	}

	return;
}

void simulation_gamestate_entities_initialize_for_new_map(
	void)
{
	event(_event_message, "networking:simulation:gamestate: initialize for new map, wiping everything");
	
	data_make_valid(g_simulation_gamestate_entity_data);
	data_delete_all(g_simulation_gamestate_entity_data);

	return;
}

void simulation_gamestate_entities_build_clear_flags(
	s_simulation_queue_gamestate_clear_data* gamestate_clear_data_out)
{
	int32 clear_count = 0;
	int32 already_marked_for_deletion = 0;

	gamestate_clear_data_out->entities.clear();
	data_iterator iterator;
	iterator_new(&iterator, g_simulation_gamestate_entity_data);
	
	while (iterator_next(&iterator))
	{
		s_simulation_gamestate_entity* current_gamestate_entity = (s_simulation_gamestate_entity*)datum_get(iterator.data, iterator.index);
		
		if (current_gamestate_entity->marked_for_deletion)
		{
			++already_marked_for_deletion;
		}
		else
		{
			gamestate_clear_data_out->entities.set(iterator.absolute_index, true);
			++clear_count;
		}
	}

	event(
		_event_message,
		"networking:simulation:gamestate: build clear flags found %d gamestate entities to clear [%d already marked]",
		clear_count,
		already_marked_for_deletion
	);

	if (already_marked_for_deletion>0)
	{
		event(
			_event_error,
			"networking:simulation:gamestate: found %d entities already marked for deletion, which I don't think should happen",
			already_marked_for_deletion
		);
	}

	return;
}


void simulation_gamestate_entities_clear_by_flags(
	const s_simulation_queue_gamestate_clear_data* gamestate_clear_data)
{
	int32 cleared_count = 0;

	for (uint32 gamestate_absolute_index = 0; gamestate_absolute_index < k_maximum_objects_per_map; ++gamestate_absolute_index)
	{
		if (gamestate_clear_data->entities.test(gamestate_absolute_index))
		{
			const s_simulation_gamestate_entity* gamestate_entity = (s_simulation_gamestate_entity*)datum_get_absolute(g_simulation_gamestate_entity_data, gamestate_absolute_index);
			const datum entity_datum_index = DATUM_INDEX_NEW(gamestate_entity->identifier, gamestate_absolute_index);

			if (!game_is_playback())
			{
				ASSERT(gamestate_entity->simulation_entity_index);
				ASSERT(gamestate_entity->marked_for_deletion);
			}

			datum_delete(g_simulation_gamestate_entity_data, entity_datum_index);
			++cleared_count;
		}
	}

	event(
		_event_message,
		"networking:simulation:gamestate: deleted %d gamestate entities by flags",
		cleared_count
	);

	return;
}


void simulation_gamestate_entities_notify_simulation_world_reset(
	void)
{
	event(_event_message, "networking:simulation:gamestate: simulation world has been reset, clearing all entity index referenced");

	data_iterator iterator;
	iterator_new(&iterator, g_simulation_gamestate_entity_data);
	
	while (iterator_next(&iterator))
	{
		((s_simulation_gamestate_entity*)datum_get(iterator.data, iterator.index))->simulation_entity_index = NONE;
	}

	return;
}

void simulation_gamestate_entities_dispose_from_old_map(
	void)
{
	if (!main_game_reset_in_progress())
	{
		data_make_invalid(g_simulation_gamestate_entity_data);
	}

	return;
}

int32 simulation_gamestate_entity_create(
	void)
{
	int32 gamestate_index = datum_new(g_simulation_gamestate_entity_data);

	if (gamestate_index!=NONE)
	{
		s_simulation_gamestate_entity* entity = simulation_gamestate_entity_get(gamestate_index);
		entity->object_index = NONE;
		entity->simulation_entity_index = NONE;
		entity->marked_for_deletion = false;
	}
	else
	{
		event(_event_warning, "networking:simulation:gamestate: failed to allocate simulation gamestate entity");
	}

	return gamestate_index;
}

int32 simulation_gamestate_entity_create_at_index(
	int32 gamestate_index)
{
	ASSERT(gamestate_index != NONE);
	
	int32 created_gamestate_index = datum_new_at_index(g_simulation_gamestate_entity_data, gamestate_index);
	
	if (created_gamestate_index == NONE)
	{
		datum_delete(g_simulation_gamestate_entity_data, gamestate_index);
		created_gamestate_index = datum_new_at_index(g_simulation_gamestate_entity_data, gamestate_index);

		event(
			_event_warning,
			"networking:simulation:gamestate: had to delete an entity to create at index 0x%8X",
			gamestate_index
		);
	}

	if (created_gamestate_index)
	{
		s_simulation_gamestate_entity* gamestate_entity = simulation_gamestate_entity_get(gamestate_index);
		gamestate_entity->object_index = NONE;
		gamestate_entity->simulation_entity_index = NONE;
		gamestate_entity->marked_for_deletion = false;
	}

	return gamestate_index;
}

void simulation_gamestate_entity_delete(
	int32 gamestate_index)
{
	ASSERT(gamestate_index != NONE);

	datum_delete(g_simulation_gamestate_entity_data, gamestate_index);
	
	return;
}

int32 simulation_gamestate_entity_get_object_index(
	int32 gamestate_index)
{
	int32 object_index = NONE;

	ASSERT(gamestate_index != NONE);
	
	s_simulation_gamestate_entity const* gamestate_entity = simulation_gamestate_entity_try_and_get(gamestate_index);
	
	if (gamestate_entity)
	{
		object_index = gamestate_entity->object_index;
	}
	
	return object_index;
}

int32 simulation_gamestate_entity_get_object_index_type_safe(
	int32 gamestate_index,
	uint32 object_type_mask)
{
	int32 object_index = NONE;

	if (gamestate_index!=NONE)
	{
		int32 unsafe_object_index = simulation_gamestate_entity_get_object_index(gamestate_index);

		if (unsafe_object_index!=NONE)
		{
			e_object_type object_type = object_get_type(unsafe_object_index);

			if (!TEST_BIT(object_type_mask, object_type))
			{
				event(
					_event_warning,
					"networking:simulation:objects: gamestate_index 0x%08X is object 0x%08X type %d but expected type mask 0x%04X, returning NONE",
					gamestate_index,
					object_index,
					object_type,
					object_type_mask
				);
			}
		}
		else
		{
			event(_event_warning, "networking:simulation:objects: tried to get object from gamestate_index 0x%08X with no object");
		}
	}
	else
	{
		event(_event_warning, "networking:simulation:objects: tried to get object from gamestate_index NONE");
	}

	return object_index;
}

void simulation_gamestate_entity_set_object_index(
	int32 gamestate_index,
	int32 object_index)
{
	ASSERT(gamestate_index != NONE);

	s_simulation_gamestate_entity* gamestate_entity = simulation_gamestate_entity_get(gamestate_index);
	gamestate_entity->object_index = object_index;
	
	return;
}

int32 simulation_gamestate_entity_get_simulation_entity_index(
	int32 gamestate_index)
{
	ASSERT(gamestate_index != NONE);

	const s_simulation_gamestate_entity* gamestate_entity = simulation_gamestate_entity_get(gamestate_index);
	int32 simulation_entity_index = gamestate_entity->simulation_entity_index;

	return simulation_entity_index;
}

void simulation_gamestate_entity_set_simulation_entity_index(
	int32 gamestate_index,
	int32 entity_index)
{
	ASSERT(gamestate_index != NONE);

	s_simulation_gamestate_entity* gamestate_entity = simulation_gamestate_entity_get(gamestate_index);
	gamestate_entity->simulation_entity_index = entity_index;
	
	return;
}

bool simulation_gamestate_indices_are_equivalent(
	int32 gamestate_1_index,
	int32 gamestate_2_index)
{
	return DATUM_INDEX_TO_ABSOLUTE_INDEX(gamestate_1_index) == DATUM_INDEX_TO_ABSOLUTE_INDEX(gamestate_2_index);
}

bool simulation_gamestate_index_valid(
	int32 gamestate_index)
{
	bool index_valid = false;

	if (gamestate_index != NONE)
	{
		index_valid = simulation_gamestate_entity_try_and_get(gamestate_index) != NULL;
	}

	return index_valid;
}


void simulation_gamestate_index_encode(
	c_bitstream* stream,
	int32 gamestate_index)
{
	stream->write_integer("gamestate-index-id", DATUM_INDEX_TO_IDENTIFIER(gamestate_index), 16);
	stream->write_integer("gamestate-index-absolute", DATUM_INDEX_TO_ABSOLUTE_INDEX(gamestate_index), 11);
	
	return;
}

void simulation_gamestate_index_decode(
	c_bitstream* stream, 
	int32* gamestate_index_out)
{
	int32 object_index_id = stream->read_integer("gamestate-index-id", 16);
	int32 object_absolute_index = stream->read_integer("gamestate-index-absolute", 11);

	*gamestate_index_out = DATUM_INDEX_NEW(object_absolute_index, object_index_id);
	
	return;
}
