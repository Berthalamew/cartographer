#pragma once
#include "objects/object_constants.h"

/* structures */

struct s_simulation_queue_gamestate_clear_data
{
	c_static_flags_no_init<k_maximum_objects_per_map> entities;
};

struct s_simulation_gamestate_entity
{
	int16 identifier;
	int32 simulation_entity_index;
	int32 object_index;
	bool marked_for_deletion;
};

/* prototypes */

void simulation_gamestate_entities_initialize(void);
void simulation_gamestate_entities_dispose(void);
void simulation_gamestate_entities_initialize_for_new_map(void);
void simulation_gamestate_entities_build_clear_flags(s_simulation_queue_gamestate_clear_data* gamestate_clear_data_out);
void simulation_gamestate_entities_clear_by_flags(const s_simulation_queue_gamestate_clear_data* gamestate_clear_data);
void simulation_gamestate_entities_notify_simulation_world_reset(void);
void simulation_gamestate_entities_dispose_from_old_map(void);
int32 simulation_gamestate_entity_create(void);
int32 simulation_gamestate_entity_create_at_index(int32 gamestate_index);
void simulation_gamestate_entity_delete(int32 gamestate_index);
int32 simulation_gamestate_entity_get_object_index(int32 gamestate_index);
int32 simulation_gamestate_entity_get_object_index_type_safe(int32 gamestate_index, uint32 object_type_mask);
void simulation_gamestate_entity_set_object_index(int32 gamestate_index, int32 object_index);
int32 simulation_gamestate_entity_get_simulation_entity_index(int32 gamestate_index);
void simulation_gamestate_entity_set_simulation_entity_index(int32 gamestate_index, int32 entity_index);
bool simulation_gamestate_indices_are_equivalent(int32 gamestate_1_index, int32 gamestate_2_index);
bool simulation_gamestate_index_valid(int32 gamestate_index);
void simulation_gamestate_index_encode(class c_bitstream* stream, int32 gamestate_index);
void simulation_gamestate_index_decode(class c_bitstream* stream, int32* gamestate_index);
