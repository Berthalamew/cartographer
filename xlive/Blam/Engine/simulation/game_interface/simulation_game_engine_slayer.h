#pragma once
#include "simulation_game_engine_globals.h"

/* structures */

struct s_slayer_engine_state_data
{
	s_game_engine_state_data global_state;
};

/* classes */

class c_simulation_slayer_engine_globals_definition : public c_simulation_game_engine_globals_definition
{
public:
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
};

/* prototypes */

void simulation_game_engine_slayer_apply_patches(void);
