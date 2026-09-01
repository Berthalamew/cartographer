#pragma once
#include "simulation_game_entities.h"

/* structures */

struct s_simulation_game_engine_player_creation_data
{
	int16 absolute_player_index;
};
ASSERT_STRUCT_SIZE(s_simulation_game_engine_player_creation_data, 2);

/* classes */

class c_simulation_game_engine_player_entity_definition : public c_simulation_entity_definition
{
public:
	virtual bool promote_game_entity_to_authority(int32 gamestate_index) override;

	virtual void build_creation_data(int32 gamestate_index, int32 creation_data_size, void* creation_data) override;

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
	virtual bool create_game_entity(
		int32 gamestate_index,
		int32 creation_data_size,
		void const* creation_data,
		uint32 initial_update_mask,
		int32 initial_state_data_size,
		void const* initial_state_data) override;
	virtual bool delete_game_entity(int32 gamestate_index) override;

	virtual bool update_game_entity(int32 gamestate_index, uint32 update_mask, int32 update_state_data_size, void const* update_state_data) override;

private:
	static int32 game_engine_get_player_index_by_gamestate_index(int32 gamestate_index);
};

/* prototypes */

void simulation_game_engine_player_apply_patches(void);
