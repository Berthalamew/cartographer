#include "stdafx.h"
#include "simulation_game_units.h"

#include "simulation_game_object_constants.h"
#include "simulation_game_objects.h"
#include "simulation_game_internal.h"


#include "game/game_globals.h"
#include "game/game_engine.h"
#include "math/color_math.h"
#include "memory/bitstream.h"
#include "networking/network_constants.h"
#include "networking/network_event.h"
#include "networking/network_utilities.h"
#include "networking/replication/replication_entity.h"
#include "units/units.h"

/* constants */

static real32 const k_weapon_update_age_min = 0.f;
static real32 const k_weapon_update_age_max = 1.f;

/* typedefs */

typedef bool(__thiscall* device_entity_update_decode_t)(
	c_simulation_unit_entity_definition*,
	bool,
	uint32*,
	int32,
	void*,
	c_bitstream*);

/* prototypes */

CLASS_HOOK_DECLARE_LABEL(c_simulation_unit_entity_definition__entity_update_decode, c_simulation_unit_entity_definition::entity_update_decode);
static void __declspec(naked) jmp_c_simulation_unit_entity_definition__entity_update_decode(void)
{
	CLASS_HOOK_JMP(c_simulation_unit_entity_definition__entity_update_decode, c_simulation_unit_entity_definition::entity_update_decode);
}

/* globals */

static device_entity_update_decode_t p_unit_entity_update_decode;

/* prototypes */

static datum __stdcall c_simulation_unit_entity_definition__create_object(
	void* _this,
	int32 creation_data_size,
	s_simulation_unit_creation_data* creation_data,
	uint32* flags,
	int32 internal_state_data_size,
	s_simulation_unit_state_data* initial_state_data);

/* public code */

void simulation_game_units_apply_patches(
	void)
{
	// Replace calls to this function so units that aren't players don't have their colors overridden
	DetourClassFunc(Memory::GetAddress<uint8*>(0x1F9DCB, 0x1E3B33), (uint8*)c_simulation_unit_entity_definition__create_object, 11);
	DETOUR_ATTACH(
		p_unit_entity_update_decode,
		Memory::GetAddress<device_entity_update_decode_t>(0x1F9E9E),
		jmp_c_simulation_unit_entity_definition__entity_update_decode
	);

	return;
}

bool c_simulation_unit_entity_definition::entity_update_decode(
	bool initial_update,
	uint32* update_mask,
	int32 state_data_size,
	void* state_data,
	c_bitstream* packet)
{
	bool decode_success = false;

	ASSERT(update_mask);
	ASSERT(*update_mask == 0);
	ASSERT(state_data_size==sizeof(struct s_simulation_unit_state_data));
	ASSERT(state_data);
	ASSERT(packet);

	bandwidth_profiler_record_push(11, packet);

	s_simulation_unit_state_data* unit_state_data = (s_simulation_unit_state_data*)state_data;
	bool object_success = object_update_decode(
		initial_update,
		update_mask,
		&unit_state_data->object_state_data,
		packet);
	decode_success = object_success;

	if (!object_success)
	{
		event(_event_warning, "simulation:units: failed to decode object");
	}


	if (packet->read_bool("control-exists"))
	{
		if (packet->read_bool("controlling-player-exists"))
		{
			unit_state_data->controlling_player_absolute_index = (int16)packet->read_integer("controlling-player-index", k_player_index_bit_count);
			decode_success = decode_success && VALID_INDEX(unit_state_data->controlling_player_absolute_index, k_maximum_players);
			if (!decode_success)
			{
				event(_event_warning, "simulation:units: failed to decode controlling player index");
			}
		}
		else
		{
			unit_state_data->controlling_player_absolute_index = NONE;
		}

		if (packet->read_bool("controlling-actor-exists"))
		{
			unit_state_data->controlling_simulation_actor_index = (int16)packet->read_integer("controlling-actor-index", k_network_actor_index_bit_count);
			decode_success = decode_success && VALID_INDEX(unit_state_data->controlling_simulation_actor_index, k_network_maximum_actors_per_simulation);
			if (!decode_success)
			{
				event(_event_warning, "simulation:units: failed to decode controlling actor index");
			}
		}
		else
		{
			unit_state_data->controlling_simulation_actor_index = NONE;
		}

		decode_success =
			decode_success &&
			(unit_state_data->controlling_player_absolute_index != NONE ||
			unit_state_data->controlling_simulation_actor_index != NONE);
		if (!decode_success)
		{
			event(
				_event_warning,
				"simulation:units: failed to decode, bad controlling_player_absolute_index/controlling_simulation_actor_index"
			);
		}

		SET_BIT(*update_mask, _simulation_unit_update_control_bit, true);
	}

	
	if (packet->read_bool("parent-vehicle-exists"))
	{
		if (packet->read_bool("parent-vehicle-exists"))
		{
			replication_entity_index_decode(packet, &unit_state_data->parent_vehicle_entity_index);
			unit_state_data->parent_vehicle_seat = (int16)packet->read_integer("parent-seat", 5);
		}
		else
		{
			unit_state_data->parent_vehicle_entity_index = NONE;
			unit_state_data->parent_vehicle_seat = NONE;
		}

		SET_BIT(*update_mask, _simulation_unit_update_parent_vehicle_bit, true);

		if (unit_state_data->parent_vehicle_entity_index==NONE)
		{
			decode_success = decode_success && unit_state_data->parent_vehicle_seat == NONE;

			if (!decode_success)
			{
				event(_event_warning, "simulation:units: failed to decode, bad parent seat index");
			}

		}
		else
		{
			decode_success = decode_success && VALID_INDEX(unit_state_data->parent_vehicle_seat, MAXIMUM_SEATS_PER_UNIT_DEFINITION);

			if (!decode_success)
			{
				event(_event_warning, "simulation:units: failed to decode, bad parent seat index range");
			}
		}
	}

	if (packet->read_bool("desired-aiming-vector-exists"))
	{
		packet->read_unit_vector("desired-aiming-vector", &unit_state_data->desired_aiming_vector);
		SET_BIT(*update_mask, _simulation_unit_update_desired_aiming_vector_bit, true);
		decode_success = decode_success && valid_real_normal3d(&unit_state_data->desired_aiming_vector);

		if (!decode_success)
		{
			event(_event_warning, "simulation:units: failed to decode, desired aiming vector bit");
		}
	}

	if (packet->read_bool("desired-weapon-set-exists"))
	{
		unit_state_data->desired_weapon_set.set_identifier = (int16)packet->read_integer("desired-weapon-set-identifier", 5)-1;
		unit_state_data->desired_weapon_set.weapon_indices[0] = (int8)packet->read_integer("desired-primary-weapon", 3)-1;
		unit_state_data->desired_weapon_set.weapon_indices[1] = (int8)packet->read_integer("desired-secondary-weapon", 3)-1;
		SET_BIT(*update_mask, _simulation_unit_update_desired_weapon_bit, true);
		decode_success =
			decode_success &&
			unit_state_data->desired_weapon_set.weapon_indices[0] == NONE ||
			unit_state_data->desired_weapon_set.weapon_indices[0] != unit_state_data->desired_weapon_set.weapon_indices[1];
		
		if (!decode_success)
		{
			event(_event_warning, "simulation:units: failed to decode, duplicate weapon index!");
		}
	}

	for (int32 weapon_num = 0; weapon_num < NUMBEROF(unit_state_data->weapons); ++weapon_num)
	{
		if (packet->read_bool("weapon-type-exists"))
		{
			s_simulation_unit_weapon_state_data* weapon_state_data = &unit_state_data->weapons[weapon_num];
			weapon_state_data->weapon_definition_index= simulation_read_definition_index("weapon-definition-index", packet);
			weapon_state_data->multiplayer_weapon_identifier = (int16)packet->read_integer("multiplayer-team-index", k_team_index_bits);
			SET_BIT(*update_mask, _simulation_unit_update_first_weapon_type_bit+weapon_num, true);
			
			if (!decode_success)
			{
				event(_event_warning, "simulation:units: failed to decode, bad weapon definition index");
			}

			decode_success =
				decode_success &&
				weapon_state_data->weapon_definition_index == NONE ||
				VALID_INDEX(weapon_state_data->multiplayer_weapon_identifier, k_game_multiplayer_team_count);

			if (!decode_success)
			{
				event(_event_warning, "simulation:units: failed to decode, bad multiplayer_weapon_identifier");
			}
		}

		if (packet->read_bool("weapon-state-exists"))
		{
			s_simulation_unit_weapon_state_data* weapon_state_data = &unit_state_data->weapons[weapon_num];
			weapon_state_data->rounds_loaded = (int16)packet->read_integer("rounds-loaded", 8);
			weapon_state_data->rounds_inventory = (int16)packet->read_integer("rounds-inventory", 11);
			weapon_state_data->age = packet->read_quantized_real("age", k_weapon_update_age_min, k_weapon_update_age_max, 7, true);
			SET_BIT(*update_mask, _simulation_unit_update_first_weapon_state_bit+weapon_num, true);
		}
	}

	if (packet->read_bool("grenade-counts-exists"))
	{
		packet->read_raw_data("grenade-counts", unit_state_data->grenade_counts, SIZEOF_BITS(unit_state_data->grenade_counts));
		SET_BIT(*update_mask, _simulation_unit_update_grenade_counts_bit, true);

		for (int32 grenade_num = 0; grenade_num<NUMBEROF(unit_state_data->grenade_counts); ++grenade_num)
		{
			decode_success = decode_success && unit_state_data->grenade_counts[grenade_num] >= 0;
			if (!decode_success)
			{
				event(_event_warning, "simulation:units: failed to decode, bad grenade count");
			}
		}
	}

	if (packet->read_bool("active-camo-exists"))
	{
		unit_state_data->active_camouflage_active = packet->read_bool("active-camo-active");
		unit_state_data->active_camo = packet->read_quantized_real("active-camo", 0.f, 1.f, 6, true);
		unit_state_data->active_camo_regrowth = packet->read_quantized_real("active-camo-regrowth", 0.f, 1.f, 6, true);
		SET_BIT(*update_mask, _simulation_unit_update_active_camouflage_bit, true);
	}

	bandwidth_profiler_record_pop(11, packet);

	decode_success = decode_success && !packet->overflowed();
	if (!decode_success)
	{
		event(_event_warning, "simulation:units: failed to decode, overflowed!");
	}

	return decode_success;
}

/* private code */

static datum __stdcall c_simulation_unit_entity_definition__create_object(
	void* _this,
	int32 creation_data_size,
	s_simulation_unit_creation_data* creation_data,
	uint32* flags,
	int32 internal_state_data_size,
	s_simulation_unit_state_data* initial_state_data)
{
	real_rgb_color change_colors[4];
	
	object_placement_data placement_data;
	object_placement_data_new(&placement_data, creation_data->object.object_definition_index, NONE, 0);

	// Hacky hack for player variants
	// TODO Remove this once we get tag injection working on servers
	if (initial_state_data->controlling_player_absolute_index != NONE)
	{
		if (datum unit_rep_tag_index = game_globals_get_representation(creation_data->appearance.player_character_type)->third_person_unit.index;
			unit_rep_tag_index != NONE)
		{
			placement_data.definition_index = unit_rep_tag_index;
			creation_data->object.object_definition_index = unit_rep_tag_index;
		}
	}
	c_simulation_object_entity_definition__object_setup_placement_data(_this, &creation_data->object, &initial_state_data->object_state_data, flags, &placement_data);

	// We check the following in order to force the player colour
	// The unit is not controlled by an actor
	// The function game_engine_get_change_colors is able to retrieve the colours for the engine mode
	if ((initial_state_data->controlling_actor_index == NONE) &&
		game_engine_get_change_colors(&creation_data->appearance, creation_data->team, change_colors))
	{
		placement_data.change_color_override_mask |= MASK(4);
		csmemcpy(placement_data.change_color_overrides, change_colors, sizeof(placement_data.change_color_overrides));
	}

	const datum unit_index = c_simulation_object_entity_definition__object_create_object(_this, &creation_data->object, &initial_state_data->object_state_data, flags, &placement_data);
	if (unit_index != NONE)
	{
		unit_delete_all_weapons(unit_index);
		event(_event_status, "simulation:objects: unit object 0x%08x created with team %d", unit_index, creation_data->team);
	}

	return unit_index;
}
