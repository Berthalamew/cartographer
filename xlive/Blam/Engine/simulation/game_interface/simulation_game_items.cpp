#include "stdafx.h"
#include "simulation_game_items.h"

#include "memory/bitstream.h"

/* prototypes */

CLASS_HOOK_DECLARE_LABEL(c_simulation_item_entity_definition__entity_creation_encode, c_simulation_item_entity_definition::entity_creation_encode);
static void __declspec(naked) jmp_c_simulation_item_entity_definition__entity_creation_encode(void)
{
	CLASS_HOOK_JMP(c_simulation_item_entity_definition__entity_creation_encode, c_simulation_item_entity_definition::entity_creation_encode);
}

CLASS_HOOK_DECLARE_LABEL(c_simulation_item_entity_definition__entity_creation_decode, c_simulation_item_entity_definition::entity_creation_decode);
static void __declspec(naked) jmp_c_simulation_item_entity_definition__entity_creation_decode(void)
{
	CLASS_HOOK_JMP(c_simulation_item_entity_definition__entity_creation_decode, c_simulation_item_entity_definition::entity_creation_decode);
}


CLASS_HOOK_DECLARE_LABEL(c_simulation_item_entity_definition__build_object_creation_data, c_simulation_item_entity_definition::build_object_creation_data);
static void __declspec(naked) jmp_c_simulation_item_entity_definition__build_object_creation_data(void)
{
	CLASS_HOOK_JMP(c_simulation_item_entity_definition__build_object_creation_data, c_simulation_item_entity_definition::build_object_creation_data);
}

/* public code */

void simulation_game_items_apply_patches(
	void)
{
	WritePointer(Memory::GetAddress(0x3C97F4), jmp_c_simulation_item_entity_definition__entity_creation_encode);
	WritePointer(Memory::GetAddress(0x3C97F8), jmp_c_simulation_item_entity_definition__entity_creation_decode);
	WritePointer(Memory::GetAddress(0x3C982C), jmp_c_simulation_item_entity_definition__build_object_creation_data);

	return;
}

void c_simulation_item_entity_definition::build_object_creation_data(
	int32 item_index, 
	int32 creation_data_size, 
	void* creation_data)
{
	s_simulation_item_creation_data* item_creation_data = (s_simulation_item_creation_data*)creation_data;

	ASSERT(creation_data_size==sizeof(struct s_simulation_item_creation_data));
	ASSERT(creation_data);

	csmemset(item_creation_data, 0, sizeof(*item_creation_data));
	c_simulation_object_entity_definition::object_build_creation_data(item_index, &item_creation_data->object);

	return;
}

void c_simulation_item_entity_definition::entity_creation_encode(
	int32 creation_data_size,
	void const* creation_data,
	s_simulation_view_telemetry_data const* telemetry_data,
	c_bitstream* packet,
	bool encode_for_network)
{
	s_simulation_item_creation_data const* item_creation_data = (s_simulation_item_creation_data const*)creation_data;


	ASSERT(creation_data_size==sizeof(struct s_simulation_item_creation_data));
	ASSERT(creation_data);
	ASSERT(packet);

	packet->push_structure("item-creation", NONE, 0);
	
	c_simulation_object_entity_definition::object_creation_encode(&item_creation_data->object, packet, encode_for_network);
	
	packet->pop_structure("item-creation", NONE);

	return;
}

bool c_simulation_item_entity_definition::entity_creation_decode(
	int32 creation_data_size,
	void* creation_data,
	c_bitstream* packet,
	bool decode_for_network)
{
	bool object_success;
	bool decode_success;

	s_simulation_item_creation_data* item_creation_data = (s_simulation_item_creation_data*)creation_data;

	ASSERT(creation_data_size==sizeof(struct s_simulation_item_creation_data));
	ASSERT(creation_data);
	ASSERT(packet);

	packet->push_structure("item-creation", NONE, 0);

	object_success = c_simulation_object_entity_definition::object_creation_decode(&item_creation_data->object, packet, decode_for_network);

	packet->pop_structure("item-creation", NONE);

	decode_success = !packet->overflowed() && object_success;

	return decode_success;
}
