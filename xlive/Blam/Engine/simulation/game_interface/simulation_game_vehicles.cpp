#include "stdafx.h"
#include "simulation_game_vehicles.h"

#include "simulation_game_object_constants.h"

#include "memory/bitstream.h"

/* prototypes */

CLASS_HOOK_DECLARE_LABEL(c_simulation_vehicle_entity_definition__entity_creation_encode, c_simulation_vehicle_entity_definition::entity_creation_encode);
static void __declspec(naked) jmp_c_simulation_vehicle_entity_definition__entity_creation_encode(void)
{
	CLASS_HOOK_JMP(c_simulation_vehicle_entity_definition__entity_creation_encode, c_simulation_vehicle_entity_definition::entity_creation_encode);
}

CLASS_HOOK_DECLARE_LABEL(c_simulation_vehicle_entity_definition__entity_creation_decode, c_simulation_vehicle_entity_definition::entity_creation_decode);
static void __declspec(naked) jmp_c_simulation_vehicle_entity_definition__entity_creation_decode(void)
{
	CLASS_HOOK_JMP(c_simulation_vehicle_entity_definition__entity_creation_decode, c_simulation_vehicle_entity_definition::entity_creation_decode);
}

/* public code */

void simulation_game_vehicles_apply_patches(
	void)
{
	WritePointer(Memory::GetAddress(0x3CA5C4, 0x0), jmp_c_simulation_vehicle_entity_definition__entity_creation_encode);
	WritePointer(Memory::GetAddress(0x3CA5C8, 0x0), jmp_c_simulation_vehicle_entity_definition__entity_creation_decode);

	return;
}

void c_simulation_vehicle_entity_definition::entity_creation_encode(
	int32 creation_data_size,
	void const* creation_data,
	struct s_simulation_view_telemetry_data const* telemetry_data,
	class c_bitstream* packet,
	bool encode_for_network)
{
	s_simulation_vehicle_creation_data const* vehicle_creation_data = (s_simulation_vehicle_creation_data const*)creation_data;

	ASSERT(creation_data_size==sizeof(struct s_simulation_vehicle_creation_data));
	ASSERT(creation_data);
	ASSERT(packet);

	packet->push_structure("vehicle-creation", NONE, 0);

	c_simulation_object_entity_definition::object_creation_encode(&vehicle_creation_data->object, packet, encode_for_network);
	packet->write_raw_data("variant-name", &vehicle_creation_data->variant, SIZEOF_BITS(vehicle_creation_data->variant));

	packet->pop_structure("vehicle-creation", NONE);

	return;
}

bool c_simulation_vehicle_entity_definition::entity_creation_decode(
	int32 creation_data_size,
	void* creation_data,
	class c_bitstream* packet,
	bool decode_for_network)
{
	bool object_success;
	bool decode_success;

	s_simulation_vehicle_creation_data* vehicle_creation_data = (s_simulation_vehicle_creation_data*)creation_data;

	ASSERT(creation_data_size==sizeof(struct s_simulation_vehicle_creation_data));
	ASSERT(creation_data);
	ASSERT(packet);

	packet->push_structure("vehicle-creation", NONE, 0);

	object_success = c_simulation_object_entity_definition::object_creation_decode(&vehicle_creation_data->object, packet, decode_for_network);
	packet->read_raw_data("variant-name", &vehicle_creation_data->variant, SIZEOF_BITS(vehicle_creation_data->variant));

	packet->pop_structure("vehicle-creation", NONE);

	decode_success = !packet->overflowed() && object_success;

	return decode_success;
}

bool c_simulation_vehicle_entity_definition::entity_update_encode(
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
	s_simulation_vehicle_state_data const* vehicle_state_data = (s_simulation_vehicle_state_data const*)state_data;

	ASSERT(update_mask!=0);
	ASSERT((update_mask & ~MASK(k_simulation_vehicle_update_flag_count))==0);
	ASSERT(update_mask_written);
	ASSERT(*update_mask_written==0);
	ASSERT(state_data_size==sizeof(struct s_simulation_vehicle_state_data));
	ASSERT(state_data);
	ASSERT(packet);

	packet->push_structure("vehicle-update", NONE, 0);

	object_update_mask = update_mask & MASK(k_simulation_object_update_flag_count);
	object_must_leave_space_bits = must_leave_space_bits;
	
	wrote_update = c_simulation_object_entity_definition::object_update_encode(
		initial_update,
		object_update_mask,
		update_mask_written,
		telemetry_data,
		&vehicle_state_data->object_state,
		packet,
		must_leave_space_bits,
		ensure_object_position_update_quantization_inside_bsp(),
		encode_for_network
	);
	
	packet->pop_structure("vehicle-update", NONE);

	return wrote_update;
}
	
bool c_simulation_vehicle_entity_definition::entity_update_decode(
	bool initial_update,
	uint32* update_mask,
	int32 state_data_size,
	void* state_data,
	class c_bitstream* packet,
	bool decode_for_network) 
{
	//bool decode_success;
	//bool object_success;

	//s_simulation_vehicle_state_data* vehicle_state_data = ;



	//return decode_success;
	return false;
}
