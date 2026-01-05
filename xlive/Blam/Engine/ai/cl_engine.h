#pragma once
#include "tag_files/tag_block.h"

/* enums */

enum e_cs_point_set_flags : int
{
	cs_point_set_flag_manual_reference_frame = FLAG(0),
	cs_point_set_flag_turret_deployment = FLAG(1)
};

/* structures */

// max count: 20
struct cs_point
{
	char name[k_tag_string_length];
	real_point3d position;
	short pad;
	int surface_index;
	real_euler_angles2d facing_direction;
};
ASSERT_STRUCT_SIZE(cs_point, 60);

// max count: 200
struct cs_point_set
{
	char name[k_tag_string_length];
	tag_block<cs_point> points;
	int16 bsp_index;            // Block index: scenario_structure_bsp_reference
	int16 manual_reference_frame;
	e_cs_point_set_flags flags;
};
ASSERT_STRUCT_SIZE(cs_point_set, 48);

// max count: 1
struct cs_script_data
{
	s_tag_block point_sets;	// struct: cs_point_set
	int8 pad[120];
};
ASSERT_STRUCT_SIZE(cs_script_data, 128);

/* prototypes */

struct command_script_datum
{
	uint32 gap0;
	int16 cs_action_type;
	int16 generic_type;
	real32 generic_float_argument;
	real32 generic_float_argument2;
	int8 gap1[24];
	int32 generic_goal;
	int32 generic_facing_target;
	int16 unk2A;
	int16 pad;
	int32 field_34;
	int32 field_38;
	int8 gap2[9];
	bool ai_shooting_at_something;
	bool ai_actively_focusing_on_something;
	bool unk47;
	int16 focus_type;
	int16 unk4A;
	datum object_of_focus;
	bool unk51;
	bool ai_object_set;
	bool ai_focusing_on_something;
	bool unk53;
	int16 aiming;
	int16 unk56;
	datum object;
	bool is_crouching;
	real32 one_divided_by_crouch_time;
	bool vehicle_speed_multiplier_set;
	int8 pad0[3];
	real32 vehicle_speed_multiplier;
	bool force_turn_sharpness;
	int8 pad1[3];
	real32 turn_sharpness;
	bool force_vehicle_boost;
	int8 gap3[10];
	bool autonomous_targeting;
	bool targeting_has_been_touched;
	int8 gap81[2];
	bool supress_combat_dialouge;
	int16 unk84;
	bool actor_type_overwritten;
	bool unk87;
	datum actor_style;
	int8 gap5[8];
	string_id current_movement_mode;
	int8 gap6[20];
	bool b_unkAC;
	int8 padAD[3];
	datum another_object;
	real32 distance;
	real32 max_distance;
	real32 follow_distance;
	int8 gap7[16];
	bool unkD0;
};
ASSERT_STRUCT_SIZE(command_script_datum, 212);


/* prototypes */

void __cdecl cs_initialize_for_new_map(void);

struct data_array* command_script_data_get(void);

struct cs_script_data* cs_scenario_get_script_data(struct scenario* scenario);

struct cs_point_set* cs_get_point_set(int32 index);
