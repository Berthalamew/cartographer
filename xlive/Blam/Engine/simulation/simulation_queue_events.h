#pragma once
#include "simulation/game_interface/simulation_game_events.h"

/* prototypes */

void simulation_queue_event_insert(
	e_simulation_event_type event_type,
	uint32 reference_count,
	const int32* entity_reference_indices,
	int32 payload_size,
	void* payload);

void simulation_queue_event_apply(const struct s_simulation_queue_element* element);

void convert_entity_references_to_gamestate_references(
	int32 const* entity_reference_indices,
	int32 reference_count,
	int32* gamestate_indices,
	int32 gamestate_indices_count);

bool encode_event_to_buffer(
	uint8* encode_buffer,
	int32 encode_buffer_size,
	int32* out_encoded_size,
	e_simulation_event_type event_type,
	int32 reference_count,
	const int32* gamestate_indices,
	int32 payload_size,
	void* payload);
