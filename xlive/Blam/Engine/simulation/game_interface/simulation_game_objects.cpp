#include "stdafx.h"
#include "simulation_game_objects.h"

#include "simulation_game_interface.h"
#include "simulation_game_internal.h"
#include "simulation_game_object_constants.h"

#include "cache/cache_files.h"
#include "game/game.h"
#include "memory/bitstream.h"
#include "models/models.h"
#include "networking/network_event.h"
#include "objects/objects.h"
#include "objects/object_definition.h"
#include "scenario/scenario.h"
#include "simulation/simulation.h"
#include "simulation/simulation_encoding.h"
#include "simulation/simulation_entity_definition.h"
#include "simulation/simulation_gamestate_entities.h"
#include "simulation/simulation_type_collection.h"
#include "simulation/simulation_world.h"
#include "tag_files/tag_files.h"

/* constants */

static real32 const k_object_shield_vitality_maximum = 3.f;
static real32 const k_object_shield_vitality_minimum = 0.f;
static real32 const k_object_translational_velocity_magnitude_maximum = 350.f;
static real32 const k_object_translational_velocity_magnitude_minimum = 0.03f;
static real32 const k_object_angular_velocity_magnitude_maximum = 30.f;
static real32 const k_object_angular_velocity_magnitude_minimum = 0.03f;
static real32 const k_object_body_vitality_maximum = 1.f;
static real32 const k_object_body_vitality_minimum = -1.f;
static real32 const k_object_scale_maximum = 10.f;

/* prototypes */

CLASS_HOOK_DECLARE_LABEL(c_simulation_object_entity_definition__build_creation_data, c_simulation_object_entity_definition::build_creation_data);
static __declspec(naked) void jmp_c_simulation_object_entity_definition__build_creation_data(void)
{
	CLASS_HOOK_JMP(c_simulation_object_entity_definition__build_creation_data, c_simulation_object_entity_definition::build_creation_data);
}

CLASS_HOOK_DECLARE_LABEL(c_simulation_object_entity_definition__build_updated_state_data, c_simulation_object_entity_definition::build_updated_state_data);
static __declspec(naked) void jmp_c_simulation_object_entity_definition__build_updated_state_data(void)
{
	CLASS_HOOK_JMP(c_simulation_object_entity_definition__build_updated_state_data, c_simulation_object_entity_definition::build_updated_state_data);
}

CLASS_HOOK_DECLARE_LABEL(c_simulation_object_entity_definition__create_game_entity, c_simulation_object_entity_definition::create_game_entity);
static __declspec(naked) void jmp_c_simulation_object_entity_definition__create_game_entity(void)
{
	CLASS_HOOK_JMP(c_simulation_object_entity_definition__create_game_entity, c_simulation_object_entity_definition::create_game_entity);
}

CLASS_HOOK_DECLARE_LABEL(c_simulation_object_entity_definition__update_game_entity, c_simulation_object_entity_definition::update_game_entity);
static __declspec(naked) void jmp_c_simulation_object_entity_definition__update_game_entity(void)
{
	CLASS_HOOK_JMP(c_simulation_object_entity_definition__update_game_entity, c_simulation_object_entity_definition::update_game_entity);
}

CLASS_HOOK_DECLARE_LABEL(c_simulation_object_entity_definition__delete_game_entity, c_simulation_object_entity_definition::delete_game_entity);
static __declspec(naked) void jmp_c_simulation_object_entity_definition__delete_game_entity(void)
{
	CLASS_HOOK_JMP(c_simulation_object_entity_definition__delete_game_entity, c_simulation_object_entity_definition::delete_game_entity);
}

CLASS_HOOK_DECLARE_LABEL(c_simulation_object_entity_definition__promote_game_entity_to_authority, c_simulation_object_entity_definition::promote_game_entity_to_authority);
static __declspec(naked) void jmp_c_simulation_object_entity_definition__promote_game_entity_to_authority(void)
{
	CLASS_HOOK_JMP(c_simulation_object_entity_definition__promote_game_entity_to_authority, c_simulation_object_entity_definition::promote_game_entity_to_authority);
}

/*
CLASS_HOOK_DECLARE_LABEL(c_simulation_object_entity_definition__gameworld_attachment_valid, c_simulation_object_entity_definition::gameworld_attachment_valid);
static __declspec(naked) void jmp_c_simulation_object_entity_definition__gameworld_attachment_valid(void)
{
	CLASS_HOOK_JMP(c_simulation_object_entity_definition__gameworld_attachment_valid, c_simulation_object_entity_definition::gameworld_attachment_valid);
}
*/

CLASS_HOOK_DECLARE_LABEL(c_simulation_object_entity_definition__object_build_creation_data, c_simulation_object_entity_definition::object_build_creation_data);
static __declspec(naked) void jmp_c_simulation_object_entity_definition__object_build_creation_data(void)
{
	CLASS_HOOK_JMP(c_simulation_object_entity_definition__object_build_creation_data, c_simulation_object_entity_definition::object_build_creation_data);
}

CLASS_HOOK_DECLARE_LABEL(c_simulation_object_entity_definition__object_setup_placement_data, c_simulation_object_entity_definition::object_setup_placement_data);
static __declspec(naked) void jmp_c_simulation_object_entity_definition__object_setup_placement_data(void)
{
	CLASS_HOOK_JMP(c_simulation_object_entity_definition__object_setup_placement_data, c_simulation_object_entity_definition::object_setup_placement_data);
}

CLASS_HOOK_DECLARE_LABEL(c_simulation_object_entity_definition__object_creation_encode, c_simulation_object_entity_definition::object_creation_encode);
static __declspec(naked) void jmp_c_simulation_object_entity_definition__object_creation_encode(void)
{
	CLASS_HOOK_JMP(c_simulation_object_entity_definition__object_creation_encode, c_simulation_object_entity_definition::object_creation_encode);
}

CLASS_HOOK_DECLARE_LABEL(c_simulation_object_entity_definition__object_creation_decode, c_simulation_object_entity_definition::object_creation_decode);
static __declspec(naked) void jmp_c_simulation_object_entity_definition__object_creation_decode(void)
{
	CLASS_HOOK_JMP(c_simulation_object_entity_definition__object_creation_decode, c_simulation_object_entity_definition::object_creation_decode);
}

// Ensure we aren't sending a variant index that's the same as the default variant
// Inefficient since the variant will be set to it regardless
// If we sync the variant index of the "default" variant this can also cause an issue where the default dialouge isn't selected but that variants specific dialouge
static bool simulation_object_variant_should_sync(const s_simulation_object_creation_data* creation_data);

static int32 __stdcall c_simulation_object_entity_definition__object_creation_required_bits(void* _this);

/* globals */

static uintptr_t p_c_simulation_unit_entity_definition_encode;
static uintptr_t p_c_simulation_unit_entity_definition_decode;
static uintptr_t p_c_simulation_object_entity_definition__object_build_creation_data;
static uintptr_t p_c_simulation_object_entity_definition__build_creation_data;
static uintptr_t p_c_simulation_object_entity_definition__create_game_entity;
static uintptr_t p_c_simulation_object_entity_definition__update_game_entity;
static uintptr_t p_c_simulation_object_entity_definition__delete_game_entity;
static uintptr_t p_c_simulation_object_entity_definition__build_updated_state_data;
static uintptr_t p_c_simulation_object_entity_definition__object_setup_placement_data;
static uintptr_t p_c_simulation_object_entity_definition__object_creation_encode;
static uintptr_t p_c_simulation_object_entity_definition__object_creation_decode;
static uintptr_t p_c_simulation_object_entity_definition__promote_game_entity_to_authority;

static uintptr_t p_simulation_object_get_replicated_object_from_entity;

/* public code */

void simulation_game_objects_apply_patches(
	void)
{
	DetourClassFunc(Memory::GetAddress<uint8*>(0x1F27D1, 0x1DD86A), (uint8*)c_simulation_object_entity_definition__object_creation_required_bits, 8);
	DETOUR_ATTACH(p_c_simulation_object_entity_definition__object_build_creation_data, Memory::GetAddress(0x1F24ED, 0x1DD586), jmp_c_simulation_object_entity_definition__object_build_creation_data);
	DETOUR_ATTACH(p_c_simulation_object_entity_definition__build_creation_data, Memory::GetAddress(0x1F2325), jmp_c_simulation_object_entity_definition__build_creation_data);
	DETOUR_ATTACH(p_c_simulation_object_entity_definition__create_game_entity, Memory::GetAddress(0x1F2397), jmp_c_simulation_object_entity_definition__create_game_entity);
	DETOUR_ATTACH(p_c_simulation_object_entity_definition__update_game_entity, Memory::GetAddress(0x1F242D), jmp_c_simulation_object_entity_definition__update_game_entity);
	DETOUR_ATTACH(p_c_simulation_object_entity_definition__delete_game_entity, Memory::GetAddress(0x1F2459), jmp_c_simulation_object_entity_definition__delete_game_entity);
	DETOUR_ATTACH(p_c_simulation_object_entity_definition__build_updated_state_data, Memory::GetAddress(0x1F2337), jmp_c_simulation_object_entity_definition__build_updated_state_data);
	DETOUR_ATTACH(p_c_simulation_object_entity_definition__object_setup_placement_data, Memory::GetAddress(0x1F2704, 0x1DD79D), jmp_c_simulation_object_entity_definition__object_setup_placement_data);
	DETOUR_ATTACH(p_c_simulation_object_entity_definition__object_creation_encode, Memory::GetAddress(0x1F3B11, 0x1DEBAA), jmp_c_simulation_object_entity_definition__object_creation_encode);
	DETOUR_ATTACH(p_c_simulation_object_entity_definition__object_creation_decode, Memory::GetAddress(0x1F3BDD, 0x1DEC76), jmp_c_simulation_object_entity_definition__object_creation_decode);
	DETOUR_ATTACH(p_c_simulation_object_entity_definition__promote_game_entity_to_authority, Memory::GetAddress(0x1F24B4), jmp_c_simulation_object_entity_definition__promote_game_entity_to_authority);
	
	DETOUR_ATTACH(p_simulation_object_get_replicated_object_from_entity, Memory::GetAddress(0x1F2211), simulation_object_get_replicated_object_from_entity);

	/*
	WritePointer(Memory::GetAddress(0x3C8E04), jmp_c_simulation_object_entity_definition__gameworld_attachment_valid);
	WritePointer(Memory::GetAddress(0x3C9684), jmp_c_simulation_object_entity_definition__gameworld_attachment_valid);
	WritePointer(Memory::GetAddress(0x3C97E4), jmp_c_simulation_object_entity_definition__gameworld_attachment_valid);
	WritePointer(Memory::GetAddress(0x3C9BEC), jmp_c_simulation_object_entity_definition__gameworld_attachment_valid);
	WritePointer(Memory::GetAddress(0x3CA0BC), jmp_c_simulation_object_entity_definition__gameworld_attachment_valid);
	WritePointer(Memory::GetAddress(0x3CA4DC), jmp_c_simulation_object_entity_definition__gameworld_attachment_valid);
	WritePointer(Memory::GetAddress(0x3CA5B4), jmp_c_simulation_object_entity_definition__gameworld_attachment_valid);
	WritePointer(Memory::GetAddress(0x3CA684), jmp_c_simulation_object_entity_definition__gameworld_attachment_valid);
	*/
	return;
}

int32 __cdecl simulation_object_get_replicated_object_from_entity(
	int32 entity_index)
{
	int32 object_index = NONE;
	
	ASSERT(!game_is_playback());

	if (entity_index!=NONE)
	{
		c_simulation_world* world = simulation_get_world();
		c_simulation_entity_database* entity_database = world->get_entity_database();
		s_simulation_entity const* entity = entity_database->entity_try_and_get(entity_index);

		if (entity)
		{
			c_simulation_type_collection* type_collection = simulation_get_type_collection();
			c_simulation_entity_definition* entity_definition = type_collection->get_entity_definition(entity->entity_type);
			
			if (entity->exists_in_gameworld)
			{
				if (entity->gamestate_index != NONE && entity_definition->entity_type_is_gameworld_object())
				{
					object_index = simulation_gamestate_entity_get_object_index(entity->gamestate_index);
				}
			}
		}
	}

	return object_index;
}

void c_simulation_object_entity_definition::build_creation_data(
	int32 gamestate_index,
	int32 creation_data_size,
	void* out_creation_data)
{
	ASSERT(gamestate_index != NONE);

	int32 object_index = simulation_gamestate_entity_get_object_index(gamestate_index);

	if (object_index != NONE)
	{
		object_header_datum const* object_header = object_header_get(object_index);

		ASSERT(!object_header->flags.test(_object_header_being_deleted_bit));

		build_object_creation_data(object_index, creation_data_size, out_creation_data);
	}
	else
	{
		event(
			_event_error,
			"networking:simulation:objects: gamestate 0x%8X not attached to object, can't build creation data",
			gamestate_index
		);
	}


	return;
}

bool c_simulation_object_entity_definition::build_updated_state_data(
	s_simulation_entity const* entity,
	uint32* update_mask,
	int32 update_state_data_size,
	void* update_state_data)
{
	bool updated_success = false;
	int32 object_index;

	ASSERT(entity);
	ASSERT(update_mask);
	ASSERT(entity->gamestate_index != NONE);

	object_index = simulation_gamestate_entity_get_object_index(entity->gamestate_index);

	if (object_index!=NONE)
	{
		uint32 incoming_update_mask= *update_mask;

		{
			object_datum const* object= object_get(object_index);
			if (object->object.object_identifier.get_type() == _object_type_vehicle)
			{
				incoming_update_mask &= MASK(k_simulation_object_update_flag_count);
			}

			*update_mask = handle_object_update(object_index, incoming_update_mask, update_state_data_size, update_state_data);
			updated_success = true;
		}
	}
	else
	{
		event(_event_error, "networking:simulation:objects: gamestate 0x%8X not attached to object, can't build updated state data", entity->gamestate_index);
	}

	return updated_success;
}

bool c_simulation_object_entity_definition::create_game_entity(
	int32 gamestate_index,
	int32 creation_data_size,
	void const* creation_data,
	uint32 initial_update_mask,
	int32 initial_state_data_size,
	void const* initial_state_data)
{
	uint32 pending_update_mask = initial_update_mask;
	bool created = false;;
	int32 object_index = NONE;

	ASSERT(gamestate_index!=NONE);
	ASSERT(simulation_gamestate_entity_get_object_index(gamestate_index) == NONE);
	
	ASSERT((pending_update_mask&~initial_update_mask)==0);

	object_index = create_object(creation_data_size, creation_data, &initial_update_mask, initial_state_data_size, initial_state_data);

	if (object_index!=NONE)
	{
		object_datum* object = object_get(object_index);
		object_header_datum* object_header = object_header_get(object_index);

		if (object_header->flags.test(_object_header_being_deleted_bit))
		{
			event(
				_event_error,
				"networking:simulation:entities:objects: create_game_entity created an object but it was immediately deleted (entity type-%d %s)",
				entity_type(),
				tag_name_strip_path(tag_get_name(object->definition_index))
			);
		}
		else
		{
			simulation_gamestate_entity_set_object_index(gamestate_index, object_index);
			object_attach_gamestate_entity(object_index, gamestate_index);
			created = true;

			if (pending_update_mask)
			{
				apply_object_update(object_index, initial_update_mask, initial_state_data_size, initial_state_data);
			}
		}

	}
	else
	{
		event(_event_error, "networking:simulation:entities:objects: failed to create object for gamestate 0x%08X", gamestate_index);
	}


	return created;
}

bool c_simulation_object_entity_definition::update_game_entity(
	int32 gamestate_index,
	uint32 update_mask,
	int32 update_state_data_size,
	void const* update_state_data)
{
	int32 object_index;

	bool updated_object = false;

	ASSERT(update_mask!=0);
	ASSERT(gamestate_index != NONE);

	object_index= simulation_gamestate_entity_get_object_index(update_mask);
	
	if (object_index==NONE)
	{
		event(_event_error, "networking:simulation:entities:objects: gamestate 0x%08X not attached to object, can't update", update_mask);
	}
	// TODO: finish this
	else if (/*gameworld_attachment_valid(gamestate_index)*/ true)
	{
		apply_object_update(object_index, update_mask, update_state_data_size, update_state_data);
		updated_object = true;
	}
	else
	{
		event(_event_error, "networking:simulation:objects: object 0x%8X not attached properly to gamestate 0x%8X (update)", object_index, gamestate_index);
	}

	return updated_object;
}

bool c_simulation_object_entity_definition::delete_game_entity(
	int32 gamestate_index)
{
	bool handled_deletion= false;

	ASSERT(gamestate_index != NONE);

	int32 object_index= simulation_gamestate_entity_get_object_index(gamestate_index);

	if (object_index==NONE)
	{
		event(_event_error, "networking:simulation:objects: failed to get object index for gamestate 0x%8X (deletion)", gamestate_index);
	}
	// TODO: finish this
	else if (/*gameworld_attachment_valid(gamestate_index)*/ true)
	{
		if (handle_delete_object(object_index))
		{
			object_datum* object = object_get(object_index);

			ASSERT(object->object.gamestate_index == NONE);
		}
		else
		{
			object_detach_gamestate_entity(object_index, gamestate_index);
			object_delete(object_index);
		}
	}
	else
	{
		event(
			_event_error,
			"networking:simulation:objects: object 0x%8X not attached properly to gamestate 0x%8X (delete)",
			object_index,
			gamestate_index
		);
	}
	

	return handled_deletion;
}

bool c_simulation_object_entity_definition::promote_game_entity_to_authority(
	int32 gamestate_index)
{
	int32 object_index;

	bool promoted = false;

	ASSERT(gamestate_index != NONE);

	object_index = simulation_gamestate_entity_get_object_index(gamestate_index);

	if (object_index==NONE)
	{
		event(_event_error, "networking:simulation:entities:objects: failed to get object attached to gamestate 0x%08X promote to authority", gamestate_index);
	}
	// TODO: finish this
	else if (/*gameworld_attachment_valid(gamestate_index)*/ true)
	{
		promoted = promote_object_to_authority(gamestate_index);
	}
	else
	{
		event(_event_error, "networking:simulation:objects: object 0x%8X not attached properly to gamestate 0x%8X (promotion)", object_index, gamestate_index);
	}

	return promoted;
}

bool c_simulation_object_entity_definition::entity_type_is_gameworld_object(
	void)
{
	return true;
}

/*
bool c_simulation_object_entity_definition::gameworld_attachment_valid(
	int32 gamestate_index)
{
	int32 object_index_attached_to_gamestate;

	bool attachment_valid = false;

	ASSERT(gamestate_index != NONE);

	object_index_attached_to_gamestate = simulation_gamestate_entity_get_object_index(gamestate_index);

	if (object_index_attached_to_gamestate!=NONE)
	{
		object_datum const* object = object_try_and_get(object_index_attached_to_gamestate);
		
		if (object)
		{
			if (object->object.gamestate_index == gamestate_index)
			{
				attachment_valid = true;
			}
			else
			{
				event(
					_event_error,
					"networking:simulation:objects: gamestate 0x%8X not attached properly to object 0x%8X (sim 0x%8X)",
					gamestate_index,
					object_index_attached_to_gamestate,
					object->object.gamestate_index
				);
			}
		}
		else
		{
			event(
				_event_error,
				"networking:simulation:objects: gamestate 0x%8X attached to bad object index 0x%8X",
				gamestate_index,
				object_index_attached_to_gamestate
			);
		}
	}

	return attachment_valid;
}
*/

// Builds creation data for objects
void c_simulation_object_entity_definition::object_build_creation_data(int32 object_index, s_simulation_object_creation_data* creation_data)
{
	const object_datum* object = object_get(object_index);

	creation_data->object_definition_index = object->definition_index;
	creation_data->scenario_datum_index = object->object.placement_index;
	creation_data->multiplayer_spawn_monitor_index = object->object.netgame_equipment_index;
	creation_data->model_variant_index = object->object.variant_index;
	creation_data->emblem_info = object->object.emblem_info;

	return;
}

bool c_simulation_object_entity_definition::object_setup_placement_data(
	struct s_simulation_object_creation_data const* object_creation_data,
	struct s_simulation_object_state_data const* object_state_data,
	uint32* initial_update_mask,
	struct object_placement_data* placement_data)
{
	bool result = false;

	ASSERT(object_creation_data);
	ASSERT(placement_data);

	if (object_creation_data->scenario_datum_index == NONE)
	{
		object_placement_data_new(placement_data, object_creation_data->object_definition_index, NONE, NULL);
		SET_BIT(placement_data->flags, 1, true);
		SET_BIT(placement_data->flags, 4, true);
		placement_data->emblem_info = object_creation_data->emblem_info;
		
		if (TEST_BIT(*initial_update_mask, _simulation_object_update_position_bit))
		{
			placement_data->position = object_state_data->relative_position;
			SET_BIT(*initial_update_mask, _simulation_object_update_position_bit, false);
		}

		if (TEST_BIT(*initial_update_mask, _simulation_object_update_forward_and_up_bit))
		{
			placement_data->forward = object_state_data->forward;
			placement_data->up = object_state_data->up;
			SET_BIT(*initial_update_mask, _simulation_object_update_forward_and_up_bit, false);
		}

		if (TEST_BIT(*initial_update_mask, _simulation_object_update_scale_bit))
		{
			placement_data->scale = object_state_data->scale;
			SET_BIT(*initial_update_mask, _simulation_object_update_scale_bit, false);
		}

		if (TEST_BIT(*initial_update_mask, _simulation_object_update_translational_velocity_bit))
		{
			placement_data->translational_velocity = object_state_data->translational_velocity;
			SET_BIT(*initial_update_mask, _simulation_object_update_translational_velocity_bit, false);
		}

		if (TEST_BIT(*initial_update_mask, _simulation_object_update_angular_velocity_bit))
		{
			placement_data->angular_velocity = object_state_data->angular_velocity;
			SET_BIT(*initial_update_mask, _simulation_object_update_angular_velocity_bit, false);
		}

		// Set variant of the object
		if (object_creation_data->model_variant_index != NONE && object_creation_data->object_definition_index != NONE)
		{
			const object_definition* object_def = (object_definition*)tag_get_fast(object_creation_data->object_definition_index);
			const datum object_model_index = object_def->object.model.index;

			if (object_model_index != NONE)
			{
				s_model_definition* model_def = (s_model_definition*)tag_get_fast(object_model_index);

				if (object_creation_data->model_variant_index < model_def->variants.count)
				{
					const s_model_variant* variant = TAG_BLOCK_GET_ELEMENT(&model_def->variants, object_creation_data->model_variant_index, s_model_variant);

					placement_data->variant_name = variant->name;
				}
			}
		}

		result = true;
	}

	return result;
}

int32 c_simulation_object_entity_definition::object_create_object(
	s_simulation_object_creation_data const *object_creation_data,
	s_simulation_object_state_data const *object_state_data,
	uint32* initial_update_mask,
	object_placement_data* placement_data)
{
	int32 object_index = INVOKE_TYPE(
		0x1F32DB,
		0x1DE374,
		int32(__thiscall*)(c_simulation_object_entity_definition*, s_simulation_object_creation_data const*, s_simulation_object_state_data const*, uint32*, object_placement_data*),
		this,
		object_creation_data,
		object_state_data,
		initial_update_mask,
		placement_data
	);

	return object_index;
}

void c_simulation_object_entity_definition::object_creation_encode(
	s_simulation_object_creation_data const* object_creation_data,
	class c_bitstream* packet,
	bool encode_for_network)
{
	ASSERT(object_creation_data);
	ASSERT(packet);

	packet->push_structure("object-creation", NONE, 0);
	
	simulation_write_definition_index("object-definition-index", packet, object_creation_data->object_definition_index);

	packet->write_bool("object-scenario-datum-index-exists", object_creation_data->scenario_datum_index!=NONE);
	
	if (object_creation_data->scenario_datum_index!=NONE)
	{
		packet->write_integer("object-scenario-datum-index", object_creation_data->scenario_datum_index, 13);
	}

	packet->write_integer("multiplayer-spawn-monitor-index", object_creation_data->multiplayer_spawn_monitor_index+1, 7);
	
	if (object_creation_data->emblem_info.foreground_emblem ||
		object_creation_data->emblem_info.background_emblem ||
		object_creation_data->emblem_info.emblem_flags.get_unsafe())
	{
		packet->write_bool("emblem-info-exists", true);
		packet->write_integer("emblem-info-foreground-index", object_creation_data->emblem_info.foreground_emblem, 6);
		packet->write_integer("emblem-info-background-index", object_creation_data->emblem_info.background_emblem, 6);
		packet->write_integer("emblem-info-flags", object_creation_data->emblem_info.emblem_flags.get_unsafe(), 4);
	}
	else
	{
		packet->write_bool("emblem-info-exists", false);
	}

	bool model_variant_id_exists = simulation_object_variant_should_sync(object_creation_data);

	packet->write_bool("model-variant-index-exists", model_variant_id_exists);

	if (model_variant_id_exists)
	{
		packet->write_integer("model-variant-index", object_creation_data->model_variant_index, 6);    // 6 bits since k_maximum_variants_per_model is 64
	}

	return;
}

bool c_simulation_object_entity_definition::object_creation_decode(
	s_simulation_object_creation_data* object_creation_data,
	c_bitstream* packet,
	bool decode_for_network)
{
	bool decode_success;

	ASSERT(object_creation_data);
	ASSERT(packet);

	packet->push_structure("object-creation", NONE, 0);

	object_creation_data->object_definition_index = simulation_read_definition_index("object-definition-index", packet);

	if (packet->read_bool("object-scenario-datum-index-exists"))
	{
		object_creation_data->scenario_datum_index = packet->read_integer("object-scenario-datum-index", 13);
	}
	else
	{
		object_creation_data->scenario_datum_index = NONE;
	}

	object_creation_data->multiplayer_spawn_monitor_index = (int8)packet->read_integer("multiplayer-spawn-monitor-index", 7) - 1;

	if (packet->read_bool("emblem-info-exists"))
	{
		object_creation_data->emblem_info.foreground_emblem = (e_emblem_foreground)packet->read_integer("emblem-info-foreground-index", 6);
		object_creation_data->emblem_info.background_emblem = (e_emblem_background)packet->read_integer("emblem-info-background-index", 6);
		object_creation_data->emblem_info.emblem_flags.set_unsafe((uint8)packet->read_integer("emblem-info-flags", 4));
	}
	else
	{
		object_creation_data->emblem_info.foreground_emblem = _emblem_foreground_seventh_column;
		object_creation_data->emblem_info.background_emblem = _emblem_background_solid;
		object_creation_data->emblem_info.emblem_flags.clear();
	}

	if (packet->read_bool("model-variant-index-exists"))
	{
		object_creation_data->model_variant_index = (int8)packet->read_integer("model-variant-index", 6);    // 6 bits since k_maximum_variants_per_model is 64
	}
	else
	{
		bool variant_block_valid = false;

		struct object_definition const* object_definition = (struct object_definition*)tag_get_fast(object_creation_data->object_definition_index);
		
		// Check to make sure the model variant count associated with the object is valid
		if (object_definition->object.model.index!=NONE)
		{
			s_model_definition* model_definition = (s_model_definition*)tag_get_fast(object_definition->object.model.index);
			
			if (model_definition->variants.count>0)
			{
				variant_block_valid = true;
			}
		}

		// Only set the variant index to NONE if our model doesn't have any variants
		if (!variant_block_valid)
		{
			object_creation_data->model_variant_index = NONE;
		}
	}

	packet->pop_structure("object-creation", NONE);

	decode_success = !packet->overflowed() && object_creation_data->object_definition_index != NONE;

	if (object_creation_data->multiplayer_spawn_monitor_index != NONE)
	{
		decode_success = decode_success && VALID_INDEX(object_creation_data->multiplayer_spawn_monitor_index, (int8)scenario_netgame_equipment_size());
	}

	return decode_success;
}

bool c_simulation_object_entity_definition::object_update_encode(
	bool initial_update,
	uint32 update_mask,
	uint32* update_mask_written,
	struct s_simulation_view_telemetry_data const* telemetry_data,
	struct s_simulation_object_state_data const* object_state_data,
	class c_bitstream* packet,
	int32 must_leave_space_bits,
	bool ensure_position_update_quantization_inside_bsp,
	bool encode_for_network)
{
	bool wrote_update = false;
	c_entity_update_encode_helper update;

	ASSERT((update_mask & ~MASK(k_simulation_object_update_flag_count))==0);
	ASSERT(update_mask_written);
	ASSERT((*update_mask_written & MASK(k_simulation_object_update_flag_count))==0);
	ASSERT(packet);

	packet->push_structure("object-update", NONE, 0);

	if (update.make_room_for_update(packet, must_leave_space_bits, 0, k_simulation_object_update_flag_count, update_mask))
	{
		if (update.write_component_header(_simulation_object_update_dead_bit, "dead-exists"))
		{
			packet->write_bool("dead", object_state_data->dead);
		}

		update.finish_component();

		if (update.write_component_header(_simulation_object_update_position_bit, "position-exists"))
		{
			bool position_inside_bsp = initial_update || ensure_position_update_quantization_inside_bsp;

			simulation_write_quantized_position(packet, &object_state_data->relative_position, 16, position_inside_bsp);
		}

		update.finish_component();

		if (update.write_component_header(_simulation_object_update_forward_and_up_bit, "forward-and-up-exists"))
		{
			packet->write_axes("forward-and-up", &object_state_data->forward, &object_state_data->up);
		}
		
		update.finish_component();

		if (update.write_component_header(_simulation_object_update_scale_bit, "scale-exists"))
		{
			packet->write_quantized_real("scale", object_state_data->scale, 0.f, k_object_scale_maximum, 7, false);
		}

		update.finish_component();

		if (update.write_component_header(_simulation_object_update_translational_velocity_bit, "translational-velocity-exists"))
		{
			packet->write_vector("translational-velocity", &object_state_data->translational_velocity, k_object_translational_velocity_magnitude_minimum, k_object_translational_velocity_magnitude_maximum, 10);
		}

		update.finish_component();

		if (update.write_component_header(_simulation_object_update_angular_velocity_bit, "angular-velocity-exists"))
		{
			packet->write_vector("angular-velocity", &object_state_data->translational_velocity, k_object_angular_velocity_magnitude_minimum, k_object_angular_velocity_magnitude_maximum, 8);
		}

		update.finish_component();

		if (update.write_component_header(_simulation_object_update_body_vitality_bit, "body-vitality-exists"))
		{
			packet->write_quantized_real("body-vitality", object_state_data->body_vitality, k_object_body_vitality_minimum, k_object_body_vitality_maximum, 8, true);
			packet->write_bool("body-stun-ticks-is-zero", object_state_data->body_stun_ticks_is_zero);
		}

		update.finish_component();

		if (update.write_component_header(_simulation_object_update_shield_vitality_bit, "shield-vitality-exists"))
		{
			packet->write_quantized_real("shield-vitality", object_state_data->shield_vitality, k_object_shield_vitality_minimum, k_object_shield_vitality_maximum, 8, true);
			packet->write_bool("shield-stun-ticks-is-zero", object_state_data->shield_stun_ticks_is_zero);
		}

		update.finish_component();

		if (update.write_component_header(_simulation_object_update_region_state_bit, "region-state-exists"))
		{
			packet->write_integer("region-count", object_state_data->region_count, 4);

			for (int32 region_index = 0; region_index<NUMBEROF(object_state_data->region_states); ++region_index)
			{
				packet->write_integer("region-state", object_state_data->region_states[region_index], 3);
			}
		}

		update.finish_component();

		if (update.write_component_header(_simulation_object_update_constraints_bit, "constraint-state-exists"))
		{
			ASSERT(object_state_data->constraint_count>=0 && object_state_data->constraint_count<=k_object_constraint_count);

			packet->write_integer("constraint-count", object_state_data->constraint_count, 5);

			if (object_state_data->constraint_count>0)
			{
				packet->write_integer("destroyed-constraints", object_state_data->destroyed_constraints, object_state_data->constraint_count);
				packet->write_integer("loosened-constraints", object_state_data->loosened_constraints, object_state_data->constraint_count);
			}
		}

		update.finish_component();
		update.finish_update(update_mask_written);

		wrote_update = true;
	}

	packet->pop_structure("object-update", NONE);

	return wrote_update;
}

bool c_simulation_object_entity_definition::object_update_decode(
	bool initial_update,
	uint32* update_mask,
	s_simulation_object_state_data* object_state_data,
	class c_bitstream* packet,
	bool decode_for_network)
{
	bool decode_success = true;

	ASSERT(update_mask);
	ASSERT(object_state_data);
	ASSERT(packet);
	ASSERT((*update_mask & MASK(k_simulation_object_update_flag_count)) == 0);

	packet->push_structure("object-update", NONE, 0);

	if (packet->read_bool("dead-exists"))
	{
		object_state_data->dead = packet->read_bool("dead");
		SET_BIT(*update_mask, _simulation_object_update_dead_bit, true);
	}

	if (packet->read_bool("position-exists"))
	{
		simulation_read_quantized_position(packet, &object_state_data->relative_position, 16);
		SET_BIT(*update_mask, _simulation_object_update_position_bit, true);
		decode_success = decode_success && valid_real_point3d(&object_state_data->relative_position);

		if (!decode_success)
		{
			event(_event_warning, "simulation:objects: failed to decode object position");
		}
	}

	if (packet->read_bool("forward-and-up-exists"))
	{
		packet->read_axes("forward-and-up", &object_state_data->forward, &object_state_data->up);
		SET_BIT(*update_mask, _simulation_object_update_forward_and_up_bit, true);
		decode_success = decode_success && valid_real_vector3d_axes2(&object_state_data->forward, &object_state_data->up);

		if (!decode_success)
		{
			event(_event_warning, "simulation:objects: failed to decode object forward/up");
		}
	}

	if (packet->read_bool("scale-exists"))
	{
		object_state_data->scale = packet->read_quantized_real("scale", 0.f, k_object_scale_maximum, 7, false);
		SET_BIT(*update_mask, _simulation_object_update_scale_bit, true);
		decode_success = decode_success && valid_real(object_state_data->scale);

		if (!decode_success)
		{
			event(_event_warning, "simulation:objects: failed to decode object scale");
		}
	}

	if (packet->read_bool("translational-velocity-exists"))
	{
		packet->read_vector(
			"translational-velocity",
			&object_state_data->translational_velocity,
			k_object_translational_velocity_magnitude_minimum,
			k_object_translational_velocity_magnitude_maximum,
			10);
		SET_BIT(*update_mask, _simulation_object_update_translational_velocity_bit, true);
		decode_success = decode_success && valid_real_vector3d(&object_state_data->translational_velocity);

		if (!decode_success)
		{
			event(_event_warning, "simulation:objects: failed to decode translational velocity");
		}
	}

	if (packet->read_bool("angular-velocity-exists"))
	{
		packet->read_vector(
			"angular-velocity",
			&object_state_data->angular_velocity,
			k_object_angular_velocity_magnitude_minimum,
			k_object_angular_velocity_magnitude_maximum,
			8);
		SET_BIT(*update_mask, _simulation_object_update_angular_velocity_bit, true);
		decode_success = decode_success && valid_real_vector3d(&object_state_data->angular_velocity);

		if (!decode_success)
		{
			event(_event_warning, "simulation:objects: failed to decode angular velocity");
		}
	}

	if (packet->read_bool("body-vitality-exists"))
	{
		object_state_data->body_vitality = packet->read_quantized_real("body-vitality", k_object_body_vitality_minimum, k_object_body_vitality_maximum, 8, true);
		object_state_data->body_stun_ticks_is_zero = packet->read_bool("body-stun-ticks-is-zero");
		SET_BIT(*update_mask, _simulation_object_update_body_vitality_bit, true);
		decode_success = decode_success && valid_real(object_state_data->body_vitality);

		if (!decode_success)
		{
			event(_event_warning, "simulation:objects: failed to decode body vitality");
		}
	}

	if (packet->read_bool("shield-vitality-exists"))
	{
		object_state_data->shield_vitality = packet->read_quantized_real("shield-vitality", k_object_shield_vitality_minimum, k_object_shield_vitality_maximum, 8, true);
		object_state_data->shield_stun_ticks_is_zero = packet->read_bool("shield-stun-ticks-is-zero");
		SET_BIT(*update_mask, _simulation_object_update_shield_vitality_bit, true);
		decode_success = decode_success && valid_real(object_state_data->shield_vitality);

		if (!decode_success)
		{
			event(_event_warning, "simulation:objects: failed to decode shield vitality");
		}
	}

	if (packet->read_bool("region-state-exists"))
	{
		object_state_data->region_count = (uint8)packet->read_integer("region-count", 4);
		decode_success = decode_success && VALID_INDEX(object_state_data->region_count, NUMBEROF(object_state_data->region_states));

		for (int32 region_index = 0; region_index < NUMBEROF(object_state_data->region_states); ++region_index)
		{
			object_state_data->region_states[region_index] = (uint8)packet->read_integer("region-state", 3);
			decode_success = decode_success && VALID_INDEX(object_state_data->region_states[region_index], k_maximum_number_of_model_states);

			if (!decode_success)
			{
				event(_event_warning, "simulation:objects: failed to decode region state");
			}
		}

		SET_BIT(*update_mask, _simulation_object_update_region_state_bit, true);
	}

	if (packet->read_bool("constraint-state-exists"))
	{
		object_state_data->constraint_count = (uint8)packet->read_integer("constraint-count", 5);
		decode_success = decode_success && VALID_INDEX(object_state_data->constraint_count, MAXIMUM_DAMAGE_CONSTRAINT_INFOS_PER_MODEL);

		if (!decode_success)
		{
			event(_event_warning, "simulation:objects: failed to constraint cound");
		}

		if (object_state_data->constraint_count)
		{
			object_state_data->destroyed_constraints = (uint16)packet->read_integer("destroyed-constraints", object_state_data->constraint_count);
			object_state_data->loosened_constraints = (uint16)packet->read_integer("loosened-constraints", object_state_data->constraint_count);
		}

		SET_BIT(*update_mask, _simulation_object_update_constraints_bit, true);
	}

	packet->pop_structure("object-update", NONE);

	decode_success = decode_success && !packet->overflowed();
	if (!decode_success)
	{
		event(_event_warning, "simulation:objects: packet overflowed!");
	}

	return decode_success;
}

/* private code */

static bool simulation_object_variant_should_sync(
	const s_simulation_object_creation_data* creation_data)
{
	bool sync_variant = creation_data->model_variant_index != NONE;
	
	const object_definition* object_def = (object_definition*)tag_get_fast(creation_data->object_definition_index);
	const datum model_tag_index = object_def->object.model.index;

	if (model_tag_index != NONE && sync_variant)
	{
		s_model_definition* model_definition = (s_model_definition*)tag_get_fast(model_tag_index);

		// Confirm that the "default" variant is not the one we are trying to sync
		const s_model_variant* variant = TAG_BLOCK_GET_ELEMENT(&model_definition->variants, creation_data->model_variant_index, s_model_variant);

		sync_variant = variant->name != object_def->object.default_model_variant;
	}

	return sync_variant;
}

static int32 __stdcall c_simulation_object_entity_definition__object_creation_required_bits(void* _this)
{
	return simulation_definition_table_index_bits() + 92 + (6 + 1);
}
