#include "stdafx.h"
#include "simulation_game_engine_slayer.h"

/* prototypes */

CLASS_HOOK_DECLARE_LABEL(c_simulation_slayer_engine_globals_definition__entity_update_encode, c_simulation_slayer_engine_globals_definition::entity_update_encode);
static void __declspec(naked) jmp_c_simulation_slayer_engine_globals_definition__entity_update_encode(void)
{
	CLASS_HOOK_JMP(c_simulation_slayer_engine_globals_definition__entity_update_encode, c_simulation_slayer_engine_globals_definition::entity_update_encode);
}

CLASS_HOOK_DECLARE_LABEL(c_simulation_slayer_engine_globals_definition__entity_update_decode, c_simulation_slayer_engine_globals_definition::entity_update_decode);
static void __declspec(naked) jmp_c_simulation_slayer_engine_globals_definition__entity_update_decode(void)
{
	CLASS_HOOK_JMP(c_simulation_slayer_engine_globals_definition__entity_update_decode, c_simulation_slayer_engine_globals_definition::entity_update_decode);
}

/* public code */

void simulation_game_engine_slayer_apply_patches(
	void)
{
	WritePointer(Memory::GetAddress(0x3CB14C, 0x0), jmp_c_simulation_slayer_engine_globals_definition__entity_update_encode);
	WritePointer(Memory::GetAddress(0x3CB150, 0x0), jmp_c_simulation_slayer_engine_globals_definition__entity_update_decode);

	return;
}

bool c_simulation_slayer_engine_globals_definition::entity_update_encode(
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
	bool wrote_update;

	s_slayer_engine_state_data const* slayer_state_data = (s_slayer_engine_state_data const*)state_data;

	ASSERT(state_data_size==sizeof(struct s_slayer_engine_state_data));
	ASSERT(state_data);

	wrote_update = c_simulation_game_engine_globals_definition::global_update_encode(update_mask, update_mask_written, &slayer_state_data->global_state, packet, must_leave_space_bits);

	return wrote_update;
}

bool c_simulation_slayer_engine_globals_definition::entity_update_decode(
	bool initial_update,
	uint32* update_mask,
	int32 state_data_size,
	void* state_data,
	class c_bitstream* packet,
	bool decode_for_network)
{
	bool success;
	uint32 update_mask_read = 0;
	
	s_slayer_engine_state_data* slayer_state_datax = (s_slayer_engine_state_data*)state_data;

	ASSERT(update_mask);
	ASSERT(state_data_size==sizeof(struct s_slayer_engine_state_data));
	ASSERT(state_data);

	if (c_simulation_game_engine_globals_definition::global_update_decode(&update_mask_read, &slayer_state_datax->global_state, packet) && update_mask_read)
	{
		success = true;
		*update_mask = update_mask_read;
	}
	else
	{
		success = false;
		*update_mask = update_mask_read;
	}

	return success;
}
