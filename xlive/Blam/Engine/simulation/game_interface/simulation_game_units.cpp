#include "stdafx.h"
#include "simulation_game_units.h"

#include "simulation_game_object_constants.h"
#include "simulation_game_objects.h"
#include "simulation_game_internal.h"

#include "game/game_globals.h"
#include "game/game_engine.h"
#include "game/players.h"
#include "math/color_math.h"
#include "memory/bitstream.h"
#include "networking/network_constants.h"
#include "networking/network_event.h"
#include "networking/network_utilities.h"
#include "networking/replication/replication_entity.h"
#include "simulation/simulation_entity_definition.h"
#include "simulation/simulation_view_telemetry.h"
#include "units/units.h"

/* constants */

static real32 const k_weapon_update_age_min = 0.f;
static real32 const k_weapon_update_age_max = 1.f;

static real32 const k_unit_update_active_camouflage_min_value = 0.f;
static real32 const k_unit_update_active_camouflage_max_value = 1.f;
static real32 const k_unit_update_active_camouflage_regrowth_min_value = 0.f;
static real32 const k_unit_update_active_camouflage_regrowth_max_value = 1.f;

/* prototypes */

CLASS_HOOK_DECLARE_LABEL(c_simulation_unit_entity_definition__entity_update_encode, c_simulation_unit_entity_definition::entity_update_encode);
static void __declspec(naked) jmp_c_simulation_unit_entity_definition__entity_update_encode(void)
{
	CLASS_HOOK_JMP(c_simulation_unit_entity_definition__entity_update_encode, c_simulation_unit_entity_definition::entity_update_encode);
}

CLASS_HOOK_DECLARE_LABEL(c_simulation_unit_entity_definition__entity_update_decode, c_simulation_unit_entity_definition::entity_update_decode);
static void __declspec(naked) jmp_c_simulation_unit_entity_definition__entity_update_decode(void)
{
	CLASS_HOOK_JMP(c_simulation_unit_entity_definition__entity_update_decode, c_simulation_unit_entity_definition::entity_update_decode);
}

CLASS_HOOK_DECLARE_LABEL(c_simulation_unit_entity_definition__build_object_creation_data, c_simulation_unit_entity_definition::build_object_creation_data);
static void __declspec(naked) jmp_c_simulation_unit_entity_definition__build_object_creation_data(void)
{
	CLASS_HOOK_JMP(c_simulation_unit_entity_definition__build_object_creation_data, c_simulation_unit_entity_definition::build_object_creation_data);
}

CLASS_HOOK_DECLARE_LABEL(c_simulation_unit_entity_definition__create_object, c_simulation_unit_entity_definition::create_object);
static void __declspec(naked) jmp_c_simulation_unit_entity_definition__create_object(void)
{
	CLASS_HOOK_JMP(c_simulation_unit_entity_definition__create_object, c_simulation_unit_entity_definition::create_object);
}

CLASS_HOOK_DECLARE_LABEL(c_simulation_unit_entity_definition__entity_creation_encode, c_simulation_unit_entity_definition::entity_creation_encode);
static void __declspec(naked) jmp_c_simulation_unit_entity_definition__entity_creation_encode(void)
{
	CLASS_HOOK_JMP(c_simulation_unit_entity_definition__entity_creation_encode, c_simulation_unit_entity_definition::entity_creation_encode);
}

CLASS_HOOK_DECLARE_LABEL(c_simulation_unit_entity_definition__entity_creation_decode, c_simulation_unit_entity_definition::entity_creation_decode);
static void __declspec(naked) jmp_c_simulation_unit_entity_definition__entity_creation_decode(void)
{
	CLASS_HOOK_JMP(c_simulation_unit_entity_definition__entity_creation_decode, c_simulation_unit_entity_definition::entity_creation_decode);
}

/* globals */

static uintptr_t p_unit_entity_update_decode;

/* prototypes */

/* public code */

void simulation_game_units_apply_patches(
	void)
{
	
	WritePointer(Memory::GetAddress(0x3C8E1C, 0x0), jmp_c_simulation_unit_entity_definition__entity_update_encode);
	WritePointer(Memory::GetAddress(0x3C8E20, 0x0), jmp_c_simulation_unit_entity_definition__entity_update_decode);
	
	WritePointer(Memory::GetAddress(0x3C8E14, 0x0), jmp_c_simulation_unit_entity_definition__entity_creation_encode);
	WritePointer(Memory::GetAddress(0x3C8E18, 0x0), jmp_c_simulation_unit_entity_definition__entity_creation_decode);
	WritePointer(Memory::GetAddress(0x3C8E4C, 0x0), jmp_c_simulation_unit_entity_definition__build_object_creation_data);
	
	// Replace calls to this function so units that aren't players don't have their colors overridden
	WritePointer(Memory::GetAddress(0x3C8E58), jmp_c_simulation_unit_entity_definition__create_object);

	// don't update the weapon state if we didn't actually received an update
	// for some reason someone at bungie thought it was a good idea to apply an weapon ammo update
	// even if we received just the weapon definition
	NopFill(Memory::GetAddress(0x1F836A, 0x1E20D0), 4);

	return;
}

void c_simulation_unit_entity_definition::build_object_creation_data(
	int32 unit_index,
	int32 creation_data_size,
	void* creation_data)
{
	s_simulation_unit_creation_data* unit_creation_data = (s_simulation_unit_creation_data*)creation_data;
	unit_datum const* unit = unit_get(unit_index);

	ASSERT(creation_data_size==sizeof(struct s_simulation_unit_creation_data));
	ASSERT(creation_data);

	csmemset(unit_creation_data, 0, sizeof(*unit_creation_data));

	c_simulation_object_entity_definition::object_build_creation_data(unit_index, &unit_creation_data->object);
	
	player_appearance_initialize(&unit_creation_data->appearance);

	if (unit->unit.player_index!=NONE)
	{
		player_datum const* player = player_get(unit->unit.player_index);

		unit_creation_data->appearance = player->configuration.appearance;
	}
	else
	{
		unit_creation_data->team = unit->unit.unit_team;
	}

	return;
}

int32 c_simulation_unit_entity_definition::create_object(
	int32 creation_data_size,
	void const* creation_data,
	uint32* flags,
	int32 internal_state_data_size,
	void const* initial_state_data)
{
	real_rgb_color change_colors[4];
	object_placement_data placement_data;
	
	s_simulation_unit_creation_data *unit_creation_data = (s_simulation_unit_creation_data *)creation_data;
	s_simulation_unit_state_data const* unit_state_data = (s_simulation_unit_state_data const*)initial_state_data;
	object_placement_data_new(&placement_data, unit_creation_data->object.object_definition_index, NONE, 0);

	// Hacky hack for player variants
	// TODO Remove this once we get tag injection working on servers
	if (unit_state_data->controlling_player_absolute_index != NONE)
	{
		datum unit_rep_tag_index = game_globals_get_representation(unit_creation_data->appearance.player_character_type)->third_person_unit.index;

		if (unit_rep_tag_index != NONE)
		{
			placement_data.definition_index = unit_rep_tag_index;
			unit_creation_data->object.object_definition_index = unit_rep_tag_index;
		}
	}

	c_simulation_object_entity_definition::object_setup_placement_data(&unit_creation_data->object, &unit_state_data->object_state_data, flags, &placement_data);

	// We check the following in order to force the player colour
	// The unit is not controlled by an actor
	// The function game_engine_get_change_colors is able to retrieve the colours for the engine mode
	if ((unit_state_data->controlling_simulation_actor_index == NONE) &&
		game_engine_get_change_colors(&unit_creation_data->appearance, unit_creation_data->team, change_colors))
	{
		placement_data.change_color_override_mask |= MASK(4);
		csmemcpy(placement_data.change_color_overrides, change_colors, sizeof(placement_data.change_color_overrides));
	}

	const datum unit_index = c_simulation_object_entity_definition::object_create_object(&unit_creation_data->object, &unit_state_data->object_state_data, flags, &placement_data);

	if (unit_index != NONE)
	{
		unit_delete_all_weapons(unit_index);
		event(_event_status, "simulation:objects: unit object 0x%08x created with team %d", unit_index, unit_creation_data->team);
	}

	return unit_index;
}


void c_simulation_unit_entity_definition::entity_creation_encode(
	int32 creation_data_size,
	void const* creation_data,
	s_simulation_view_telemetry_data const* telemetry_data,
	c_bitstream* packet,
	bool encode_for_network)
{
	s_simulation_unit_creation_data const* unit_creation_data = (s_simulation_unit_creation_data const*)creation_data;

	ASSERT(creation_data_size==sizeof(struct s_simulation_unit_creation_data));
	ASSERT(creation_data);
	ASSERT(packet);

	packet->push_structure("unit-creation", NONE, 0);

	c_simulation_object_entity_definition::object_creation_encode(&unit_creation_data->object, packet, encode_for_network);
	player_appearance_encode(packet, &unit_creation_data->appearance);
	packet->write_bool("team-valid", unit_creation_data->team != _game_team_observer);

	if (unit_creation_data->team!=_game_team_observer)
	{
		packet->write_integer("team", unit_creation_data->team, k_team_index_bits);
	}

	packet->pop_structure("unit-creation", NONE);

	return;
}

bool c_simulation_unit_entity_definition::entity_creation_decode(
	int32 creation_data_size,
	void* creation_data,
	c_bitstream* packet,
	bool decode_for_network)
{
	bool object_success;
	bool appearance_success;
	bool decode_success;

	s_simulation_unit_creation_data* unit_creation_data = (s_simulation_unit_creation_data *)creation_data;

	ASSERT(creation_data_size==sizeof(struct s_simulation_unit_creation_data));
	ASSERT(creation_data);
	ASSERT(packet);

	packet->push_structure("unit-creation", NONE, 0);

	object_success = c_simulation_object_entity_definition::object_creation_decode(&unit_creation_data->object, packet, decode_for_network);
	appearance_success = player_appearance_decode(packet, &unit_creation_data->appearance);

	if (packet->read_bool("team-valid"))
	{
		unit_creation_data->team = (e_game_team)packet->read_integer("team", k_team_index_bits);
	}
	else
	{
		unit_creation_data->team = _game_team_observer;
	}

	packet->pop_structure("unit-creation", NONE);

	decode_success = !packet->overflowed() && appearance_success;
	decode_success = decode_success && appearance_success;

	return decode_success;
}

bool c_simulation_unit_entity_definition::entity_update_encode(
	bool initial_update,
	uint32 update_mask,
	uint32* update_mask_written,
	int32 state_data_size,
	void const* state_data,
	struct s_simulation_view_telemetry_data const* telemetry_data,
	class c_bitstream* packet,
	int32 must_leave_space_bits,
	bool encode_for_network)
{
	uint32 object_update_mask;
	int32 object_must_leave_space_bits;

	bool wrote_update = false;
	s_simulation_unit_state_data const* unit_state_data = (s_simulation_unit_state_data const*)state_data;

	ASSERT(update_mask!=0);
	ASSERT((update_mask & ~MASK(k_simulation_unit_update_flag_count))==0);
	ASSERT(update_mask_written);
	ASSERT(*update_mask_written==0);
	ASSERT(state_data_size==sizeof(struct s_simulation_unit_state_data));
	ASSERT(state_data);
	ASSERT(packet);

	packet->push_structure("unit-update", NONE, 0);

	object_update_mask = update_mask & MASK(k_simulation_object_update_flag_count);
	object_must_leave_space_bits = must_leave_space_bits + 14;

	if (c_simulation_object_entity_definition::object_update_encode(
			initial_update,
			object_update_mask,
			update_mask_written,
			telemetry_data,
			&unit_state_data->object_state_data,
			packet,
			object_must_leave_space_bits,
			ensure_object_position_update_quantization_inside_bsp(),
			encode_for_network)
		)
	{
		int32 first_update_flag = _simulation_unit_update_control_bit;
		int32 update_flag_count = k_simulation_unit_update_flag_count - k_simulation_object_update_flag_count;
		uint32 unit_update_mask = MASK(k_simulation_unit_update_flag_count) - MASK(k_simulation_object_update_flag_count);;

		c_entity_update_encode_helper update;

		if (update.make_room_for_update(packet, must_leave_space_bits, first_update_flag, update_flag_count, unit_update_mask))
		{
			if (update.write_component_header(_simulation_unit_update_control_bit, "control-exists"))
			{
				packet->write_bool("controlling-player-exists", unit_state_data->controlling_player_absolute_index != NONE);

				if (unit_state_data->controlling_player_absolute_index != NONE)
				{
					packet->write_integer("controlling-player-index", unit_state_data->controlling_player_absolute_index, k_player_index_bits);
				}

				packet->write_bool("controlling-actor-exists", unit_state_data->controlling_simulation_actor_index != NONE);

				if (unit_state_data->controlling_simulation_actor_index != NONE)
				{
					packet->write_integer("controlling-actor-index", unit_state_data->controlling_simulation_actor_index, k_network_actor_index_bit_count);
				}
			}

			update.finish_component();

			if (update.write_component_header(_simulation_unit_update_parent_vehicle_bit, "parent-vehicle-exists"))
			{
				if (encode_for_network)
				{
					bool update_possible = true;

					if (unit_state_data->parent_vehicle_entity_index != NONE)
					{
						ASSERT(telemetry_data->provider);

						update_possible = telemetry_data->provider->entity_is_active(unit_state_data->parent_vehicle_entity_index);
					}

					if (update_possible)
					{
						packet->write_bool("parent-vehicle-exists", unit_state_data->parent_vehicle_entity_index != NONE);
						if (unit_state_data->parent_vehicle_entity_index != NONE)
						{
							replication_entity_index_encode(packet, unit_state_data->parent_vehicle_entity_index);
							packet->write_integer("parent-seat", unit_state_data->parent_vehicle_seat, 5);
						}
					}
					else
					{
						update.skip_component();
					}
				}
				else
				{
					// FIXME: finish this
					unreachable();
				}
			}

			update.finish_component();

			if (update.write_component_header(_simulation_unit_update_desired_aiming_vector_bit, "desired-aiming-vector-exists"))
			{
				packet->write_unit_vector("desired-aiming-vector", &unit_state_data->desired_aiming_vector);
			}

			update.finish_component();


			if (update.write_component_header(_simulation_unit_update_desired_weapon_bit, "desired-weapon-set-exists"))
			{
				packet->write_integer("desired-weapon-set-identifier", unit_state_data->desired_weapon_set.set_identifier + 1, 5);
				packet->write_integer("desired-primary-weapon", unit_state_data->desired_weapon_set.weapon_indices[0] + 1, 3);
				packet->write_integer("desired-secondary-weapon", unit_state_data->desired_weapon_set.weapon_indices[1] + 1, 3);
			}

			update.finish_component();

			for (int32 weapon_num = 0; weapon_num < NUMBEROF(unit_state_data->weapons); ++weapon_num)
			{
				if (update.write_component_header(weapon_num + _simulation_unit_update_first_weapon_type_bit, "weapon-type-exists"))
				{
					s_simulation_unit_weapon_state_data const* weapon_state_data = &unit_state_data->weapons[weapon_num];

					simulation_write_definition_index("weapon-definition-index", packet, weapon_state_data->weapon_definition_index);
					packet->write_integer("multiplayer-team-index", weapon_state_data->multiplayer_weapon_identifier + 1, 4);
				}

				update.finish_component();

				if (update.write_component_header(weapon_num + _simulation_unit_update_first_weapon_state_bit, "weapon-type-exists"))
				{
					s_simulation_unit_weapon_state_data const* weapon_state_data = &unit_state_data->weapons[weapon_num];

					packet->write_integer("rounds-loaded", weapon_state_data->simulation_weapon_identifier, 8);
					packet->write_integer("rounds-inventory", weapon_state_data->rounds_loaded, 11);
					packet->write_quantized_real("age", weapon_state_data->age, k_weapon_update_age_min, k_weapon_update_age_max, 7, true);
				}

				update.finish_component();
			}

			if (update.write_component_header(_simulation_unit_update_grenade_counts_bit, "grenade-counts-exists"))
			{
				packet->write_raw_data("grenade-counts", unit_state_data->grenade_counts, SIZEOF_BITS(unit_state_data->grenade_counts));
			}

			update.finish_component();

			if (update.write_component_header(_simulation_unit_update_active_camouflage_bit, "active-camo-exists"))
			{
				packet->write_bool("active-camo-active", unit_state_data->active_camouflage_active);
				packet->write_quantized_real(
					"active-camo",
					unit_state_data->active_camo,
					k_unit_update_active_camouflage_min_value,
					k_unit_update_active_camouflage_max_value,
					6,
					true
				);
				packet->write_quantized_real(
					"active-camo-regrowth",
					unit_state_data->active_camo_regrowth,
					k_unit_update_active_camouflage_regrowth_min_value,
					k_unit_update_active_camouflage_regrowth_max_value,
					6,
					true
				);
			}

			update.finish_component();
			update.finish_update(update_mask_written);

			wrote_update = true;
		}
	}

	packet->pop_structure("unit-update", NONE);

	return wrote_update;
}

bool c_simulation_unit_entity_definition::entity_update_decode(
	bool initial_update,
	uint32* update_mask,
	int32 state_data_size,
	void* state_data,
	c_bitstream* packet,
	bool decode_for_network)
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
		packet,
		decode_for_network);
	decode_success = object_success;

	if (!object_success)
	{
		event(_event_warning, "simulation:units: failed to decode object");
	}

	if (packet->read_bool("control-exists"))
	{
		if (packet->read_bool("controlling-player-exists"))
		{
			unit_state_data->controlling_player_absolute_index = (int16)packet->read_integer("controlling-player-index", k_player_index_bits);
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
			if (decode_for_network)
			{
				replication_entity_index_decode(packet, &unit_state_data->parent_vehicle_entity_index);
			}
			else
			{
				// FIXME
			}
			
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
		unit_state_data->active_camo = packet->read_quantized_real(
			"active-camo",
			k_unit_update_active_camouflage_min_value,
			k_unit_update_active_camouflage_max_value,
			6,
			true
		);
		unit_state_data->active_camo_regrowth = packet->read_quantized_real(
			"active-camo-regrowth",
			k_unit_update_active_camouflage_regrowth_min_value,
			k_unit_update_active_camouflage_regrowth_max_value,
			6, 
			true
		);

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
