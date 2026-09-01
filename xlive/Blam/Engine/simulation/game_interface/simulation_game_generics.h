#pragma once
#include "simulation_game_objects.h"

/* structures */

struct s_simulation_generic_creation_data
{
	s_simulation_object_creation_data object;
	string_id variant;
};
ASSERT_STRUCT_SIZE(s_simulation_generic_creation_data, 20);

/* classes */

class c_simulation_generic_entity_definition : public c_simulation_object_entity_definition
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

	virtual void build_object_creation_data(int32 generic_index, int32 creation_data_size, void* creation_data) override;
};

/* prototypes */

void simulation_game_generics_apply_patches(void);
