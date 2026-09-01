#include "stdafx.h"
#include "simulation_game_projectiles.h"

#include "game/game.h"
#include "items/projectiles.h"
#include "memory/bitstream.h"
#include "networking/replication/replication_entity.h"
#include "networking/network_event.h"
#include "simulation/simulation_gamestate_entities.h"

/* prototypes */

CLASS_HOOK_DECLARE_LABEL(c_simulation_projectile_entity_definition__build_object_creation_data, c_simulation_projectile_entity_definition::build_object_creation_data);
static void __declspec(naked) jmp_c_simulation_projectile_entity_definition__build_object_creation_data(void)
{
	CLASS_HOOK_JMP(c_simulation_projectile_entity_definition__build_object_creation_data, c_simulation_projectile_entity_definition::build_object_creation_data);
}

CLASS_HOOK_DECLARE_LABEL(c_simulation_projectile_entity_definition__entity_creation_encode, c_simulation_projectile_entity_definition::entity_creation_encode);
static void __declspec(naked) jmp_c_simulation_projectile_entity_definition__entity_creation_encode(void)
{
	CLASS_HOOK_JMP(c_simulation_projectile_entity_definition__entity_creation_encode, c_simulation_projectile_entity_definition::entity_creation_encode);
}

CLASS_HOOK_DECLARE_LABEL(c_simulation_projectile_entity_definition__entity_creation_decode, c_simulation_projectile_entity_definition::entity_creation_decode);
static void __declspec(naked) jmp_c_simulation_projectile_entity_definition__entity_creation_decode(void)
{
	CLASS_HOOK_JMP(c_simulation_projectile_entity_definition__entity_creation_decode, c_simulation_projectile_entity_definition::entity_creation_decode);
}

/* public code */

void simulation_game_projectiles_apply_patches(
	void)
{
	WritePointer(Memory::GetAddress(0x3CA104), jmp_c_simulation_projectile_entity_definition__build_object_creation_data);
	WritePointer(Memory::GetAddress(0x3CA0CC), jmp_c_simulation_projectile_entity_definition__entity_creation_encode);
	WritePointer(Memory::GetAddress(0x3CA0D0), jmp_c_simulation_projectile_entity_definition__entity_creation_decode);

	return;
}

void c_simulation_projectile_entity_definition::build_object_creation_data(
	int32 projectile_index,
	int32 creation_data_size,
	void* creation_data)
{
	projectile_datum* projectile = projectile_get(projectile_index);
	s_simulation_projectile_creation_data* projectile_creation_data = (s_simulation_projectile_creation_data*)creation_data;

	ASSERT(creation_data_size==sizeof(struct s_simulation_projectile_creation_data));
	ASSERT(creation_data);

	csmemset(projectile_creation_data, 0, sizeof(*projectile_creation_data));

	c_simulation_object_entity_definition::object_build_creation_data(projectile_index, &projectile_creation_data->object_creation);

	if (projectile->object.damage_owner_owner_index==NONE)
	{
		projectile_creation_data->owner_player_absolute_index=NONE;
	}
	else
	{
		projectile_creation_data->owner_player_absolute_index = DATUM_INDEX_TO_ABSOLUTE_INDEX(projectile->object.damage_owner_owner_index);
	}

	projectile_creation_data->target_entity_index = NONE;
	projectile_creation_data->target_entity_model_target_index = NONE;
	
	if (projectile->projectile.target_object_index!=NONE)
	{
		object_datum const* target_object = object_get(projectile->projectile.target_object_index);
		int32 target_gamestate_index = target_object->object.gamestate_index;
		int32 target_entity_index = NONE;

		if (target_gamestate_index!=NONE)
		{
			target_entity_index = simulation_gamestate_entity_get_simulation_entity_index(target_gamestate_index);

			if (target_entity_index == NONE)
			{
				event(
					_event_error,
					"networking:simulation:projectiles: failed to get target entity index for gamestate 0x%8X (object %s)",
					target_gamestate_index,
					target_entity_index
				);
			}
		}

		projectile_creation_data->target_entity_index = target_entity_index;
		projectile_creation_data->target_entity_model_target_index = projectile->projectile.target_model_target_index;
	}

	projectile_creation_data->tracer = TEST_BIT(projectile->projectile.flags, _projectile_tracer_bit);
	projectile_creation_data->disable_deceleration = TEST_BIT(projectile->projectile.flags, _projectile_has_nonzero_angular_velocity_bit);
	
	return;
}

void c_simulation_projectile_entity_definition::entity_creation_encode(
	int32 creation_data_size,
	void const* creation_data,
	s_simulation_view_telemetry_data const* telemetry_data,
	c_bitstream* packet,
	bool encode_for_network)
{
	s_simulation_projectile_creation_data const* projectile_creation_data = (s_simulation_projectile_creation_data*)creation_data;
	
	ASSERT(creation_data_size==sizeof(struct s_simulation_projectile_creation_data));
	ASSERT(creation_data);
	ASSERT(packet);

	packet->push_structure("projectile-creation", NONE, 0);

	c_simulation_object_entity_definition::object_creation_encode(&projectile_creation_data->object_creation, packet, encode_for_network);

	packet->write_bool("owner-player-exists", projectile_creation_data->owner_player_absolute_index!=NONE);

	if (projectile_creation_data->owner_player_absolute_index!=NONE)
	{
		packet->write_integer("owner-player-index", projectile_creation_data->owner_player_absolute_index, k_player_index_bits);
	}

	{
		bool write_target_data = false;
		
		if (encode_for_network)
		{
			packet->write_bool("target-exists", projectile_creation_data->target_entity_index != NONE);

			if (projectile_creation_data->target_entity_index!=NONE)
			{
				replication_entity_index_encode(packet, projectile_creation_data->target_entity_index);
				write_target_data = true;
			}
		}
		else
		{
			// FIXME: IMPLEMENT THIS!!!!!!
			unreachable();
		}

		if (write_target_data)
		{
			packet->write_integer("target-model-target-index", projectile_creation_data->target_entity_model_target_index, 5);
		}
	}
	
	packet->write_bool("tracer", projectile_creation_data->tracer);
	packet->write_bool("disable-deceleration", projectile_creation_data->disable_deceleration);

	packet->pop_structure("projectile-creation", NONE);

	return;
}

bool c_simulation_projectile_entity_definition::entity_creation_decode(
	int32 creation_data_size,
	void* creation_data,
	c_bitstream* packet, 
	bool decode_for_network)
{
	bool object_success;
	bool decode_success;

	s_simulation_projectile_creation_data* projectile_creation_data = (s_simulation_projectile_creation_data*)creation_data;;

	ASSERT(creation_data_size==sizeof(struct s_simulation_projectile_creation_data));
	ASSERT(creation_data);
	ASSERT(packet);

	packet->push_structure("projectile-creation", NONE, 0);

	object_success = c_simulation_object_entity_definition::object_creation_decode(&projectile_creation_data->object_creation, packet, decode_for_network);

	if (packet->read_bool("owner-player-exists"))
	{
		projectile_creation_data->owner_player_absolute_index = packet->read_integer("owner-player-index", k_player_index_bits);
		object_success = object_success && VALID_INDEX(projectile_creation_data->owner_player_absolute_index, k_maximum_players);
	}
	else
	{
		projectile_creation_data->owner_player_absolute_index = NONE;
	}

	if (packet->read_bool("target-exists"))
	{
		if (decode_for_network)
		{
			replication_entity_index_decode(packet, &projectile_creation_data->target_entity_index);

			if (packet->read_bool("model-target-exists"))
			{
				projectile_creation_data->target_entity_model_target_index = packet->read_integer("target-model-target-index", 5);
			}
			else
			{
				projectile_creation_data->target_entity_model_target_index = NONE;
			}
		}
		else
		{
			// FIXME: IMPLEMENT THIS!!!!!!
			unreachable();
		}
	}
	else
	{
		projectile_creation_data->target_entity_index = NONE;
		projectile_creation_data->target_entity_model_target_index = NONE;
	}
	
	projectile_creation_data->tracer = packet->read_bool("tracer");
	projectile_creation_data->disable_deceleration = packet->read_bool("disable-deceleration");

	packet->pop_structure("projectile-creation", NONE);

	decode_success = !packet->overflowed() && object_success;

	return decode_success;
}
