#pragma once

/* constants */

enum
{
	k_replication_entity_table_length = 1024,
	k_replication_entity_absolute_index_bits = 10,
	k_replication_entity_seed_bits = 4,
};

/* enums */

enum e_replication_entity_flags
{
	_replication_entity_allocated_flag = 0,
	_replication_entity_marked_for_deletion_flag,
	_replication_entity_local_flag,
	k_replication_entity_flags_count,
};

enum e_replication_entity_view_code
{
	replication_entity_view_code_create_entity = 0x1,
	replication_entity_view_code_delete_entity = 0x2,
	replication_entity_view_code_create_entity_collection = 0x3,
	replication_entity_view_code_delete_entity_collection = 0x4,
	replication_entity_view_code_update_entity = 0x5,
};

enum e_replication_entity_view_state
{
	_replication_entity_view_state_none = 0x0,
	_replication_entity_view_state_ready = 0x1,
	_replication_entity_view_state_replicating = 0x2,
	_replication_entity_view_state_active = 0x3,
	_replication_entity_view_state_deleting = 0x4,
	k_replication_entity_view_state_count = 0x5,
};

/* structures */

struct s_replication_entity_data
{
	uint8 /*e_replication_entity_flags*/ flags;
	uint8 seed;
	int16 deletion_mask;
	int32 field_4;
};

/* prototypes */

// Encode the entity index to the bitstream passed
void replication_entity_index_encode(class c_bitstream* packet, int32 entity_index);
// Decode the entity index from the bitstream passed
void replication_entity_index_decode(class c_bitstream* packet, int32* entity_index);
