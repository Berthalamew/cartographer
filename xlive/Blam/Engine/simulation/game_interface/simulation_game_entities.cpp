#include "stdafx.h"
#include "simulation_game_entities.h"

#include "cache/cache_files.h"
#include "game/game.h"
#include "game/game_engine_simulation.h"
#include "networking/network_event.h"
#include "objects/objects.h"
#include "simulation/simulation.h"
#include "simulation/simulation_gamestate_entities.h"
#include "simulation/simulation_world.h"
#include "tag_files/tag_files.h"

/* public code */

void simulation_game_entities_apply_patches(
	void)
{
	PatchCall(Memory::GetAddress(0x1B6585), simulation_entity_create);
	PatchCall(Memory::GetAddress(0x1B65D7), simulation_entity_create);
	PatchCall(Memory::GetAddress(0x1B7233), simulation_entity_create);
	PatchCall(Memory::GetAddress(0x1B8D6C), simulation_entity_create);

	return;
}

bool simulation_object_index_valid(
	datum object_index)
{
	return object_index != NONE && object_try_and_get(object_index) != NULL;
}

int32 __cdecl simulation_entity_create(
	e_simulation_entity_type entity_type,
	int32 object_index,
	int32 gamestate_index)
{
	//return INVOKE(0x1B99C0, 0x1B2E95, simulation_entity_create, entity_type, object_index);

	int32 entity_index = NONE;
	c_simulation_world* world = simulation_get_world();

	if (world->is_distributed() && world->is_authority())
	{
		c_simulation_entity_database* entity_database = world->get_entity_database();
		entity_index = entity_database->entity_create(entity_type);

		if (entity_index!=NONE)
		{
			s_simulation_entity* entity = entity_database->entity_get(entity_index);

			ASSERT(gamestate_index != NONE);

			entity->gamestate_index = gamestate_index;
			simulation_gamestate_entity_set_simulation_entity_index(entity->gamestate_index, entity->entity_index);
		}
		else
		{
			event(
				_event_error,
				"simulation:entities: failed to create entity (type %d/%s object [0x%08x])",
				entity_type,
				simulation_entity_type_get_name(entity_type),
				object_index
			);
		}
	}

	return entity_index;
}

void simulation_entity_update(int32 entity_index, int32 object_index, uint32 flags)
{
	c_simulation_world* world = simulation_get_world();
	
	if (world->is_distributed() && world->is_authority())
	{
		c_simulation_entity_database* entity_database= world->get_entity_database();

		if (entity_database->entity_is_local(entity_index))
		{
			entity_database->entity_update(entity_index, flags, false);
		}
	}

	return;
}

void simulation_entity_force_update(int32 entity_index, int32 object_index, uint32 flags)
{
	c_simulation_world* world = simulation_get_world();

	if (world->is_distributed() && world->is_authority())
	{
		c_simulation_entity_database* entity_database = world->get_entity_database();

		if (entity_database->entity_is_local(entity_index))
		{
			entity_database->entity_update(entity_index, flags, true);
		}
	}

	return;
}

void simulation_entity_delete(
	int32 entity_index,
	int32 object_index,
	int32 gamestate_index)
{
	c_simulation_world* world = simulation_get_world();

	if (world->is_distributed())
	{
		c_simulation_entity_database* entity_database = world->get_entity_database();
		s_simulation_entity* entity = entity_database->entity_get(entity_index);
		
		ASSERT(gamestate_index != NONE);
		ASSERT(entity->gamestate_index == gamestate_index);
		ASSERT(simulation_gamestate_entity_get_object_index(entity->gamestate_index)==object_index);
		ASSERT(entity->exists_in_gameworld);

		if (entity_database->entity_is_local(entity_index))
		{
			entity->gamestate_index = NONE;
			entity->exists_in_gameworld = false;
			entity_database->entity_delete(entity_index);
		}
		else
		{
			if (!simulation_reset_in_progress() && game_in_progress())
			{
				if (object_index==NONE)
				{
					vassert(false, "networking:simulation:entity: game engine entity deleted with non-local entity 0x%08x type %d", entity_index, entity->entity_type);
				}
				else
				{
					object_datum const* object = object_get(object_index);

					vassert(
						false,
						"networking:simulation:entity: object 0x%08x (%s) deleted with non-local entity 0x%08x type %d",
						object_index,
						tag_name_strip_path(tag_get_name(object->definition_index)),
						entity_index,
						entity->entity_type
					);
				}
			}

			entity->gamestate_index = NONE;
			entity->exists_in_gameworld = false;
		}
	}

	return;
}

e_simulation_entity_type __cdecl simulation_entity_type_from_object_creation(
	int32 object_definition_index,
	int32 parent_object_index)
{
	return INVOKE(0x1B9BA2, 0x0, simulation_entity_type_from_object_creation, object_definition_index, parent_object_index);
}

e_simulation_entity_type simulation_entity_type_from_game_engine(
	void)
{
	return game_engine_globals_get_simulation_entity_type();
}

char const* simulation_entity_type_get_name(
	e_simulation_entity_type entity_type)
{
	c_simulation_world* world = simulation_get_world();
	c_simulation_entity_database const* entity_database = world->get_entity_database();
	char const* entity_type_name = entity_database->get_entity_type_name(entity_type);
	
	return entity_type_name;
}

int32 simulation_entity_get_gamestate_index(
	int32 entity_index)
{
	int32 gamestate_index = NONE;

	if (entity_index != NONE)
	{
		c_simulation_world* world = simulation_get_world();
		{
			c_simulation_entity_database* entity_database = world->get_entity_database();
			s_simulation_entity const* entity = entity_database->entity_try_and_get(entity_index);

			if (entity)
			{
				gamestate_index = entity->gamestate_index;
			}
		}
	}

	return gamestate_index;
}
