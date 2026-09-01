#pragma once
#include "simulation_game_entities.h"

/* structures */

struct s_simulation_breakable_surface_group_creation_data
{
	int16 group_index;
};
ASSERT_STRUCT_SIZE(s_simulation_breakable_surface_group_creation_data, 2);

/* classes */

class c_simulation_breakable_surface_group_entity_definition : public c_simulation_entity_definition
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

	virtual void build_creation_data(int32 gamestate_index, int32 creation_data_size, void* creation_data) override;
	

};

/* prototypes */

void simulation_game_damage_apply_patches(void);
