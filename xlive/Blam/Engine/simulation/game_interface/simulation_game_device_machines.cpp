#include "stdafx.h"
#include "simulation_game_device_machines.h"

#include "simulation_game_object_constants.h"

#include "memory/bitstream.h"
#include "networking/network_utilities.h"

/* constants */

static real32 const k_device_update_position_min = 0.f;
static real32 const k_device_update_position_max = 1.f;

/* typedefs */

typedef bool(__thiscall* device_entity_update_decode_t)(
	c_simulation_device_entity_definition*,
	bool,
	uint32*,
	int32,
	void*,
	c_bitstream*);

/* prototypes */

CLASS_HOOK_DECLARE_LABEL(c_simulation_device_entity_definition__entity_update_decode, c_simulation_device_entity_definition::entity_update_decode);
static void __declspec(naked) jmp_c_simulation_device_entity_definition__entity_update_decode(void)
{
	CLASS_HOOK_JMP(c_simulation_device_entity_definition__entity_update_decode, c_simulation_device_entity_definition::entity_update_decode);
}

/* globals */

static device_entity_update_decode_t p_device_entity_update_decode;

/* public code */

void simulation_game_device_machines_apply_patches(
	void)
{
	DETOUR_ATTACH(
		p_device_entity_update_decode,
		Memory::GetAddress<device_entity_update_decode_t>(0x1FAF3A),
		jmp_c_simulation_device_entity_definition__entity_update_decode
	);
	return;
}

bool c_simulation_device_entity_definition::entity_update_decode(
	bool initial_update,
	uint32* update_mask,
	int32 state_data_size,
	void* state_data,
	class c_bitstream* packet)
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
		packet);
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
