#pragma once
#include "simulation/game_interface/simulation_game_events.h"

/* prototypes */

void simulation_queue_event_apply(const struct s_simulation_queue_element* update);
void simulation_queue_event_insert(
	e_simulation_event_type event_type,
	uint32 reference_count,
	const int32* entity_reference_indices,
	int32 payload_size,
	void* payload);
