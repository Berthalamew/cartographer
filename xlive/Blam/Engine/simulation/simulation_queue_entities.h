#pragma once
#include "game_interface/simulation_game_entities.h"

struct s_simulation_queue_entity_data
{
	int32 entity_index;
	e_simulation_entity_type entity_type;
	int32 creation_data_size;
	void* creation_data;
	int32 state_data_size;
	void* state_data;
};

/* prototypes */

bool simulation_queue_entity_creation_allocate(
	struct s_simulation_queue_entity_data* entity_data,
	uint32 initial_update_mask,
	struct s_simulation_queue_element** simulation_queue_element_out,
	int32* gamestate_index_out); 
void simulation_queue_entity_creation_insert(struct s_simulation_queue_element* element);
void simulation_queue_entity_creation_apply(const s_simulation_queue_element* element);
bool simulation_queue_entity_update_allocate(
	struct s_simulation_queue_entity_data* sim_queue_entity_data,
	int32 gamestate_index,
	uint32 update_mask,
	struct s_simulation_queue_element** element);
void simulation_queue_entity_update_insert(struct s_simulation_queue_element* simulation_queue_element);
void simulation_queue_entity_update_apply(const struct s_simulation_queue_element* element);
void simulation_queue_entity_deletion_insert(struct s_simulation_entity* entity, bool force_cleanup_after_deletion);
void simulation_queue_entity_deletion_apply(const struct s_simulation_queue_element* element);
void simulation_queue_entity_promotion_insert(struct s_simulation_entity* entity);
void simulation_queue_entity_promotion_apply(const struct s_simulation_queue_element* element);
void simulation_queue_entity_encode_header(class c_bitstream* bitstream, e_simulation_entity_type type, int32 gamestate_index);
bool simulation_queue_entity_decode_header(class c_bitstream* bitstream, e_simulation_entity_type* entity_type, int32* gamestate_index);
bool encode_simulation_queue_creation_to_buffer(
	uint8* buffer,
	int32 buffer_size,
	int32 gamestate_index,
	struct s_simulation_queue_entity_data const* entity_data,
	uint32 initial_update_mask,
	int32* encoded_size_out);
bool decode_simulation_queue_creation_from_buffer(uint8* buffer, int32 buffer_size, struct s_simulation_queue_decoded_creation_data* decoded_creation_data);
bool encode_simulation_queue_update_to_buffer(
	uint8* buffer,
	int32 buffer_size,
	struct s_simulation_queue_entity_data const* entity_data,
	int32 gamestate_index,
	uint32 update_mask,
	int32* encoded_size_out);
bool decode_simulation_queue_update_from_buffer(
	uint8* buffer,
	int32 buffer_size,
	struct s_simulation_queue_decoded_update_data* decoded_update_data);
