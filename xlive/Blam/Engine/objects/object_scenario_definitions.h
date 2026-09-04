#pragma once
#include "object_constants.h"
#include "object_identifier.h"

/* enums */

enum e_bsp_policy : int8
{
	_bsp_policy_default = 0,
	_bsp_policy_always_placed = 1,
	_bsp_policy_manual_bsp_placement = 2
};

enum e_scenario_object_placement_flags : int32
{
	_scenario_object_placement_not_automatically_bit = 0,
	_scenario_object_placement_not_on_easy_bit,
	_scenario_object_placement_not_on_normal_bit,
	_scenario_object_placement_not_on_hard_bit,
	_scenario_object_placement_lock_type_to_env_object_bit,
	_scenario_object_placement_lock_transform_to_env_object_bit,
	_scenario_object_placement_never_placed_bit,
	_scenario_object_placement_lock_name_to_env_object_bit,
	_scenario_object_placement_create_at_rest_bit,
	k_scenario_object_placement_flags
};

enum e_scenario_object_transform_flags : int16
{
	_scenario_object_mirrored_bit = 0,
	k_number_of_scenario_transform_flags
};

/* structures */

// max count: MAXIMUM_SCENARIO_OBJECT_PALETTE_ENTRIES_PER_BLOCK 256
struct scenario_object_palette_entry
{
	tag_reference name;
	uint32 unused[8];
};
ASSERT_STRUCT_SIZE(scenario_object_palette_entry, 40);

struct s_scenario_object_permutation
{
	string_id variant_name;
	e_object_change_color_index active_change_colors;
	pixel32 primary_color;
	pixel32 secondary_color;
	pixel32 tertiary_color;
	pixel32 quaternary_color;
};
ASSERT_STRUCT_SIZE(s_scenario_object_permutation, 24);

struct s_scenario_object_datum
{
	// obed
	e_scenario_object_placement_flags placement_flags;
	real_point3d position;
	real_euler_angles3d rotation;
	real32 scale;
	e_scenario_object_transform_flags transform_flags;
	uint16 manual_bsp_flags;
	c_object_identifier object_id;
	e_bsp_policy bsp_policy;
	int8 pad;
	int16 editor_folder_index;
};
ASSERT_STRUCT_SIZE(s_scenario_object_datum, 48);
