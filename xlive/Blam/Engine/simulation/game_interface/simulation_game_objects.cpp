#include "stdafx.h"
#include "simulation_game_objects.h"

#include "simulation_game_interface.h"
#include "simulation_game_object_constants.h"

#include "cache/cache_files.h"
#include "memory/bitstream.h"
#include "models/models.h"
#include "networking/network_event.h"
#include "objects/objects.h"
#include "objects/object_definition.h"
#include "simulation/simulation_encoding.h"

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

/* typedefs */

typedef void(__stdcall* c_simulation_unit_entity_definition_creation_encode_t)(void*, s_simulation_object_creation_data*, c_bitstream*);
typedef bool(__stdcall* c_simulation_unit_entity_definition_creation_decode_t)(void* thisptr, void* creation_data, c_bitstream* stream);

/* prototypes */

// Ensure we aren't sending a variant index that's the same as the default variant
// Inefficient since the variant will be set to it regardless
// If we sync the variant index of the "default" variant this can also cause an issue where the default dialouge isn't selected but that variants specific dialouge
static bool simulation_object_variant_should_sync(s_simulation_object_creation_data* creation_data);

/* globals */

static c_simulation_unit_entity_definition_creation_encode_t p_c_simulation_unit_entity_definition_encode;
static c_simulation_unit_entity_definition_creation_decode_t p_c_simulation_unit_entity_definition_decode;

/* public code */

bool c_simulation_object_entity_definition::object_update_decode(
	bool initial_update,
	uint32* update_mask,
	s_simulation_object_state_data* object_state_data,
	class c_bitstream* packet)
{
	bool decode_success = true;

	ASSERT(update_mask);
	ASSERT(object_state_data);
	ASSERT(packet);
	ASSERT((*update_mask & MASK(k_simulation_object_update_flag_count))==0);

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

		for (int32 i = 0; i<NUMBEROF(object_state_data->region_states); ++i)
		{
			object_state_data->region_states[i] = (uint8)packet->read_integer("region-state", 3);
			decode_success = decode_success && VALID_INDEX(object_state_data->region_states[i], k_maximum_number_of_model_states);

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

	decode_success = decode_success && !packet->overflowed();
	if (!decode_success)
	{
		event(_event_warning, "simulation:objects: packet overflowed!");
	}

	return decode_success;
}

// Builds creation data for objects
void __stdcall c_simulation_object_entity_definition__object_build_creation_data(
	void* _this,
	datum object_index,
	s_simulation_object_creation_data* creation_data)
{
	const object_datum* object = object_get(object_index);
	creation_data->object_definition_index = object->definition_index;
	creation_data->object_scenario_datum_index = object->object.placement_index;
	creation_data->multiplayer_spawn_monitor_index = object->object.netgame_equipment_index;
	creation_data->model_variant_index = object->object.model_variant_id;
	creation_data->emblem_info = object->object.emblem_info;
	return;
}

static bool simulation_object_variant_should_sync(
	s_simulation_object_creation_data* creation_data)
{
	bool sync_variant = creation_data->model_variant_index != NONE;
	object_definition* object_def = (object_definition*)tag_get_fast(creation_data->object_definition_index);
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

void __stdcall c_simulation_object_entity_definition__object_creation_encode(void* _this, s_simulation_object_creation_data* creation_data, c_bitstream* packet)
{
	bool model_variant_id_exists = simulation_object_variant_should_sync(creation_data);

	packet->write_bool("model-variant-index-exists", model_variant_id_exists);
	if (model_variant_id_exists)
	{
		packet->write_integer("model-variant-index", creation_data->model_variant_index, 6);    // 6 bits since k_maximum_variants_per_model is 64
	}

	p_c_simulation_unit_entity_definition_encode(_this, creation_data, packet);
	return;
}

bool __stdcall c_simulation_object_entity_definition__object_creation_decode(void* _this, s_simulation_object_creation_data* creation_data, c_bitstream* packet)
{
	if (packet->read_bool("model-variant-index-exists"))
	{
		creation_data->model_variant_index = (int8)packet->read_integer("model-variant-index", 6);    // 6 bits since k_maximum_variants_per_model is 64
	}
	else
	{
		creation_data->model_variant_index = NONE;
	}
	return p_c_simulation_unit_entity_definition_decode(_this, creation_data, packet);
}

datum __cdecl simulation_object_get_replicated_object_from_entity(int32 entity_index)
{
	return INVOKE(0x1F2211, 0x1DD2AA, simulation_object_get_replicated_object_from_entity, entity_index);
}

bool __stdcall c_simulation_object_entity_definition__object_setup_placement_data(void* _this,
	s_simulation_object_creation_data* object_creation_data,
	s_simulation_object_state_data* state_data,
	uint32* flags,
	object_placement_data* placement_data)
{
	bool result = false;
	
	ASSERT(object_creation_data);
	ASSERT(placement_data);

	if (object_creation_data->object_scenario_datum_index == NONE)
	{
		object_placement_data_new(placement_data, object_creation_data->object_definition_index, NONE, 0);
		SET_BIT(placement_data->flags, 1, true);
		SET_BIT(placement_data->flags, 4, true);
		placement_data->emblem_info = object_creation_data->emblem_info;
		if (TEST_BIT(*flags, 1))
		{
			placement_data->position = state_data->relative_position;
			*flags &= ~FLAG(1);
		}
		if (TEST_BIT(*flags, 2))
		{
			placement_data->forward = state_data->forward;
			placement_data->up = state_data->up;
			*flags &= ~FLAG(2);
		}
		if (TEST_BIT(*flags, 3))
		{
			placement_data->scale = state_data->scale;
			*flags &= ~FLAG(3);
		}
		if (TEST_BIT(*flags, 4))
		{
			placement_data->translational_velocity = state_data->translational_velocity;
			*flags &= ~FLAG(4);
		}
		if (TEST_BIT(*flags, 5))
		{
			placement_data->angular_velocity = state_data->angular_velocity;
			*flags &= ~FLAG(5);
		}

		// Set variant of the object
		if (object_creation_data->model_variant_index != NONE && object_creation_data->object_definition_index != NONE)
		{
			object_definition* object_def = (object_definition*)tag_get_fast(object_creation_data->object_definition_index);
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

int32 __stdcall c_simulation_object_entity_definition__object_creation_required_bits(void* _this)
{
	return simulation_definition_table_index_bits() + 92 + (6 + 1);
}

datum __stdcall c_simulation_object_entity_definition__object_create_object(
	void* _this,
	s_simulation_object_creation_data* object_creation,
	s_simulation_object_state_data* state_data,
	uint32* flags,
	object_placement_data* placement_data)
{
	typedef datum(__thiscall* object_create_object_t)(void*, s_simulation_object_creation_data*, s_simulation_object_state_data*, uint32*, object_placement_data*);
	auto p_object_create_object = Memory::GetAddress<object_create_object_t>(0x1F32DB, 0x1DE374);
	return p_object_create_object(_this, object_creation, state_data, flags, placement_data);
}

void simulation_game_objects_apply_patches(void)
{
	DetourClassFunc(Memory::GetAddress<uint8*>(0x1F27D1, 0x1DD86A), (uint8*)c_simulation_object_entity_definition__object_creation_required_bits, 8);
	DetourClassFunc(Memory::GetAddress<uint8*>(0x1F24ED, 0x1DD586), (uint8*)c_simulation_object_entity_definition__object_build_creation_data, 9);
	DetourClassFunc(Memory::GetAddress<uint8*>(0x1F2704, 0x1DD79D), (uint8*)c_simulation_object_entity_definition__object_setup_placement_data, 10);
	p_c_simulation_unit_entity_definition_encode = (c_simulation_unit_entity_definition_creation_encode_t)DetourClassFunc(
		Memory::GetAddress<uint8*>(0x1F3B11, 0x1DEBAA),
		(uint8*)c_simulation_object_entity_definition__object_creation_encode,
		8);
	p_c_simulation_unit_entity_definition_decode = (c_simulation_unit_entity_definition_creation_decode_t)DetourClassFunc(
		Memory::GetAddress<uint8*>(0x1F3BDD, 0x1DEC76),
		(uint8*)c_simulation_object_entity_definition__object_creation_decode,
		10);
	return;
}