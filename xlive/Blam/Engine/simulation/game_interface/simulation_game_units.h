#pragma once
#include "simulation_game_objects.h"
#include "saved_games/player_profile.h"
#include "units/units.h"
#include "units/unit_definitions.h"

/* structures */

struct s_simulation_unit_weapon_state_data
{
	int32 weapon_definition_index;
	int16 multiplayer_weapon_identifier;
	int16 simulation_weapon_identifier;
	int16 rounds_loaded;
	int16 rounds_inventory;
	real32 age;
};

struct s_simulation_unit_state_data
{
	s_simulation_object_state_data object_state_data;
	int16 controlling_player_absolute_index;
	int16 controlling_simulation_actor_index;
	datum parent_vehicle_entity_index;
	int16 parent_vehicle_seat;
	real_vector3d desired_aiming_vector;
	s_unit_weapon_set desired_weapon_set;
	s_simulation_unit_weapon_state_data weapons[MAXIMUM_INITIAL_WEAPONS_PER_UNIT];
	int8 grenade_counts[k_unit_grenade_types_count];
	bool active_camouflage_active;
	real32 active_camo;
	real32 active_camo_regrowth;
};
ASSERT_STRUCT_SIZE(s_simulation_unit_state_data, 0xF8);

struct s_simulation_unit_creation_data
{
	s_simulation_object_creation_data object;
	s_player_appearance appearance;
	e_game_team team;
	int8 pad1[2];
};
ASSERT_STRUCT_SIZE(s_simulation_unit_creation_data, 36);

/* classes */

class c_simulation_unit_entity_definition : public c_simulation_object_entity_definition
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

void simulation_game_units_apply_patches(void);
