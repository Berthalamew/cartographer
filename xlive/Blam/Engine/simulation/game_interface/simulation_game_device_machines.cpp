#include "stdafx.h"
#include "simulation_game_device_machines.h"

#include "simulation_game_object_constants.h"

#include "memory/bitstream.h"
#include "networking/network_utilities.h"
#include "simulation/simulation_entity_definition.h"

/* constants */

static real32 const k_device_update_position_min = 0.f;
static real32 const k_device_update_position_max = 1.f;

/* prototypes */

CLASS_HOOK_DECLARE_LABEL(c_simulation_device_entity_definition__entity_creation_encode, c_simulation_device_entity_definition::entity_creation_encode);
static void __declspec(naked) jmp_c_simulation_device_entity_definition__entity_creation_encode(void)
{
	CLASS_HOOK_JMP(c_simulation_device_entity_definition__entity_creation_encode, c_simulation_device_entity_definition::entity_creation_encode);
}

CLASS_HOOK_DECLARE_LABEL(c_simulation_device_entity_definition__entity_creation_decode, c_simulation_device_entity_definition::entity_creation_decode);
static void __declspec(naked) jmp_c_simulation_device_entity_definition__entity_creation_decode(void)
{
	CLASS_HOOK_JMP(c_simulation_device_entity_definition__entity_creation_decode, c_simulation_device_entity_definition::entity_creation_decode);
}

CLASS_HOOK_DECLARE_LABEL(c_simulation_device_entity_definition__entity_update_encode, c_simulation_device_entity_definition::entity_update_encode);
static void __declspec(naked) jmp_c_simulation_device_entity_definition__entity_update_encode(void)
{
	CLASS_HOOK_JMP(c_simulation_device_entity_definition__entity_update_encode, c_simulation_device_entity_definition::entity_update_encode);
}

CLASS_HOOK_DECLARE_LABEL(c_simulation_device_entity_definition__entity_update_decode, c_simulation_device_entity_definition::entity_update_decode);
static void __declspec(naked) jmp_c_simulation_device_entity_definition__entity_update_decode(void)
{
	CLASS_HOOK_JMP(c_simulation_device_entity_definition__entity_update_decode, c_simulation_device_entity_definition::entity_update_decode);
}

CLASS_HOOK_DECLARE_LABEL(c_simulation_device_entity_definition__build_object_creation_data, c_simulation_device_entity_definition::build_object_creation_data);
static void __declspec(naked) jmp_c_simulation_device_entity_definition__build_object_creation_data(void)
{
	CLASS_HOOK_JMP(c_simulation_device_entity_definition__build_object_creation_data, c_simulation_device_entity_definition::build_object_creation_data);
}

/* public code */

void simulation_game_device_machines_apply_patches(
	void)
{
	WritePointer(Memory::GetAddress(0x3C9694), jmp_c_simulation_device_entity_definition__entity_creation_encode);
	WritePointer(Memory::GetAddress(0x3C9698), jmp_c_simulation_device_entity_definition__entity_creation_decode);
	WritePointer(Memory::GetAddress(0x3C969C), jmp_c_simulation_device_entity_definition__entity_update_encode);
	WritePointer(Memory::GetAddress(0x3C96A0), jmp_c_simulation_device_entity_definition__entity_update_decode);
	WritePointer(Memory::GetAddress(0x3C96CC), jmp_c_simulation_device_entity_definition__build_object_creation_data);

	return;
}

void c_simulation_device_entity_definition::entity_creation_encode(
	int32 creation_data_size,
	void const* creation_data,
	struct s_simulation_view_telemetry_data const* telemetry_data,
	class c_bitstream* packet,
	bool encode_for_network)
{
	s_simulation_device_creation_data const* device_creation_data = (s_simulation_device_creation_data const*)creation_data;

	ASSERT(creation_data_size==sizeof(struct s_simulation_device_creation_data));
	ASSERT(creation_data);
	ASSERT(packet);



	packet->push_structure("device-creation", NONE, 0);

	c_simulation_object_entity_definition::object_creation_encode(&device_creation_data->object, packet, encode_for_network);

	packet->pop_structure("device-creation", NONE);

	return;
}

bool c_simulation_device_entity_definition::entity_creation_decode(
	int32 creation_data_size,
	void* creation_data,
	class c_bitstream* packet,
	bool decode_for_network)
{
	bool object_success;
	bool decode_success;

	s_simulation_device_creation_data* device_creation_data= (s_simulation_device_creation_data*)creation_data;

	ASSERT(creation_data_size==sizeof(struct s_simulation_device_creation_data));
	ASSERT(creation_data);
	ASSERT(packet);

	packet->push_structure("device-creation", NONE, 0);

	object_success = c_simulation_object_entity_definition::object_creation_decode(&device_creation_data->object, packet, decode_for_network);

	packet->pop_structure("device-creation", NONE);

	decode_success = !packet->overflowed() && object_success;

	return decode_success;
}

bool c_simulation_device_entity_definition::entity_update_encode(
	bool initial_update,
	uint32 update_mask,
	uint32* update_mask_written,
	int32 state_data_size,
	void const* state_data,
	s_simulation_view_telemetry_data const* telemetry_data,
	c_bitstream* packet,
	int32 must_leave_space_bits,
	bool encode_for_network)
{
	bool wrote_update = false;
	s_simulation_device_state_data const* device_state_data = (s_simulation_device_state_data const*)state_data;
	int32 object_must_leave_space_bits = must_leave_space_bits + 4;
	uint32 object_update_mask = update_mask & MASK(k_simulation_object_update_flag_count);
	
	ASSERT(update_mask!=0);
	ASSERT((update_mask & ~MASK(k_simulation_device_update_flag_count))==0);
	ASSERT(update_mask_written);
	ASSERT(*update_mask_written==0);
	ASSERT(state_data_size==sizeof(struct s_simulation_device_state_data));
	ASSERT(state_data);
	ASSERT(packet);

	packet->push_structure("device-update", NONE, 0);

	if (c_simulation_object_entity_definition::object_update_encode(
		initial_update,
		object_update_mask,
		update_mask_written,
		telemetry_data,
		&device_state_data->object_state,
		packet,
		object_must_leave_space_bits,
		ensure_object_position_update_quantization_inside_bsp(),
		encode_for_network))
	{
		c_entity_update_encode_helper update;

		int32 first_update_flag = _simulation_device_update_position_bit;
		int32 update_flag_count = k_simulation_device_update_flag_count - k_simulation_object_update_flag_count;
		uint32 weapon_update_mask = MASK(k_simulation_device_update_flag_count) - MASK(k_simulation_object_update_flag_count);

		if (update.make_room_for_update(packet, must_leave_space_bits, first_update_flag, update_flag_count, update_mask & weapon_update_mask)
		)
		{
			if (update.write_component_header(_simulation_device_update_position_bit, "position-exists"))
			{
				packet->write_quantized_real("position", device_state_data->position, k_device_update_position_min, k_device_update_position_max, 14, false);
			}

			update.finish_component();

			if (update.write_component_header(_simulation_device_update_position_group_position_bit, "position-group-position-exists"))
			{
				packet->write_quantized_real("position-group-position", device_state_data->position_group_position, k_device_update_position_min, k_device_update_position_max, 14, false);
			}

			update.finish_component();
			update.finish_update(update_mask_written);
			wrote_update = true;
		}
	}

	packet->pop_structure("device-update", NONE);

	return wrote_update;
}

bool c_simulation_device_entity_definition::entity_update_decode(
	bool initial_update,
	uint32* update_mask,
	int32 state_data_size,
	void* state_data,
	class c_bitstream* packet,
	bool decode_for_network)
{
	bool decode_success = false;

	ASSERT(update_mask);
	ASSERT(*update_mask==0);
	ASSERT(state_data_size==sizeof(struct s_simulation_device_state_data));
	ASSERT(state_data);

	bandwidth_profiler_record_push(18, packet);

	s_simulation_device_state_data* device_state_data = (s_simulation_device_state_data*)state_data;
	bool object_success = c_simulation_object_entity_definition::object_update_decode(
		initial_update,
		update_mask,
		&device_state_data->object_state,
		packet,
		decode_for_network);
	decode_success = object_success;

	if (packet->read_bool("position-exists"))
	{
		device_state_data->position = packet->read_quantized_real(
			"position",
			k_device_update_position_min,
			k_device_update_position_max,
			14, 
			false);
		SET_BIT(*update_mask, _simulation_device_update_position_bit, true);
	}

	if (packet->read_bool("position-group-position-exists"))
	{
		device_state_data->position_group_position = packet->read_quantized_real(
			"position-group-position",
			k_device_update_position_min,
			k_device_update_position_max,
			14,
			false);
		SET_BIT(*update_mask, _simulation_device_update_position_group_position_bit, true);
	}

	bandwidth_profiler_record_pop(18, packet);

	decode_success = decode_success && !packet->overflowed();

	return decode_success;
}


void c_simulation_device_entity_definition::build_object_creation_data(
	int32 device_index,
	int32 creation_data_size,
	void* creation_data)
{
	s_simulation_device_creation_data* device_creation_data = (s_simulation_device_creation_data*)creation_data;

	ASSERT(creation_data_size==sizeof(struct s_simulation_device_creation_data));
	ASSERT(creation_data);

	csmemset(creation_data, 0, sizeof(*device_creation_data));
	c_simulation_object_entity_definition::object_build_creation_data(device_index, &device_creation_data->object);

	return;
}
