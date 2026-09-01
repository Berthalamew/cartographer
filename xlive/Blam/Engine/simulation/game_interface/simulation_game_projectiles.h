#pragma once
#include "simulation_game_objects.h"

/* structures */

struct s_simulation_projectile_creation_data
{
	s_simulation_object_creation_data object_creation;
	int32 owner_player_absolute_index;
	int32 target_entity_index;
	int32 target_entity_model_target_index;
	bool tracer;
	bool disable_deceleration;
	int16 pad1;
};
ASSERT_STRUCT_SIZE(s_simulation_projectile_creation_data, 32);

/* classes */

class c_simulation_projectile_entity_definition : public c_simulation_object_entity_definition
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
	virtual void build_object_creation_data(int32 projectile_index, int32 creation_data_size, void* creation_data) override;
};

/* prototypes */

void simulation_game_projectiles_apply_patches(void);
