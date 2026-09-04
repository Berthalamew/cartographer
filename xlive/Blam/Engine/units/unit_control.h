#pragma once
#include "game/aim_assist.h"

/* structures */

struct s_unit_weapon_set
{
	int16 set_identifier;
	int8 weapon_indices[2];
};

struct unit_control_data
{
	string_id animation_state;
	uint16 aiming_speed;
	uint16 weapon_set_identifier;
	uint8 field_8;
	uint8 field_9;
	uint16 grenade_index;
	uint16 zoom_level;
	int8 gap_E[2];
	int64 control_flags;
	real_vector3d throttle;
	real32 primary_trigger;
	real32 secondary_trigger;
	real_vector3d facing_vector;
	real_vector3d aiming_vector;
	real_vector3d looking_vector;
	int32 field_50;
	int32 field_54;
	int32 field_58;
	s_aim_assist_targeting_result target_info;
};
ASSERT_STRUCT_SIZE(unit_control_data, 0x80);
