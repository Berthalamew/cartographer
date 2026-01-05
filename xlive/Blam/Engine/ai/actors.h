#pragma once
#include "actor_firing_position.h"
#include "actor_moving.h"
#include "pathfinding_utilities.h"

#include "game/game_allegiance.h"
#include "memory/data.h"
#include "objects/object_location.h"
#include "tag_files/tag_groups.h"

/*
   TODO: Reverse engineer more of the actor struct and fill this data in appropriately.
   For now, we only know where the character datum which was used to create the actor is.
*/

// Unsure about the size for this struct...
struct actor_datum_struct_41C
{
	uint8 gap_0[52];
	int field_34;
	__int16 field_38;
	uint8 gap_34[42];
	bool unk_bool_480;
	uint8 gap_481[43];
};

// Unsure about the size for this struct...
struct actor_control_data
{
	int ignore_target_object_index;
	bool unk_bool_4E8;
	uint8 gap_4F0[3];
	c_ai_point3d ai_point_4EC;
	int field_4FC;
	float field_500;
	__int16 field_504;
	bool unk_bool_506;
	uint8 gap_507;
	__int16 field_508;
	uint8 gap_50A[2];
	bool unk_bool_50C;
	uint8 gap_50D[159];
	int field_5AC;
	__int16 field_5B0;
	bool unk_bool_5B2;
	uint8 pad_5B3;
	__int16 field_5B4;
	__int16 field_5B6;
	uint8 gap_5B8[24];
	bool unk_bool_5D0;
	uint8 gap_5D1;
	uint8 unk_bool_5D2;
	uint8 gap_5D3[77];
	__int16 suppressed_shooting_tick_count;
	uint8 gap_622[52];
	__int16 field_656;
	uint8 gap_658[44];
	__int16 field_684;
	__int16 tick_count_686;
	__int16 field_688;
	__int16 field_68A;
	uint32 field_68C;
	uint8 gap_690[16];
	__int16 field_6A0;
	uint8 gap_6A2[2];
	datum field_6A4;
	uint8 gap_6A8[8];
	__int16 field_6B0;
	uint8 gap_6B2[2];
	datum field_6B4;
	uint8 gap_6B8[26];
	real_vector3d desired_facing_vector;
	real_vector3d desired_aiming_vector;
	real_vector3d desired_looking_vector;
	uint8 gap_6F8[6];
	__int16 fire_state;
	uint8 gap_700[34];
	__int16 current_fire_target_type;
	datum current_fire_target_prop_index;
	uint8 gap_728[136];
	real_vector3d burst_aim_vector;
};

// Unsure about the size for this struct...
struct actor_meta
{
	uint8 gap_0[4];
	__int16 type;
	bool unk_bool_6;
	bool swarm;
	bool unk_bool_8;
	bool active;
	bool squadless;
	bool force_active;
	uint8 gap_B[4];
	uint32 flee_firing_position_index;
	int pathfinding_timeslice;
	datum unit_index;
	datum swarm_index;
	uint32 field_20;
	e_game_team unit_team;
	uint8 gap_1C[2];
	int field_28;
	__int16 tick_count_2C;
	uint8 gap_2E[2];
	datum squad_index;
	datum squad_to_migrate_to;
	uint8 gap_38[6];
	__int16 sbsp_index;
	bool unk_bool_34;
	uint8 gap_41;
	uint16 field_42;
	bool unk_bool_44;
	uint8 gap_45;
	uint16 field_46;
	uint8 gap_42[12];
	datum character_tag_datum;
	int field_58;
	datum tracks[8];
	datum clump_index;
};

// Unsure about the size for this struct...
struct actor_datum_struct_90
{
	uint16 behavior_info_index;
	uint8 gap_0[62];
};

// Unsure about the size for this struct...
struct actor_target
{
	datum target_prop_index;
	uint8 gap_33C[8];
	datum retreat_target_prop_index;
};

// Unsure about the size for this struct...
struct actor_danger_zone
{
	__int16 danger_type;
	__int16 field_35A;
	bool unk_bool_35C;
	uint8 gap_35D;
	bool unk_bool_35E;
	uint8 gap_35F;
	datum object_index;
};

struct actor_datum
{
	actor_meta meta;
	uint8 gap_80[4];
	__int16 actor_status;
	__int16 field_86;
	uint8 gap_88[8];
	actor_datum_struct_90 array_90[2];
	uint8 gap_90[32];
	datum actor_datum;
	uint8 gap_134[92];
	__int16 gap_90_array_size;
	uint8 gap_192[150];
	bool blind;
	bool unk_bool_229;
	bool unk_bool_22A;
	bool unk_bool_22B;
	real_point3d position_22C;
	real_point3d current_position;
	uint8 gap_244[12];
	s_location location;
	uint8 gap_258[12];
	actor_input input;
	float body_current_vitality;
	float shield_current_vitality;
	float field_2D8;
	float field_2DC;
	bool unk_bool_2E0;
	uint8 gap_2E1[7];
	int field_2E8;
	__int16 field_2EC;
	__int16 field_2EE;
	bool unk_bool_2F0;
	uint8 gap_2F1;
	__int16 field_2F2;
	__int16 field_2F4;
	uint8 gap_2F6[10];
	int field_300;
	__int16 field_304;
	uint16 tick_count_306;
	uint8 gap_308[32];
	__int16 field_328;
	uint8 gap_32A[2];
	datum field_32C;
	uint8 gap_330[7];
	actor_target target;
	int field_348;
	uint8 gap_34C[12];
	actor_danger_zone danger_zone;
	uint8 gap_364[4];
	datum field_368;
	uint8 gap_36C[72];
	datum field_3B4;
	uint8 gap_3B8[60];
	firing_position_ref firing_positions;
	actor_datum_struct_41C some_struct_41C;
	__int16 field_4AC;
	bool unk_bool_4AE;
	bool unk_bool_4AF;
	uint32 field_4B0;
	uint32 field_4B4;
	c_ai_point3d ai_point_4B8;
	int field_4C8;
	uint32 field_4CC;
	uint32 field_4D0;
	bool unk_bool_4D4;
	uint8 gap_4D5;
	uint8 gap_4D6[14];
	actor_control_data control;
	float damage_modifier;
	uint8 gap_7C0[32];
	datum field_7E0;
	int field_7E4;
	uint8 gap_7E8[24];
	string_id movement_mode;
	datum field_804;
	uint8 gap_808[16];
	int field_818;
	int field_81C;
	real_vector3d vector_820;
	int field_82C;
	int field_830;
	real_vector3d vector_834;
	real_vector3d vector_840;
	real_vector3d vector_84C;
	uint8 gap_858[16];
	datum cs_script_datum;
	uint8 gap_86C[44];
};
ASSERT_STRUCT_SIZE(actor_datum, 0x898);

/* prototypes */

data_array* get_actor_table(void);

void __cdecl actors_initialize_for_new_map(void);
