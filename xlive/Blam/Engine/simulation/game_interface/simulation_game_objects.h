#pragma once
#include "simulation_game_entities.h"
#include "models/model_constants.h"
#include "objects/emblems.h"

/* constants */

enum
{
	k_object_constraint_count=16
};

enum
{
	_simulation_object_interpolation_relative_position_valid_bit = 0,
	_simulation_object_interpolation_orientation_valid_bit,
	k_simulation_object_interpolation_count,
};

/* structures */

struct s_simulation_object_state_data
{
	real_point3d relative_position;
	real_vector3d forward;
	real_vector3d up;
	real32 scale;
	real_vector3d translational_velocity;
	real_vector3d angular_velocity;
	real32 body_vitality;
	bool body_stun_ticks_is_zero;
	real32 shield_vitality;
	bool shield_stun_ticks_is_zero;
	bool dead;
	int16 owner_team_index;
	uint8 region_count;
	uint8 region_states[MAXIMUM_REGIONS_PER_MODEL];
	uint8 constraint_count;
	uint16 destroyed_constraints;
	uint16 loosened_constraints;

	// Unsure about these....

	int32 parent_entity_index;
	int32 parent_gamestate_index;

	uint8 unknown[32];
};
ASSERT_STRUCT_SIZE(s_simulation_object_state_data, 0x90);

struct s_simulation_object_creation_data
{
	int32 scenario_datum_index;
	int32 object_definition_index;
	int8 multiplayer_spawn_monitor_index;
	// unused padding
	char pad[3];

	s_emblem_info emblem_info;
	
	// Repurpose padding for variant index
	int8 model_variant_index;
};
ASSERT_STRUCT_SIZE(s_simulation_object_creation_data, 16);

/* classes */

class c_simulation_object_entity_definition : public c_simulation_entity_definition
{
public:
	virtual bool build_updated_state_data(s_simulation_entity const* entity, uint32* update_mask, int32 update_state_data_size, void* update_state_data) override;
	virtual bool create_game_entity(
		int32 gamestate_index,
		int32 creation_data_size,
		void const* creation_data,
		uint32 initial_update_mask,
		int32 initial_state_data_size,
		void const* initial_state_data) override;

	virtual bool entity_replication_required_for_view_activation(struct s_simulation_entity const* entity) override;
	virtual bool entity_type_is_gameworld_object(void) override;
	virtual bool entity_can_be_created(struct s_simulation_entity const* entity, struct s_simulation_view_telemetry_data const* telemetry_data) override;
	virtual bool build_baseline_state_data(int32 creation_data_size, void* creation_data, int32 state_data_size, void* out_state_baseline_data) override;
	virtual uint32 rotate_entity_indices(struct s_simulation_entity* entity) override;
	virtual bool update_game_entity(int32 gamestate_index, uint32 update_mask, int32 update_state_data_size, void const* update_state_data) override;
	virtual bool delete_game_entity(int32 gamestate_index) override;
	virtual bool promote_game_entity_to_authority(int32 gamestate_index) override;
	virtual void build_creation_data(int32 gamestate_index, int32 creation_data_size, void* out_creation_data) override;
	
	// c_simulation_object_entity_definition additions

	virtual void* object_required_to_join_game(datum object_index) = 0;
	virtual int32 create_object(int32 creation_data_size, void const* creation_data, uint32* flags, int32 internal_state_data_size, void const* initial_state_data) = 0;
	virtual void* handle_delete_object(int32 object_index) = 0;
	virtual void* apply_object_update(datum object_index, uint32 entity_sync_flag, int32 state_data_size, const void* state_data) = 0;
	virtual void* promote_object_to_authority(datum object_index) = 0;
	virtual bool ensure_object_position_update_quantization_inside_bsp(void) = 0;
	virtual void* object_interpolation_data_out(int32 a1, void* a2, void* a3) = 0;
	virtual void* object_interpolation_data(int32 a1, void* a2, void* a3) = 0;

protected:
	static void object_build_creation_data(int32 object_index, struct s_simulation_object_creation_data* creation_data);

	static bool object_setup_placement_data(
		struct s_simulation_object_creation_data const* object_creation_data,
		struct s_simulation_object_state_data const* object_state_data,
		uint32* initial_update_mask,
		struct object_placement_data* placement_data);

	int32 object_create_object(
		struct s_simulation_object_creation_data const *object_creation_data,
		s_simulation_object_state_data const *object_state_data,
		uint32* initial_update_mask,
		object_placement_data* placement_data);

	static void object_creation_encode(struct s_simulation_object_creation_data const* object_creation_data, class c_bitstream* packet, bool encode_for_network);
	static bool object_creation_decode(struct s_simulation_object_creation_data* object_creation_data, class c_bitstream* packet, bool decode_for_network);
	static bool object_update_encode(
		bool initial_update,
		uint32 update_mask,
		uint32* update_mask_written,
		struct s_simulation_view_telemetry_data const* telemetry_data,
		struct s_simulation_object_state_data const* object_state_data,
		class c_bitstream* packet,
		int32 must_leave_space_bits,
		bool ensure_position_update_quantization_inside_bsp,
		bool encode_for_network);
	static bool object_update_decode(
		bool initial_update,
		uint32* update_mask,
		struct s_simulation_object_state_data* object_state_data,
		class c_bitstream* packet,
		bool decode_for_network);
};

/* prototypes */

void simulation_game_objects_apply_patches(void);

int32 __cdecl simulation_object_get_replicated_object_from_entity(int32 entity_index);
