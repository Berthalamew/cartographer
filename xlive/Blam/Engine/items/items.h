#pragma once
#include "objects/objects.h"

/* enums */

enum e_item_flags
{
	_item_in_unit_inventory_bit,
	_item_hidden_in_unit_inventory_bit,

	// TODO: confirm below names
	_item_belongs_to_player_bit,
	_item_has_nonzero_angular_velocity_bit,
	_item_on_structure_bit,
	_item_on_object_bit,
	_item_does_not_accelerate_bit,
	_item_part_of_respawn_system,
	_item_bit_8,
	NUMBER_OF_ITEM_DATUM_FLAGS,
};

/* structs */

struct _item_datum
{
	c_flags_no_init<e_item_flags, uint16, NUMBER_OF_ITEM_DATUM_FLAGS> flags;
	int16 detonation_ticks;
	int16 bsp_index;
	int16 rested_surface_index;
	int16 pad;
	uint8 field_136;
	uint8 breakable_surface_index;
	uint8 field_138[2];
	bool unk_bool_13A;
	uint8 node_index;
	datum ignore_object_index;
	uint8 field_13B[12];
	datum unit_index;
	uint32 relevance;
	datum inventory_owner_unit_index;
	real_vector3d rotation_axis;
	real32 rotation_sine;
	real32 rotation_cosine;
};

struct item_datum
{
	datum definition_index;
	_object_datum object;
	_item_datum item;
};
ASSERT_STRUCT_SIZE(item_datum, 364);
