#pragma once
#include "simulation_game_objects.h"

/* structures */

struct s_simulation_device_creation_data
{
	s_simulation_object_creation_data object;
};
ASSERT_STRUCT_SIZE(s_simulation_device_creation_data, 16);

struct s_simulation_device_state_data
{
	s_simulation_object_state_data object_state;
	real32 position;
	real32 position_group_position;
};
ASSERT_STRUCT_SIZE(s_simulation_device_state_data, 152);

/* classes */

class c_simulation_device_entity_definition : public c_simulation_object_entity_definition
{
public:
	virtual bool entity_update_decode(
		bool initial_update,
		uint32* update_mask,
		int32 state_data_size,
		void* state_data,
		class c_bitstream* packet);
};

/* prototypes */

void simulation_game_device_machines_apply_patches(void);
