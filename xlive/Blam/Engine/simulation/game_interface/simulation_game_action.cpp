#include "stdafx.h"
#include "simulation_game_action.h"

#include "simulation_game_entities.h"

#include "cache/cache_files.h"
#include "game/game.h"
#include "game/game_engine_simulation.h"
#include "networking/network_event.h"
#include "objects/objects.h"
#include "physics/breakable_surfaces.h"
#include "simulation/simulation.h"
#include "simulation/simulation_gamestate_entities.h"
#include "simulation/simulation_world.h"
#include "tag_files/tag_files.h"

/* prototypes */

static void simulation_action_object_create_build_entity_types(
	int32 object_index,
	int32 parent_object_index,
	int32 maximum_entity_count,
	int32* out_entity_count,
	e_simulation_entity_type* out_entity_types,
	int32* out_entity_object_indices);

/* globals */

static uintptr_t p_simulation_action_object_create;
static uintptr_t p_simulation_action_object_update;
static uintptr_t p_simulation_action_game_engine_player_update;
static uintptr_t p_simulation_action_game_engine_globals_update;
static uintptr_t p_simulation_action_breakable_surfaces_delete;
static uintptr_t p_simulation_action_game_statborg_update;

/* public code */

void simulation_game_action_apply_patches(
	void)
{
	DETOUR_ATTACH(p_simulation_action_object_create, Memory::GetAddress(0x1B8D14, 0x1B2C44), simulation_action_object_create);
	DETOUR_ATTACH(p_simulation_action_object_update, Memory::GetAddress(0x1B6685), simulation_action_object_update);
	DETOUR_ATTACH(p_simulation_action_game_engine_player_update, Memory::GetAddress(0x1B6662), simulation_action_game_engine_player_update);
	DETOUR_ATTACH(p_simulation_action_game_engine_globals_update, Memory::GetAddress(0x1B65B7), simulation_action_game_engine_globals_update);
	DETOUR_ATTACH(p_simulation_action_breakable_surfaces_delete, Memory::GetAddress(0xB0BD8), simulation_action_breakable_surfaces_delete);
	DETOUR_ATTACH(p_simulation_action_game_statborg_update, Memory::GetAddress(0x1B6609), simulation_action_game_statborg_update);

	PatchCall(Memory::GetAddress(0x6FF5B, 0x6EB53), simulation_action_game_engine_player_create);
	PatchCall(Memory::GetAddress(0x75148, 0x72240), simulation_action_game_engine_player_create);
	PatchCall(Memory::GetAddress(0x6FF4E), simulation_action_game_engine_globals_create);
	PatchCall(Memory::GetAddress(0x75126), simulation_action_game_engine_globals_create);
	PatchCall(Memory::GetAddress(0x6FF53), simulation_action_game_statborg_create);
	PatchCall(Memory::GetAddress(0x7516B), simulation_action_game_statborg_create);
	PatchCall(Memory::GetAddress(0x1360BC), simulation_action_object_delete);
	PatchCall(Memory::GetAddress(0x182BC5), simulation_action_object_delete);
	PatchCall(Memory::GetAddress(0x1F325E), simulation_action_object_detach_from_gamestate_and_delete);
	PatchCall(Memory::GetAddress(0xB1E80), simulation_action_breakable_surfaces_create);
	PatchCall(Memory::GetAddress(0xB2003), simulation_action_breakable_surfaces_create);
	
	return;
}

void __cdecl simulation_action_game_engine_globals_create(
	void)
{
	if (game_is_server() && game_is_distributed())
	{
		ASSERT(game_engine_globals_get_gamestate_index()==NONE);

		int32 gamestate_index = simulation_gamestate_entity_create();
		game_engine_globals_set_gamestate_index(gamestate_index);
		simulation_gamestate_entity_set_object_index(gamestate_index, NONE);
		
		if (!game_is_playback())
		{
			e_simulation_entity_type entity_type = simulation_entity_type_from_game_engine();

			if (entity_type != k_simulation_entity_type_none)
			{
				int32 entity_index = simulation_entity_create(entity_type, NONE, gamestate_index);
				
				if (entity_index!=NONE)
				{
					c_simulation_world* world = simulation_get_world();
					c_simulation_entity_database* entity_database = world->get_entity_database();
					s_simulation_entity const* entity = entity_database->entity_get(entity_index);

					ASSERT(entity->gamestate_index != NONE);

					entity_database->entity_capture_creation_data(entity_index);
				}
			}
		}
	}

	return;
}

void __cdecl simulation_action_game_engine_globals_update(
	uint32 flags)
{
	if (game_is_server() && game_is_distributed() && !game_is_playback())
	{
		int32 gamestate_index = game_engine_globals_get_gamestate_index();
		
		if (gamestate_index==NONE)
		{
			if (game_is_available())
			{
				event(_event_warning, "networking:simulation:action: game engine globals does not have gamestate to update?");
			}
		}
		else
		{
			int32 entity_index = simulation_gamestate_entity_get_simulation_entity_index(gamestate_index);

			if (entity_index==NONE)
			{
				event(_event_warning, "networking:simulation:action: game engine globals has invalid entity index (gamestate 0x%8X) can't update", gamestate_index);
			}
			else
			{
				simulation_entity_update(entity_index, NONE, flags);
			}
		}
	}
}

void __cdecl simulation_action_game_engine_globals_delete(
	void)
{
	if (game_is_server() && game_is_distributed())
	{
		int32 gamestate_index = game_engine_globals_get_gamestate_index();
		
		if (gamestate_index==NONE)
		{
			event(_event_warning, "networking:simulation:action game engine globals has invalid gamestate index, can't delete");
		}
		else
		{
			if (!game_is_playback())
			{
				int32 entity_index = simulation_gamestate_entity_get_simulation_entity_index(gamestate_index);
				
				if (entity_index==NONE)
				{
					event(
						_event_warning,
						"networking:simulation:action game engine globals [gamestate index 0x%8X] has no entity to delete",
						gamestate_index
					);
				}
				else
				{
					simulation_entity_delete(entity_index, NONE, gamestate_index);
				}
			}

			simulation_gamestate_entity_delete(gamestate_index);
		}

		game_engine_globals_set_gamestate_index(NONE);
	}

	return;
}

void __cdecl simulation_action_game_statborg_create(
	void)
{
	if (game_is_server() && game_is_distributed())
	{
		ASSERT(game_engine_globals_get_statborg_gamestate_index() == NONE);
		
		int32 gamestate_index = simulation_gamestate_entity_create();

		game_engine_globals_set_statborg_gamestate_index(gamestate_index);
		simulation_gamestate_entity_set_object_index(gamestate_index, NONE);

		if (!game_is_playback())
		{
			int32 entity_index = simulation_entity_create(_simulation_entity_type_game_statborg, NONE, gamestate_index);
			
			if (entity_index!=NONE)
			{
				c_simulation_world* world = simulation_get_world();
				c_simulation_entity_database* entity_database = world->get_entity_database();
				s_simulation_entity const* entity = entity_database->entity_get(entity_index);

				ASSERT(entity->gamestate_index != NONE);

				entity_database->entity_capture_creation_data(entity_index);
			}
		}
	}

	return;
}

void __cdecl simulation_action_game_statborg_update(
	uint32 flags)
{
	if (game_is_server() && game_is_distributed() && !game_is_playback())
	{
		int32 gamestate_index = game_engine_globals_get_statborg_gamestate_index();

		if (gamestate_index==NONE)
		{
			event(_event_error, "networking:simulation:action: statborg does not have valid gamestate index to update");
		}
		else
		{
			int32 entity_index = simulation_gamestate_entity_get_simulation_entity_index(gamestate_index);
			
			if (entity_index==NONE)
			{
				event(
					_event_warning,
					"networking:simulation:action: statborg has invalid entity, can't update (gamestate 0x%8X)",
					gamestate_index
				);
			}
			else
			{
				simulation_entity_update(entity_index, NONE, flags);
			}
		}
	}

	return;
}

void simulation_action_game_statborg_delete(
	void)
{
	if (game_is_server() && game_is_distributed())
	{
		int32 gamestate_index = game_engine_globals_get_statborg_gamestate_index();
		
		if (gamestate_index==NONE)
		{
			event(_event_warning, "networking:simulation:action: statborg has invalid gamestate index, cannot delete");
		}
		else
		{
			if (!game_is_playback())
			{
				int32 entity_index = simulation_gamestate_entity_get_simulation_entity_index(gamestate_index);
				
				if (entity_index==NONE)
				{
					event(
						_event_warning, 
						"networking:simulation:action: statborg gamestate index 0x%8X not attached to entity",
						gamestate_index
					);
				}
				else
				{
					simulation_entity_delete(entity_index, NONE, gamestate_index);
				}
			}

			simulation_gamestate_entity_delete(gamestate_index);
		}

		game_engine_globals_set_statborg_gamestate_index(NONE);
	}

	return;
}


void __cdecl simulation_action_game_engine_player_create(
	int16 player_absolute_index)
{
	if (game_is_server() && game_is_distributed())
	{
		ASSERT(game_engine_globals_get_player_gamestate_index(player_absolute_index) == NONE);

		int32 gamestate_index = simulation_gamestate_entity_create();

		game_engine_globals_set_player_gamestate_index(player_absolute_index, gamestate_index);
		simulation_gamestate_entity_set_object_index(gamestate_index, NONE);

		if (!game_is_playback())
		{
			int32 entity_index = simulation_entity_create(_simulation_entity_type_game_engine_player, NONE, gamestate_index);

			if (entity_index != NONE)
			{
				c_simulation_world* world = simulation_get_world();
				c_simulation_entity_database* entity_database = world->get_entity_database();
				s_simulation_entity* entity = entity_database->entity_get(entity_index);

				ASSERT(entity->gamestate_index != NONE);

				entity_database->entity_capture_creation_data(entity_index);
			}
		}
	}

	return;
}

void __cdecl simulation_action_game_engine_player_update(
	int16 player_absolute_index,
	uint32 flags)
{
	//INVOKE(0x1B6662, 0x1B0592, simulation_action_game_engine_player_update, player_index, update_mask);

	if (game_is_server() && game_is_distributed() && !game_is_playback())
	{
		int32 gamestate_index = game_engine_globals_get_player_gamestate_index(player_absolute_index);

		if (gamestate_index == NONE)
		{
			if (game_is_available())
			{
				event(_event_error, "networking:simulation:action: failed to update player %d not attached to gamestate", player_absolute_index);
			}
		}
		else
		{
			int32 entity_index = simulation_gamestate_entity_get_simulation_entity_index(gamestate_index);

			if (entity_index != NONE)
			{
				simulation_entity_update(entity_index, NONE, flags);
			}
			else
			{
				event(
					_event_warning,
					"networking:simulation:action: failed to update player %d gamestate 0x%8X not attached to entity",
					player_absolute_index,
					gamestate_index
				);
			}
		}
	}

	return;
}

void simulation_action_game_engine_player_delete(
	int16 player_absolute_index)
{
	if (game_is_server() && game_is_distributed())
	{
		int32 gamestate_index = game_engine_globals_get_player_gamestate_index(player_absolute_index);

		if (gamestate_index == NONE)
		{
			event(_event_warning, "networking:simulation:action: global player %d has no gamestate representation", player_absolute_index);
		}
		else
		{
			if (!game_is_playback())
			{
				int32 entity_index = simulation_gamestate_entity_get_simulation_entity_index(gamestate_index);

				if (entity_index == NONE)
				{
					event(
						_event_warning,
						"networking:simulation:action: global player %d gamestate index 0x%8X not attached to entity",
						player_absolute_index,
						gamestate_index
					);
				}
				else
				{
					simulation_entity_delete(entity_index, NONE, gamestate_index);
				}
			}

			simulation_gamestate_entity_delete(gamestate_index);
		}

		game_engine_globals_set_player_gamestate_index(player_absolute_index, NONE);
	}

	return;
}

void __cdecl simulation_action_object_create(
	int32 object_index)
{
	//INVOKE(0x1B8D14, 0x1B2C44, simulation_action_object_create, object_index);
	
	if (game_is_server() && game_is_distributed())
	{
		e_simulation_entity_type entity_types[4];
		int32 entity_object_indices[4];

		int32 entity_count = 0;

		simulation_action_object_create_build_entity_types(
			object_index,
			NONE,
			NUMBEROF(entity_object_indices),
			&entity_count,
			entity_types,
			entity_object_indices
		);

		ASSERT(entity_count<=NUMBEROF(entity_object_indices));
	
		if (entity_count>0)
		{
			for (int32 creation_index = 0; creation_index<entity_count; ++creation_index)
			{
				int32 entity_object_index = entity_object_indices[creation_index];

				ASSERT(entity_object_index != NONE);
				ASSERT(object_get(entity_object_index)->object.gamestate_index == NONE);

				int32 gamestate_index = simulation_gamestate_entity_create();
				
				object_attach_gamestate_entity(entity_object_index, gamestate_index);
				simulation_gamestate_entity_set_object_index(gamestate_index, entity_object_index);
				
				if (!game_is_playback())
				{
					int32 entity_index = simulation_entity_create(entity_types[creation_index], entity_object_indices[creation_index], gamestate_index);

					if (entity_index!=NONE)
					{
						c_simulation_entity_database* entity_database = simulation_get_world()->get_entity_database();
						object_datum const* object = object_get(entity_object_index);
						object_header_datum const* object_header = object_header_get(entity_object_index);
						s_simulation_entity const* entity = entity_database->entity_get(entity_index);

						ASSERT(!object_header->flags.test(_object_header_being_deleted_bit));


						ASSERT(entity->gamestate_index != NONE);

						event(
							_event_status,
							"networking:simulation:objects: %s '%s' index 0x%08X created entity type %d/%s index 0x%8X",
							object_type_get_name(object->object.object_identifier.get_type()),
							tag_name_strip_path(tag_get_name(object->definition_index)),
							entity_object_index,
							entity_types[creation_index],
							simulation_entity_type_get_name(entity_types[creation_index]),
							entity_index
						);

						entity_database->entity_capture_creation_data(entity_index);
					}
					else
					{
						event(
							_event_error,
							"networking:simulation:action: object entity creation failed! 0x%8X/%d",
							entity_object_indices[creation_index],
							entity_types[creation_index]
						);
					}
				}
			}
		}
	}

	return;
}

void __cdecl simulation_action_object_update(
	int32 object_index,
	uint32 update_mask)
{
	if (game_is_distributed() && game_is_server() && !game_is_playback())
	{
		object_datum const* object = object_get(object_index);

		if (object->object.gamestate_index!=NONE)
		{
			int32 entity_index = simulation_gamestate_entity_get_simulation_entity_index(object->object.gamestate_index);;
		
			if (entity_index!=NONE)
			{
				simulation_entity_update(entity_index, object_index, update_mask);
			}
		}
	}

	return;
}

void __cdecl simulation_action_object_force_update(
	int32 object_index,
	uint32 flags)
{
	if (game_is_distributed() && game_is_server() && !game_is_playback())
	{
		object_datum const* object = object_get(object_index);
		
		if (object->object.gamestate_index!=NONE)
		{
			int32 entity_index = simulation_gamestate_entity_get_simulation_entity_index(object->object.gamestate_index);
			
			if (entity_index==NONE)
			{
				event(
					_event_error,
					"networking:simulation:action: failed to get entity index for gamestate 0x%8X (object %s)",
					object->object.gamestate_index,
					/*object_describe(object_index)*/ "N/A"
				);
				
			}
			else
			{
				simulation_entity_force_update(entity_index, object_index, flags);
			}
		}
	}

	return;
}

void __cdecl simulation_action_object_delete(
	int32 object_index)
{
	if (game_is_server() && game_is_distributed())
	{
		object_datum const* object= object_get(object_index);
		
		if (object->object.gamestate_index!=NONE)
		{
			if (!game_is_playback())
			{
				int32 entity_index = simulation_gamestate_entity_get_simulation_entity_index(object->object.gamestate_index);

				if (entity_index!=NONE)
				{
					simulation_entity_delete(entity_index, object_index, object->object.gamestate_index);
				}
				else
				{
					event(
						_event_warning,
						"networking:simulation:action: object 0x%8X gamestate index 0x%8X not attached to entity (can't delete entity)",
						object_index,
						object->object.gamestate_index
					);
				}
			}

			simulation_gamestate_entity_delete(object->object.gamestate_index);
			object_detach_gamestate_entity(object_index, object->object.gamestate_index);
		}
	}

	return;
}

void __cdecl simulation_action_object_detach_from_gamestate_and_delete(
	int32 object_index)
{
	object_datum const* object= object_get(object_index);

	ASSERT(game_is_predicted());

	if (object->object.gamestate_index!=NONE)
	{
		int32 gamestate_index = object->object.gamestate_index;

		object_detach_gamestate_entity(object_index, gamestate_index);
		simulation_gamestate_entity_set_object_index(gamestate_index, NONE);
	}
	else
	{
		event(
			_event_warning,
			"networking:simulation: attempting to detach and delete an object [0x%08X '%s'] that is not attached to a gamestate index?",
			object_index,
			/*object_describe(object_index)*/ "N/A"
		);
	}

	object_delete(object_index);
	
	return;
}

void __cdecl simulation_action_breakable_surfaces_create(
	int32 group_index)
{
	if (game_is_server() && game_is_distributed())
	{
		ASSERT(group_index != NONE);
		ASSERT(breakable_surface_group_get_gamestate_index(NONE, group_index) == NONE);

		int32 gamestate_index = simulation_gamestate_entity_create();

		breakable_surface_group_set_gamestate_index(NONE, group_index, gamestate_index);
		simulation_gamestate_entity_set_object_index(gamestate_index, group_index);

		if (game_is_distributed() && !game_is_playback())
		{
			int32 entity_index = simulation_entity_create(_simulation_entity_type_breakable_surface_group, group_index, gamestate_index);

			if (entity_index!=NONE)
			{
				c_simulation_world* world = simulation_get_world();
				c_simulation_entity_database* entity_database = world->get_entity_database();
				s_simulation_entity* entity = entity_database->entity_get(entity_index);

				ASSERT(entity->gamestate_index != NONE);

				entity_database->entity_capture_creation_data(entity_index);
			}
		}
	}

	return;
}

void __cdecl simulation_action_breakable_surfaces_delete(
	void)
{
	if (game_is_server() && game_is_distributed())
	{
		for (int32 group_index = 0; group_index<k_maximum_distributed_networking_breakable_surface_groups; ++group_index)
		{
			int32 gamestate_index = breakable_surface_gamestate_indices_get()[group_index];

			if (gamestate_index != NONE)
			{
				if (!game_is_playback())
				{
					int32 entity_index = simulation_gamestate_entity_get_simulation_entity_index(gamestate_index);

					if (entity_index == NONE)
					{
						event(
							_event_warning,
							"networking:simulation:action breakable surface [gamestate index 0x%8X] has no entity to delete",
							gamestate_index
						);
					}
					else
					{
						simulation_entity_delete(entity_index, group_index, gamestate_index);
					}
				}

				simulation_gamestate_entity_delete(gamestate_index);

				breakable_surface_gamestate_indices_get()[group_index] = NONE;
			}
		}
	}

	return;
}

void __cdecl simulation_action_pickup_equipment(int32 unit_datum_index, int32 grenade_tag_index)
{
	INVOKE(0x1B6F12, 0x1B0E42, simulation_action_pickup_equipment, unit_datum_index, grenade_tag_index);
	return;
}

/* private code */

static void simulation_action_object_create_build_entity_types(
	int32 object_index,
	int32 parent_object_index,
	int32 maximum_entity_count,
	int32* out_entity_count,
	e_simulation_entity_type* out_entity_types,
	int32* out_entity_object_indices)
{
	object_datum const* object = object_get(object_index);
	object_header_datum const* object_header = object_header_get(object_index);

	if (!object_header->flags.test(_object_header_being_deleted_bit) && object->object.gamestate_index==NONE)
	{
		e_simulation_entity_type entity_type = simulation_entity_type_from_object_creation(object->definition_index, parent_object_index);

		if (entity_type != k_simulation_entity_type_none && *out_entity_count < maximum_entity_count)
		{
			ASSERT(VALID_INDEX(*out_entity_count, maximum_entity_count));

			out_entity_types[*out_entity_count] = entity_type;
			out_entity_object_indices[(*out_entity_count)++] = object_index;

			object_datum const* child_object;
			for (int32 child_object_index = object->object.first_child_object_index; child_object_index != NONE; child_object_index = child_object->object.next_object_index)
			{
				child_object = object_get(child_object_index);
				
				if (object->object.flags.test(_object_created_with_parent_bit))
				{
					simulation_action_object_create_build_entity_types(
						child_object_index,
						object_index,
						maximum_entity_count,
						out_entity_count,
						out_entity_types,
						out_entity_object_indices
					);
				}
			}
		}
	}

	return;
}
