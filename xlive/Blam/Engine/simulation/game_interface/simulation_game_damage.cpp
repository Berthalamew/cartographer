#include "stdafx.h"
#include "simulation_game_damage.h"

#include "memory/bitstream.h"
#include "networking/network_event.h"
#include "simulation/simulation_gamestate_entities.h"

/* prototypes */

CLASS_HOOK_DECLARE_LABEL(c_simulation_breakable_surface_group_entity_definition__build_creation_data, c_simulation_breakable_surface_group_entity_definition::build_creation_data);
static __declspec(naked) void jmp_c_simulation_breakable_surface_group_entity_definition__build_creation_data(void)
{
	CLASS_HOOK_JMP(c_simulation_breakable_surface_group_entity_definition__build_creation_data, c_simulation_breakable_surface_group_entity_definition::build_creation_data);
}

CLASS_HOOK_DECLARE_LABEL(c_simulation_breakable_surface_group_entity_definition__entity_creation_encode, c_simulation_breakable_surface_group_entity_definition::entity_creation_encode);
static __declspec(naked) void jmp_c_simulation_breakable_surface_group_entity_definition__entity_creation_encode(void)
{
	CLASS_HOOK_JMP(c_simulation_breakable_surface_group_entity_definition__entity_creation_encode, c_simulation_breakable_surface_group_entity_definition::entity_creation_encode);
}

CLASS_HOOK_DECLARE_LABEL(c_simulation_breakable_surface_group_entity_definition__entity_creation_decode, c_simulation_breakable_surface_group_entity_definition::entity_creation_decode);
static __declspec(naked) void jmp_c_simulation_breakable_surface_group_entity_definition__entity_creation_decode(void)
{
	CLASS_HOOK_JMP(c_simulation_breakable_surface_group_entity_definition__entity_creation_decode, c_simulation_breakable_surface_group_entity_definition::entity_creation_decode);
}

/* public code */

void simulation_game_damage_apply_patches(
	void)
{
	WritePointer(Memory::GetAddress(0x3C9CEC), jmp_c_simulation_breakable_surface_group_entity_definition__build_creation_data);
	WritePointer(Memory::GetAddress(0x3C9CD4), jmp_c_simulation_breakable_surface_group_entity_definition__entity_creation_encode);
	WritePointer(Memory::GetAddress(0x3C9CD8), jmp_c_simulation_breakable_surface_group_entity_definition__entity_creation_decode);

	return;
}

void c_simulation_breakable_surface_group_entity_definition::build_creation_data(
	int32 gamestate_index,
	int32 creation_data_size,
	void* creation_data)
{
	s_simulation_breakable_surface_group_creation_data* breakable_surface_creation_data = (s_simulation_breakable_surface_group_creation_data*)creation_data;

	ASSERT(creation_data_size == sizeof(s_simulation_breakable_surface_group_creation_data));
	ASSERT(creation_data);
	ASSERT(gamestate_index != NONE);

	breakable_surface_creation_data->group_index = (int16)simulation_gamestate_entity_get_object_index(gamestate_index);

	if (breakable_surface_creation_data->group_index == NONE)
	{
		event(_event_warning, "networking:simulation:damage:breakable-surfaces: failed to get group index for gamestate 0x%8X (build-creation-data)", gamestate_index);
	}

	return;
}

void c_simulation_breakable_surface_group_entity_definition::entity_creation_encode(
	int32 creation_data_size,
	void const* creation_data,
	struct s_simulation_view_telemetry_data const* telemetry_data,
	class c_bitstream* packet,
	bool encode_for_network)
{
	s_simulation_breakable_surface_group_creation_data const* breakable_surface_creation_data = (s_simulation_breakable_surface_group_creation_data const*)creation_data;

	ASSERT(creation_data_size==sizeof(s_simulation_breakable_surface_group_creation_data));
	ASSERT(creation_data);

	packet->push_structure("breakable-surface-group-creation", NONE, 0);

	packet->write_raw_data("group_index", &breakable_surface_creation_data->group_index, SIZEOF_BITS(breakable_surface_creation_data->group_index));

	packet->pop_structure("breakable-surface-group-creation", NONE);

	return;
}

bool c_simulation_breakable_surface_group_entity_definition::entity_creation_decode(
	int32 creation_data_size,
	void* creation_data,
	class c_bitstream* packet,
	bool decode_for_network)
{
	bool decode_success;

	s_simulation_breakable_surface_group_creation_data* breakable_surface_creation_data = (s_simulation_breakable_surface_group_creation_data*)creation_data;

	packet->push_structure("breakable-surface-group-creation", NONE, 0);

	packet->read_raw_data("group_index", &breakable_surface_creation_data->group_index, SIZEOF_BITS(breakable_surface_creation_data->group_index));

	packet->pop_structure("breakable-surface-group-creation", NONE);

	decode_success = !packet->overflowed() && breakable_surface_creation_data->group_index != NONE;

	return decode_success;
}
