#pragma once

/* constants */

enum
{
	k_simulation_entity_database_maximum_entities = 1024
};

/* macros */

#define ENTITY_INDEX_NEW(absolute_index, seed)			((absolute_index) | ((((uint8)seed) << 28)))
#define ENTITY_INDEX_TO_ABSOLUTE_INDEX(_entity_index)	((_entity_index) & (k_simulation_entity_database_maximum_entities-1))
#define ENTITY_INDEX_TO_SEED(_entity_index)				((_entity_index) >> 28)

/* enums */

enum e_simulation_entity_type : int16
{
	_simulation_entity_type_slayer_engine_globals = 0,
	_simulation_entity_type_ctf_engine_globals,
	_simulation_entity_type_oddball_engine_globals,
	_simulation_entity_type_king_engine_globals,
	_simulation_entity_type_territories_engine_globals,
	_simulation_entity_type_juggernaut_engine_globals,
	_simulation_entity_type_game_engine_player,
	_simulation_entity_type_game_statborg,
	_simulation_entity_type_breakable_surface_group,
	_simulation_entity_type_unit,
	_simulation_entity_type_item,
	_simulation_entity_type_generic,
	_simulation_entity_type_vehicle,
	_simulation_entity_type_projectile,
	_simulation_entity_type_weapon,
	_simulation_entity_type_turret,
	_simulation_entity_type_device,
	k_simulation_entity_count,

	k_simulation_entity_type_none = NONE,
	k_simulation_entity_type_first_game_engine = _simulation_entity_type_slayer_engine_globals,
	k_simulation_entity_type_last_game_engine = _simulation_entity_type_juggernaut_engine_globals,
};

/* structures */

struct s_simulation_entity
{
	int32 entity_index;
	e_simulation_entity_type entity_type;
	bool exists_in_gameworld;
	int8 event_reference_count;
	int32 gamestate_index;						// Converted to gamestate_index in Halo 3
	uint32 pending_update_mask;
	uint32 force_update_mask;
	int32 creation_data_size;
	void* creation_data;
	int32 state_data_size;
	void* state_data;
};
ASSERT_STRUCT_SIZE(s_simulation_entity, 0x24);

struct s_simulation_baseline_state_data
{
	uint8 gap_0[248];
};
ASSERT_STRUCT_SIZE(s_simulation_baseline_state_data, 248);

/* classes */

class c_simulation_entity_definition
{
public:
	virtual e_simulation_entity_type entity_type(void) = 0;
	virtual const char* entity_type_name(void) = 0;
	virtual int32 state_data_size(void) const = 0;
	virtual int32 creation_data_size(void) const = 0;
	virtual int32 update_flag_count(void) = 0;
	virtual uint32 initial_update_mask(void) = 0;
	virtual bool entity_replication_required_for_view_activation(const s_simulation_entity* entity) = 0;
	virtual bool entity_type_is_gameworld_object(void) = 0;
	virtual bool entity_can_be_created(struct s_simulation_entity const* entity, struct s_simulation_view_telemetry_data const* telemetry_data) = 0;
	virtual int8 creation_minimum_required_bits(struct s_simulation_entity const* entity, struct s_simulation_view_telemetry_data const* telemetry_data, int32* minimum_required_bits) = 0;
	virtual void write_creation_description_to_string(struct s_simulation_entity* entity, void* tel_data, int32 buffer_size, char* buffer) = 0;
	virtual void write_update_description_to_string(struct s_simulation_entity const* entity, struct s_entity_update_data const* update_data, int32 buffer_length, char* buffer) = 0;
	virtual void entity_creation_encode(int32 creation_data_size, void const* creation_data, struct s_simulation_view_telemetry_data const* telemetry_data, class c_bitstream* packet, bool encode_for_network) = 0;
	virtual bool entity_creation_decode(int32 creation_data_size, void* creation_data, class c_bitstream* packet, bool decode_for_network) = 0;
	virtual bool entity_update_encode(
		bool initial_update,
		uint32 update_mask,
		uint32* update_mask_written,
		int32 state_data_size,
		void const* state_data,
		struct s_simulation_view_telemetry_data const* telemetry_data,
		class c_bitstream* packet,
		int32 must_leave_space_bits,
		bool encode_for_network) = 0;
	virtual bool entity_update_decode(
		bool initial_update,
		uint32* update_mask,
		int32 state_data_size,
		void* state_data,
		class c_bitstream* packet,
		bool decode_for_network) = 0;
	virtual bool entity_state_lossy_compare(void* state_data_a, void* state_data_b, int32 state_data_size) = 0;
	virtual bool entity_creation_lossy_compare(void* creation_data_a, void* creation_data_b, int32 creation_data_size) = 0;
	virtual void build_creation_data(int32 gamestate_index, int32 creation_data_size, void* out_creation_data) = 0;
	virtual bool build_baseline_state_data(int32 creation_data_size, void* creation_data, int32 state_data_size, void* out_state_baseline_data) = 0;
	virtual bool build_updated_state_data(s_simulation_entity const* entity, uint32* update_mask, int32 state_data_size, void* state_data) = 0;
	virtual uint32 rotate_entity_indices(s_simulation_entity* entity) = 0;
	virtual bool create_game_entity(
		int32 gamestate_index,
		int32 creation_data_size,
		void const* creation_data,
		uint32 initial_update_mask,
		int32 initial_state_data_size,
		void const* initial_state_data) = 0;
	virtual bool update_game_entity(int32 gamestate_index, uint32 update_mask, int32 update_state_data_size, void const* update_state_data) = 0;
	virtual bool delete_game_entity(int32 gamestate_index) = 0;
	virtual bool promote_game_entity_to_authority(int32 gamestate_index) = 0;
	virtual void build_object_creation_data(int32 object_index, int32 creation_data_size, void* creation_data) = 0;
	virtual uint32 handle_object_update(int32 object_index, uint32 update_mask, int32 state_data_size, void* state_data) = 0;
};

/* prototypes */

void simulation_game_entities_apply_patches(void);

bool simulation_object_index_valid(datum object_index);

int32 __cdecl simulation_entity_create(e_simulation_entity_type entity_type, int32 object_index, int32 gamestate_index);
void simulation_entity_update(int32 entity_index, int32 object_index, uint32 flags);
void simulation_entity_force_update(int32 entity_index, int32 object_index, uint32 flags);
void simulation_entity_delete(int32 entity_index, int32 object_index, int32 gamestate_index);

e_simulation_entity_type __cdecl simulation_entity_type_from_object_creation(int32 object_definition_index, int32 parent_object_index);

e_simulation_entity_type simulation_entity_type_from_game_engine(void);
char const* simulation_entity_type_get_name(e_simulation_entity_type entity_type);

int32 simulation_entity_get_gamestate_index(int32 entity_index);
