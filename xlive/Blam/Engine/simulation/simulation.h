#pragma once

#include "simulation_world.h"
#include "simulation_type_collection.h"
#include "game/player_control.h"

// TODO structure?
struct s_simulation_update
{
	int gap_0;
	bool simulation_in_progress;
};

bool simulation_in_progress();
void simulation_destroy_update();
bool simulation_query_object_is_predicted(datum object_datum);
c_simulation_type_collection* simulation_get_type_collection();

void __cdecl simulation_process_input(uint32 player_action_mask, const player_action* player_actions);

void simulation_apply_patches();

c_simulation_world* simulation_get_world();