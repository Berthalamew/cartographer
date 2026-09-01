#include "stdafx.h"
#include "simulation_game_interface.h"

#include "simulation/simulation_type_collection.h"

/* public code */

void __cdecl simulation_game_register_types(
	c_simulation_type_collection* event_type,
	int32* entity_type_count,
	int32* event_type_count)
{
	INVOKE(0x1DAF44, 0x1C0653, simulation_game_register_types, event_type, entity_type_count, event_type_count);
	return;
}

int32 simulation_definition_table_index_bits(void)
{
	return 9;
}
