#include "stdafx.h"
#include "simulation_game_engine_globals.h"

/* prototypes */

CLASS_HOOK_DECLARE_LABEL(c_simulation_game_engine_globals_definition__build_creation_data, c_simulation_game_engine_globals_definition::build_creation_data);
static void __declspec(naked) jmp_c_simulation_game_engine_globals_definition__build_creation_data(void)
{
	CLASS_HOOK_JMP(c_simulation_game_engine_globals_definition__build_creation_data, c_simulation_game_engine_globals_definition::build_creation_data);
}

/* globals */

static uintptr_t p_c_simulation_game_engine_globals_definition__build_creation_data;

/* public code */

void simulation_game_engine_globals_apply_patches(
	void)
{
	DETOUR_ATTACH(p_c_simulation_game_engine_globals_definition__build_creation_data, Memory::GetAddress(0x1F7555, 0x0), jmp_c_simulation_game_engine_globals_definition__build_creation_data);

	return;
}

void c_simulation_game_engine_globals_definition::build_creation_data(
	int32 gamestate_index,
	int32 creation_data_size,
	void* out_creation_data)
{
	ASSERT(creation_data_size==0);

	return;
}

void c_simulation_game_engine_globals_definition::entity_creation_encode(
	int32 creation_data_size,
	void const* creation_data,
	struct s_simulation_view_telemetry_data const* telemetry_data,
	class c_bitstream* packet,
	bool encode_for_network)
{
	ASSERT(creation_data_size==0);

	return;
}

bool c_simulation_game_engine_globals_definition::entity_creation_decode(
	int32 creation_data_size,
	void* creation_data,
	class c_bitstream* packet,
	bool decode_for_network)
{
	ASSERT(creation_data_size==0);

	return true;
}

bool __stdcall c_simulation_game_engine_globals_definition::global_update_encode(
	uint32 update_mask,
	uint32* update_mask_written,
	struct s_game_engine_state_data const* state_data,
	class c_bitstream* packet,
	int32 must_leave_space_bits)
{
	return INVOKE(0x1F76B7, 0x0, c_simulation_game_engine_globals_definition::global_update_encode, update_mask, update_mask_written, state_data, packet, must_leave_space_bits);
}

bool __stdcall c_simulation_game_engine_globals_definition::global_update_decode(
	uint32* update_mask,
	struct s_game_engine_state_data* state_data,
	class c_bitstream* packet)
{
	return INVOKE(0x1F789E, 0x0, c_simulation_game_engine_globals_definition::global_update_decode, update_mask, state_data, packet);
}
