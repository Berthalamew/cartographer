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
	virtual void entity_creation_encode(
		int32 creation_data_size,
		void const* creation_data,
		struct s_simulation_view_telemetry_data const* telemetry_data,
		class c_bitstream* packet,
		bool encode_for_network) override;
	virtual bool entity_creation_decode(
		int32 creation_data_size, 
		void* creation_data,
		class c_bitstream* packet,
		bool decode_for_network) override;
	virtual bool entity_update_encode(
		bool initial_update,
		uint32 update_mask,
		uint32* update_mask_written,
		int32 state_data_size,
		void const* state_data,
		struct s_simulation_view_telemetry_data const* telemetry_data,
		class c_bitstream* packet,
		int32 must_leave_space_bits,
		bool encode_for_network) override;
	virtual bool entity_update_decode(
		bool initial_update,
		uint32* update_mask,
		int32 state_data_size,
		void* state_data,
		class c_bitstream* packet,
		bool decode_for_network) override;

	virtual void build_object_creation_data(int32 device_index, int32 creation_data_size, void* creation_data) override;
};

/* prototypes */

void simulation_game_device_machines_apply_patches(void);
