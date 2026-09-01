#include "stdafx.h"
#include "simulation_game_weapons.h"

#include "memory/bitstream.h"

/* prototypes */

CLASS_HOOK_DECLARE_LABEL(c_simulation_weapon_entity_definition__build_object_creation_data, c_simulation_weapon_entity_definition::build_object_creation_data);
static void __declspec(naked) jmp_c_simulation_weapon_entity_definition__build_object_creation_data(void)
{
	CLASS_HOOK_JMP(c_simulation_weapon_entity_definition__build_object_creation_data, c_simulation_weapon_entity_definition::build_object_creation_data);
}

CLASS_HOOK_DECLARE_LABEL(c_simulation_weapon_entity_definition__entity_creation_encode, c_simulation_weapon_entity_definition::entity_creation_encode);
static void __declspec(naked) jmp_c_simulation_weapon_entity_definition__entity_creation_encode(void)
{
	CLASS_HOOK_JMP(c_simulation_weapon_entity_definition__entity_creation_encode, c_simulation_weapon_entity_definition::entity_creation_encode);
}

CLASS_HOOK_DECLARE_LABEL(c_simulation_weapon_entity_definition__entity_creation_decode, c_simulation_weapon_entity_definition::entity_creation_decode);
static void __declspec(naked) jmp_c_simulation_weapon_entity_definition__entity_creation_decode(void)
{
	CLASS_HOOK_JMP(c_simulation_weapon_entity_definition__entity_creation_decode, c_simulation_weapon_entity_definition::entity_creation_decode);
}

/* public code */

void simulation_game_weapons_apply_patches(
	void)
{
	WritePointer(Memory::GetAddress(0x3C9C34, 0x0), jmp_c_simulation_weapon_entity_definition__build_object_creation_data);
	WritePointer(Memory::GetAddress(0x3C9BFC, 0x0), jmp_c_simulation_weapon_entity_definition__entity_creation_encode);
	WritePointer(Memory::GetAddress(0x3C9C00, 0x0), jmp_c_simulation_weapon_entity_definition__entity_creation_decode);


	return;
}

void c_simulation_weapon_entity_definition::build_object_creation_data(
	int32 weapon_index,
	int32 creation_data_size,
	void* creation_data)
{
	s_simulation_weapon_creation_data* weapon_creation_data = (s_simulation_weapon_creation_data*)creation_data;

	ASSERT(creation_data_size==sizeof(struct s_simulation_weapon_creation_data));
	ASSERT(creation_data);

	csmemset(weapon_creation_data, 0, sizeof(*weapon_creation_data));
	c_simulation_item_entity_definition::build_object_creation_data(weapon_index, sizeof(weapon_creation_data->item), &weapon_creation_data->item);

	return;
}

void c_simulation_weapon_entity_definition::entity_creation_encode(
	int32 creation_data_size,
	void const* creation_data,
	s_simulation_view_telemetry_data const* telemetry_data,
	c_bitstream* packet,
	bool encode_for_network)
{
	s_simulation_weapon_creation_data const* weapon_creation_data = (s_simulation_weapon_creation_data const*)creation_data;

	ASSERT(creation_data_size==sizeof(struct s_simulation_weapon_creation_data));
	ASSERT(creation_data);
	ASSERT(packet);

	packet->push_structure("weapon-create", NONE, 0);
	
	c_simulation_item_entity_definition::entity_creation_encode(sizeof(weapon_creation_data->item), &weapon_creation_data->item, telemetry_data, packet, encode_for_network);

	packet->pop_structure("weapon-create", NONE);

	return;
}


bool c_simulation_weapon_entity_definition::entity_creation_decode(
	int32 creation_data_size,
	void* creation_data,
	c_bitstream* packet,
	bool decode_for_network)
{
	bool item_success;
	bool decode_success;
	
	s_simulation_weapon_creation_data* weapon_creation_data = (s_simulation_weapon_creation_data*)creation_data;

	ASSERT(creation_data_size==sizeof(struct s_simulation_weapon_creation_data));
	ASSERT(creation_data);
	ASSERT(packet);

	item_success = c_simulation_item_entity_definition::entity_creation_decode(sizeof(weapon_creation_data->item), &weapon_creation_data->item, packet, decode_for_network);

	decode_success = !packet->overflowed() && item_success;

	return decode_success;
}
