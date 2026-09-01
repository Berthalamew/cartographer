#pragma once
#include "simulation_game_entities.h"

/* structures */

struct s_game_engine_state_data
{
	uint16 initial_teams;
	uint16 valid_team_designators;
	uint16 valid_teams;
	uint16 active_teams;
	uint16 ever_active_teams;
	int16 initial_team_count;
	int16 team_designator_to_team_index[9];
	uint8 current_state;
	bool game_finished;
	int16 round_index;
	int16 round_timer;
};

/* classes */

class c_simulation_game_engine_globals_definition : public c_simulation_entity_definition
{
public:
	virtual void build_creation_data(int32 gamestate_index, int32 creation_data_size, void* out_creation_data) override;
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

	static bool __stdcall global_update_encode(
		uint32 update_mask,
		uint32* update_mask_written,
		struct s_game_engine_state_data const* state_data,
		class c_bitstream* packet,
		int32 must_leave_space_bits);
	static bool __stdcall global_update_decode(
		uint32* update_mask,
		struct s_game_engine_state_data* state_data,
		class c_bitstream* packet);

};
